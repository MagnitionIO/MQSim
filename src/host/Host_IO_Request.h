#ifndef HOST_IO_REQUEST_H
#define HOST_IO_REQUEST_H

#include <sstream>
#include "../ssd/SSD_Defs.h"
#include "lib/base.h"

namespace Host_Components {
    enum class Host_IO_Request_Type {
        READ, WRITE
    };

    class Host_IO_Request {
    public:
        sim_time_type Arrival_time;//The time that the request has been generated
        sim_time_type Enqueue_time;//The time that the request enqueued into the I/O queue
        LHA_type Start_LBA;
        unsigned int LBA_count;
        Host_IO_Request_Type Type;
        uint16_t IO_queue_info;
        uint16_t Source_flow_id;//Only used in SATA host interface
        request_id_t req_id;
        completion_info_t *info;

        [[nodiscard]] std::string str() const {
            std::ostringstream st;
            st  << "Request ID: " << req_id
                << "Arrival Time: " << Arrival_time
                << " Enqueue Time: " << Enqueue_time
                << " Start LBA: " << Start_LBA
                << " LBA Count: " << LBA_count
                << " Type: " << (int) Type
                << " IO Queue Info: " << IO_queue_info
                << " Source Flow ID: " << Source_flow_id << "\n";
            return st.str();
        }

        ~Host_IO_Request();
    };
}

#endif // !HOST_IO_REQUEST_H
