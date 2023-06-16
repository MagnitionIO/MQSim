//
// Created by Khubaib Umer on 16/12/2022.
//

#ifndef MQSIM_IO_FLOW_INTEGRATION_BASED_H
#define MQSIM_IO_FLOW_INTEGRATION_BASED_H

#include "ASCII_Trace_Definition.h"
#include "IO_Flow_Base.h"
#include "../lib/libmqsim.h"
#include <fstream>
#include <iostream>
#include <string>

namespace Host_Components {
    class IO_Flow_Integration_Based : public IO_Flow_Base {
    public:
        IO_Flow_Integration_Based(const sim_object_id_type &name, uint16_t flow_id, LHA_type start_lsa_on_device,
                                  LHA_type end_lsa_on_device, uint16_t io_queue_id,
                                  uint16_t nvme_submission_queue_size, uint16_t nvme_completion_queue_size,
                                  IO_Flow_Priority_Class::Priority priority_class, double initial_occupancy_ratio,
                                  Trace_Time_Unit time_unit,
                                  HostInterface_Types SSD_device_type, PCIe_Root_Complex *pcie_root_complex,
                                  SATA_HBA *sata_hba,
                                  bool enabled_logging, sim_time_type logging_period, std::string logging_file_path);

        ~IO_Flow_Integration_Based() = default;

        Host_IO_Request *Generate_next_request() override;

        void NVMe_consume_io_request(Completion_Queue_Entry *) override;

        void SATA_consume_io_request(Host_IO_Request *) override;

        void Start_simulation() override;

        void Validate_simulation_config() override;

        void Execute_simulator_event(MQSimEngine::Sim_Event *) override;

        void Create_simulator_event(request_type_t *req);

        void Get_statistics(Utils::Workload_Statistics& stats, MQSimEngine::Sim_Object *dev, LPA_type(*Convert_host_logical_address_to_device_address)(MQSimEngine::Sim_Object *ins, LHA_type lha),
                                           page_status_type(*Find_NVM_subunit_access_bitmap)(MQSimEngine::Sim_Object *ins, LHA_type lha)) override;

    private:
        Trace_Time_Unit time_unit;
        unsigned int total_requests{};
        sim_time_type time_offset;
        Host_IO_Request *new_user_request{};
        sim_time_type last_req_arrival_time = 0;

    };
} // namespace Host_Components

#endif // MQSIM_IO_FLOW_INTEGRATION_BASED_H
