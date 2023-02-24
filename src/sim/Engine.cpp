#include <stdexcept>
#include <unistd.h>
#include "Engine.h"
#include "../src/lib/libmqsim.h"
#include "../utils/Logical_Address_Partitioning_Unit.h"
#include "IO_Flow_Integration_Based.h"

#ifndef SINGLETON
#define SINGLETON
#endif

#ifndef LIBMQ_TEST
#define LIBMQ_TEST
#endif

#include "lib/integration.h"
#include "IO_Flow_Integration_Based.h"

extern INTEGRATION_BIT;

namespace MQSimEngine {

    void Engine::Reset() {
        _EventList->Clear();
        _ObjectList.clear();
        _sim_time = 0;
        stop = false;
        started = false;
        Simulator->lapu->Reset();
    }

    //Add an object to the simulator object list
    void Engine::AddObject(Sim_Object *obj) {
        if (_ObjectList.find(obj->ID()) != _ObjectList.end()) {
            throw std::invalid_argument("Duplicate object key: " + obj->ID());
        }
        _ObjectList.insert(std::pair<sim_object_id_type, Sim_Object *>(obj->ID(), obj));
    }

    Sim_Object *Engine::GetObject(sim_object_id_type object_id) {
        auto itr = _ObjectList.find(object_id);
        if (itr == _ObjectList.end()) {
            return NULL;
        }

        return (*itr).second;
    }

    void Engine::RemoveObject(Sim_Object *obj) {
        std::unordered_map<sim_object_id_type, Sim_Object *>::iterator it = _ObjectList.find(obj->ID());
        if (it == _ObjectList.end()) {
            throw std::invalid_argument("Removing an unregistered object.");
        }
        _ObjectList.erase(it);
    }

    void Engine::report_request_complete(completion_info_t *info)
    {
        done_queue.emplace(info);
    }

    void Engine::enqueue_completed_request_ids(insert_to_queue_f insert)
    {
        for (const auto &rid : done_queue)
        {
            insert(getDataStoreInstance(), rid);
        }
        done_queue.clear();
    }

    bool Engine::process_request(void *req, uint64_t *required_time, uint64_t curr_time, insert_to_queue_f queue)
    {
        request_type_t * _req = nullptr;
        _sim_time = curr_time;
        if (req) {
            _req = (request_type_t*)req;
            auto io_flow_ins = dynamic_cast<Host_Components::IO_Flow_Integration_Based*>(getIntegrationFlowInstance());
            Simulator->Register_sim_event(_req->arrival_time, io_flow_ins);
            io_flow_ins->Create_simulator_event(_req);
        }
        if (_EventList->Count == 0) {
            return false;
        }

        Sim_Event *ev = NULL;
        EventTreeNode *minNode = _EventList->Get_min_node();
        ev = minNode->FirstSimEvent;

        if (curr_time < ev->Fire_time)
        {
            // If this happens go back and reschedule
            *required_time = ev->Fire_time;
            return true;
        }

        _sim_time = ev->Fire_time; // Fast-forward simulator time

        while (ev != NULL) {
            if (!ev->Ignore) {
                ev->Target_sim_object->Execute_simulator_event(ev);
            }
            Sim_Event *consumed_event = ev;
            ev = ev->Next_event;
            delete consumed_event;
        }
        _EventList->Remove(minNode);

        bool ret = true;
        if (_EventList->Count != 0) {
            *required_time = _EventList->Get_min_node()->FirstSimEvent->Fire_time;
        }
        else
        {
            *required_time = 0;
            ret = false;
        }
        enqueue_completed_request_ids(queue);
        return ret;
    }

    void Engine::initialize()
    {
        for (auto &obj: _ObjectList) {
            if (!obj.second->IsTriggersSetUp()) {
                obj.second->Setup_triggers();
            }
        }

        for (auto &obj: _ObjectList) {
            obj.second->Validate_simulation_config();
        }

        for (auto &obj: _ObjectList) {
            obj.second->Start_simulation();
        }
    }

    /// This is the main method of simulator which starts simulation process.
    void Engine::Start_simulation() {
        started = true;

        for (std::unordered_map<sim_object_id_type, Sim_Object *>::iterator obj = _ObjectList.begin();
             obj != _ObjectList.end();
             ++obj) {
            if (!obj->second->IsTriggersSetUp()) {
                obj->second->Setup_triggers();
            }
        }

        for (std::unordered_map<sim_object_id_type, Sim_Object *>::iterator obj = _ObjectList.begin();
             obj != _ObjectList.end();
             ++obj) {
            obj->second->Validate_simulation_config();
        }

        for (std::unordered_map<sim_object_id_type, Sim_Object *>::iterator obj = _ObjectList.begin();
             obj != _ObjectList.end();
             ++obj) {
            obj->second->Start_simulation();
        }

        Sim_Event *ev = NULL;
        while (true) {
            if (_EventList->Count == 0 || stop) {
                break;
            }

            EventTreeNode *minNode = _EventList->Get_min_node();
            ev = minNode->FirstSimEvent;

            _sim_time = ev->Fire_time; // Fast-forward simulator time

            while (ev != NULL) {
                if (!ev->Ignore) {
                    ev->Target_sim_object->Execute_simulator_event(ev);
                }
                Sim_Event *consumed_event = ev;
                ev = ev->Next_event;
                delete consumed_event;
            }
            _EventList->Remove(minNode);
        }
    }

    void Engine::Stop_simulation() {
        stop = true;
    }

    sim_time_type Engine::Time() {
        return _sim_time;
    }

    Sim_Event *
    Engine::Register_sim_event(sim_time_type fireTime, Sim_Object *targetObject, void *parameters, int type) {
        Sim_Event *ev = new Sim_Event(fireTime, targetObject, parameters, type);
        DEBUG("RegisterEvent " << fireTime << " " << targetObject)
        _EventList->Insert_sim_event(ev);
        return ev;
    }

    void Engine::Ignore_sim_event(Sim_Event *ev) {
        ev->Ignore = true;
    }

    bool Engine::Has_started() {
        return started;
    }

    bool Engine::Is_integrated_execution_mode() {
        return false;
    }
}