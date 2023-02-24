#ifndef IO_FLOW_TRACE_BASED_H
#define IO_FLOW_TRACE_BASED_H

#include <string>
#include <iostream>
#include <fstream>
#include "IO_Flow_Base.h"
#include "ASCII_Trace_Definition.h"
#include "lib/libmqsim.h"

namespace Host_Components
{
class IO_Flow_Trace_Based : public IO_Flow_Base
{
public:
	IO_Flow_Trace_Based(const sim_object_id_type &name, uint16_t flow_id, LHA_type start_lsa_on_device, LHA_type end_lsa_on_device, uint16_t io_queue_id,
						uint16_t nvme_submission_queue_size, uint16_t nvme_completion_queue_size, IO_Flow_Priority_Class::Priority priority_class, double initial_occupancy_ratio,
						std::string trace_file_path, Trace_Time_Unit time_unit, unsigned int total_replay_count, unsigned int percentage_to_be_simulated,
						HostInterface_Types SSD_device_type, PCIe_Root_Complex *pcie_root_complex, SATA_HBA *sata_hba,
						bool enabled_logging, sim_time_type logging_period, std::string logging_file_path);
	~IO_Flow_Trace_Based();
	Host_IO_Request *Generate_next_request() override;
	void NVMe_consume_io_request(Completion_Queue_Entry *) override;
	void SATA_consume_io_request(Host_IO_Request *) override;
	void Start_simulation() override;
	void Validate_simulation_config() override;
	void Execute_simulator_event(MQSimEngine::Sim_Event *) override;
	void Get_statistics(Utils::Workload_Statistics& stats, MQSimEngine::Sim_Object *dev, LPA_type(*Convert_host_logical_address_to_device_address)(MQSimEngine::Sim_Object *ins, LHA_type lha),
                        page_status_type(*Find_NVM_subunit_access_bitmap)(MQSimEngine::Sim_Object *ins, LHA_type lha)) override;

#ifdef BUILD_LIB
    Host_IO_Request* Generate_Sim_Request(request_type_t *req);
    void Execute_simulator_event(bool dummy, request_type_t *req);
#endif

private:
	Trace_Time_Unit time_unit;
	unsigned int percentage_to_be_simulated;
	std::string trace_file_path;
	std::ifstream trace_file;
	unsigned int total_replay_no, replay_counter;
	unsigned int total_requests_in_file;
	std::vector<std::string> current_trace_line;
	sim_time_type time_offset;
};
} // namespace Host_Components

#endif // !IO_FLOW_TRACE_BASED_H
