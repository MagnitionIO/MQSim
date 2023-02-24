#include "User_Request.h"
#include <atomic>

namespace SSD_Components
{
	User_Request::User_Request() : Sectors_serviced_from_cache(0)
	{
		ID = "" + std::to_string(lastId++);
		ToBeIgnored = false;
	}
}
