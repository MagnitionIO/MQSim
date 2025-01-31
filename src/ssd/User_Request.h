#ifndef USER_REQUEST_H
#define USER_REQUEST_H

#include <string>
#include <list>
#include "SSD_Defs.h"
#include "../sim/Sim_Defs.h"
#include "Host_Interface_Defs.h"
#include "NVM_Transaction.h"
#include <sstream>

namespace SSD_Components
{
	enum class UserRequestType { READ, WRITE };
	class NVM_Transaction;
	class User_Request
	{
	public:
		User_Request();
		IO_Flow_Priority_Class::Priority Priority_class;
		io_request_id_type ID;
		LHA_type Start_LBA;

		sim_time_type STAT_InitiationTime;
		sim_time_type STAT_ResponseTime;
		std::list<NVM_Transaction*> Transaction_list;
		unsigned int Sectors_serviced_from_cache;

		unsigned int Size_in_byte;
		unsigned int SizeInSectors;
		UserRequestType Type;
		stream_id_type Stream_id;
		bool ToBeIgnored;
		void* IO_command_info;//used to store host I/O command info
		void* Data;

        [[nodiscard]] std::string str() const {
            std::ostringstream st;
            st << "**User_Request** Priority Class: " << Priority_class << " ID: " << ID
            << " Start LBA: " << Start_LBA << " STAT Init Time: " << STAT_InitiationTime
            << " STAT Response Time: " << STAT_ResponseTime << " Sectors from Cache" << Sectors_serviced_from_cache
            << " Size Bytes: " << Size_in_byte << " Size Sectors: " << SizeInSectors
            << " Type: " << (int)Type << " Stream ID: " << Stream_id << "\n";
            return st.str();
        }
	private:
		std::atomic<unsigned int> lastId = ATOMIC_VAR_INIT(0);
	};
}

#endif // !USER_REQUEST_H
