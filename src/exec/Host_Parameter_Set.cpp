#include <algorithm>
#include "Host_Parameter_Set.h"

Host_Parameter_Set::Host_Parameter_Set()
{
    PCIe_Lane_Bandwidth = 0.4;//uint is GB/s
    PCIe_Lane_Count = 4;
    Enable_ResponseTime_Logging = false;
    ResponseTime_Logging_Period_Length = 400000;//nanoseconds
}

void Host_Parameter_Set::XML_serialize(Utils::XmlWriter& xmlwriter)
{
	std::string tmp;
	tmp = "Host_Parameter_Set";
	xmlwriter.Write_open_tag(tmp);

	std::string attr = "PCIe_Lane_Bandwidth";
	std::string val = std::to_string(PCIe_Lane_Bandwidth);
	xmlwriter.Write_attribute_string(attr, val);

	attr = "PCIe_Lane_Count";
	val = std::to_string(PCIe_Lane_Count);
	xmlwriter.Write_attribute_string(attr, val);

	attr = "SATA_Processing_Delay";
	val = std::to_string(SATA_Processing_Delay);
	xmlwriter.Write_attribute_string(attr, val);

	attr = "Enable_ResponseTime_Logging";
	val = (Enable_ResponseTime_Logging ? "true" : "false");
	xmlwriter.Write_attribute_string(attr, val);

	attr = "ResponseTime_Logging_Period_Length";
	val = std::to_string(ResponseTime_Logging_Period_Length);
	xmlwriter.Write_attribute_string(attr, val);

	xmlwriter.Write_close_tag();
}

void Host_Parameter_Set::XML_deserialize(rapidxml::xml_node<> *node)
{
	try {
		for (auto param = node->first_node(); param; param = param->next_sibling()) {
			if (strcmp(param->name(), "PCIe_Lane_Bandwidth") == 0) {
				std::string val = param->value();
				PCIe_Lane_Bandwidth = std::stod(val);
			} else if (strcmp(param->name(), "PCIe_Lane_Count") == 0) {
				std::string val = param->value();
				PCIe_Lane_Count = std::stoul(val);
			} else if (strcmp(param->name(), "SATA_Processing_Delay") == 0) {
				std::string val = param->value();
				SATA_Processing_Delay = std::stoul(val);
			} else if (strcmp(param->name(), "Enable_ResponseTime_Logging") == 0) {
				std::string val = param->value();
				std::transform(val.begin(), val.end(), val.begin(), ::toupper);
				Enable_ResponseTime_Logging = (val.compare("FALSE") == 0 ? false : true);
			} else if (strcmp(param->name(), "ResponseTime_Logging_Period_Length") == 0) {
				std::string val = param->value();
				ResponseTime_Logging_Period_Length = std::stoul(val);
			}
		}
	} catch (...) {
		PRINT_ERROR("Error in the Host_Parameter_Set!")
	}
}
