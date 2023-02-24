//
// Created by Khubaib Umer on 18/01/2023.
//

#include <memory>
#include "libmqsim.h"
#include "ScopedMapInserter.h"
#include "../exec/Execution_Parameter_Set.h"
#include "../exec/SSD_Device.h"
#include "../exec/Host_System.h"

#ifndef SINGLETON
#define SINGLETON
#endif

#include "integration.h"
#include "config.h"
#include "data_store.h"
#include "safe_unordered_map.h"

#define GET_HOST(x) ((Host_System*) GET_SYSTEM(x)->host)
#define GET_SSD(x)  ((SSD_Device*) GET_SYSTEM(x)->ssd)
#define GET_ENGINE(x) ((MQSimEngine::Engine*) GET_SYSTEM(x)->engine)
#define GET_SYSTEM(x) ((system_t*) (x))
#define GET_DATASTORE(x) (GET_SYSTEM(x)->datastore)

using EngineInstanceMap_t = safe_unorderd_map<tid_t, MQSimEngine::Engine *>;
EngineInstanceMap_t EngineMap;

using HostInstanceMap_t = safe_unorderd_map<tid_t, Host_System *>;
HostInstanceMap_t HostMap;

using DataStoreMap_t = safe_unorderd_map<tid_t, void *>;
DataStoreMap_t DataStoreMap;

IO_Flow_Parameter_Set_Integration_Based *create_integration_flow() {
    const auto &str_xml = "<?xml version=\"1.0\" encoding=\"us-ascii\"?>\n"
                          "<IO_Scenario>\n"
                          "<Priority_Class>HIGH</Priority_Class>\n"
                          "<Device_Level_Data_Caching_Mode>WRITE_CACHE</Device_Level_Data_Caching_Mode>\n"
                          "<Channel_IDs>0,1,2,3,4,5,6,7</Channel_IDs>\n"
                          "<Chip_IDs>0,1,2,3</Chip_IDs>\n"
                          "<Die_IDs>0,1</Die_IDs>\n"
                          "<Plane_IDs>0,1</Plane_IDs>\n"
                          "<Initial_Occupancy_Percentage>70</Initial_Occupancy_Percentage>\n"
                          "</IO_Scenario>";

    char *str = strdup(str_xml);

    auto flow = new IO_Flow_Parameter_Set_Integration_Based;
    rapidxml::xml_document<> doc;
    doc.parse<0>(str);

    for (auto xml_io_scenario = doc.first_node(
            "IO_Scenario"); xml_io_scenario; xml_io_scenario = xml_io_scenario->next_sibling("IO_Scenario")) {
        for (auto flow_def = xml_io_scenario->first_node(); flow_def; flow_def = flow_def->next_sibling()) {
            ((IO_Flow_Parameter_Set_Integration_Based *) flow)->XML_deserialize(flow_def);
        }
    }
    flow->Time_Unit = Trace_Time_Unit::NANOSECOND;
    return flow;
}


EXTERN_C bool init_system(system_t **handle, const char *ssd_cfg) {
    if (*handle) {
        log_warn ("%s", "MQSim instance is already initialized");
        return true;
    }
    *handle = _calloc(1, system_t);
    auto exec_params = new Execution_Parameter_Set; // throws std::bad_alloc

    read_configuration_parameters(ssd_cfg, &exec_params);
    exec_params->Host_Configuration.IO_Flow_Definitions.push_back(create_integration_flow());

    GET_SYSTEM(*handle)->engine = new MQSimEngine::Engine;
    SCOPED_INSERTER(EngineMap, get_tid(), GET_ENGINE(*handle));

    GET_SYSTEM(*handle)->ssd = new SSD_Device(&exec_params->SSD_Device_Configuration,
                                              &exec_params->Host_Configuration.IO_Flow_Definitions);//Create SSD_Device based on the specified parameters
    if (!GET_SYSTEM(*handle)->ssd) {
        log_error ("%s", "Allocation of ssd failed");
        return false;
    }

    GET_SYSTEM(*handle)->host = new Host_System(&exec_params->Host_Configuration,
                                                exec_params->SSD_Device_Configuration.Enabled_Preconditioning,
                                                GET_SSD(*handle)->Host_interface);
    if (!GET_SYSTEM(*handle)->host) {
        log_error ("%s", "Allocation of host failed");
        return false;
    }
    SCOPED_INSERTER(HostMap, get_tid(), GET_HOST(*handle));

    GET_HOST(*handle)->Attach_ssd_device(GET_SSD(*handle));
    GET_ENGINE(*handle)->initialize();
    GET_DATASTORE(*handle) = new_data_store();
    *handle = GET_SYSTEM(*handle);

    log_debug ("%s", "System is initialized!!");
    return true;
}

pthread_t simulator_engine;

EXTERN_C void ignite_system() {
    // Start Servicing Thread
    pthread_create(&simulator_engine, NULL, &MQSimEngine::Engine::StartServicingThread, NULL);
}

EXTERN_C bool set_callbacks(cb_f clock, done_cb_f req_complete, done_cb_f timer) {
    STATELESS_CALL(MAGNITION, set_master_clock, clock);
    STATELESS_CALL(MAGNITION, set_done_callback, req_complete);
    STATELESS_CALL(MAGNITION, register_timer_callback, timer);
    STATELESS_CALL0(MAGNITION, notify);
    return true;
}

EXTERN_C void timer_fired(uint64_t requested_time) {
    printf("Timer fired for time: %llu\n", requested_time);
    STATELESS_CALL(MAGNITION, notify);
}


EXTERN_C bool dispatch_request(request_type_t *req) {
    auto ret = STATEFUL_CALL(MAGNITION->controller, add_job, req);
    STATELESS_CALL0(MAGNITION, notify);
    return ret;
}

EXTERN_C void destroy(system_t *handle) {
    SCOPED_INSERTER(EngineMap, get_tid(), GET_ENGINE(handle));
    SCOPED_INSERTER(HostMap, get_tid(), GET_HOST(handle));
    SCOPED_INSERTER(DataStoreMap, get_tid(), GET_DATASTORE(handle));
    delete GET_SSD(handle);
    delete GET_HOST(handle);
    delete GET_ENGINE(handle);
    clear_store(GET_DATASTORE(handle), true);
    STATELESS_CALL(MAGNITION, free, GET_DATASTORE(handle));
    STATELESS_CALL(MAGNITION, free, handle);
    printf("Destroyed %p!!\n", handle);
}

EXTERN_C bool process_request(system_t *handle, request_type_t *request, uint64_t *required_time, uint64_t curr_time,
                              insert_to_queue_f queue) {
    const tid_t tid = get_tid();
    SCOPED_INSERTER(HostMap, tid, GET_HOST(handle));
    SCOPED_INSERTER(EngineMap, tid, GET_ENGINE(handle));
    SCOPED_INSERTER(DataStoreMap, tid, GET_DATASTORE(handle));
    return GET_ENGINE(handle)->process_request(request, required_time, curr_time, queue);
}

EXTERN_C void dump_results(system_t *handle, uint64_t curr_time) {
    SCOPED_INSERTER(HostMap, get_tid(), GET_HOST(handle));
    SCOPED_INSERTER(EngineMap, get_tid(), GET_ENGINE(handle));
    std::vector<Host_Components::IO_Flow_Base *> IO_flows = GET_HOST(handle)->Get_io_flows();
    for (auto &IO_flow: IO_flows) {
        log_debug ("STATS (%llu) Flow:%s  - total requests generated:%u total requests serviced:%u",
                   curr_time, IO_flow->ID().c_str(), IO_flow->Get_generated_request_count(),
                   IO_flow->Get_serviced_request_count());
        log_debug ("STATS (%llu) device response time:%u (us) end-to-end request delay:%u (us)",
                   curr_time, IO_flow->Get_device_response_time(),
                   IO_flow->Get_end_to_end_request_delay());
    }
}

EXTERN_C uint64_t getSimTime(system_t *handle) {
    SCOPED_INSERTER(HostMap, get_tid(), GET_HOST(handle));
    SCOPED_INSERTER(EngineMap, get_tid(), GET_ENGINE(handle));
    return Simulator->Time();
}

void print_handle(system_t *handle) {
    printf("[%llu] handle %p host %p ssd %p engine %p data_store %p\n",
           get_tid(), handle, GET_HOST(handle), GET_SSD(handle), GET_ENGINE(handle), GET_DATASTORE(handle));
}

EXTERN_C MQSimEngine::Sim_Object *getIntegrationFlowInstance() {
    auto it = HostMap.find(get_tid());
    return ((it.first != 0) ? it.second->Get_Integration_IO_Flow() : nullptr);
}

EXTERN_C MQSimEngine::Engine *getEngineInstance() {
    const auto &it = EngineMap.find(get_tid());
    return ((it.first != 0) ? it.second : nullptr);
}

EXTERN_C void *getDataStoreInstance() {
    const auto &it = DataStoreMap.find(get_tid());
    return ((it.first != 0) ? it.second : nullptr);
}

EXTERN_C void *getNextStoreEntry(system_t *handle) {
    if (!is_store_empty(GET_DATASTORE(handle))) {
        return pop_head(GET_DATASTORE(handle));
    }
    return nullptr;
}
