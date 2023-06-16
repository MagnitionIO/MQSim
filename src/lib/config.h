//
// Created by Khubaib Umer on 24/01/2023.
//

#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <sstream>
#include "lib/logger.h"

static void read_config_parameters(const char *ssd_config_file_path, Execution_Parameter_Set **exec_params) {
    std::ifstream ssd_config_file;
    ssd_config_file.open(ssd_config_file_path);

    if (!ssd_config_file) {
        log_error("%s", "The specified SSD configuration file does not exist.");
        log_error("%s", "[====================] Done!");
    } else {
        //Read input workload parameters
        std::string line((std::istreambuf_iterator<char>(ssd_config_file)),
                         std::istreambuf_iterator<char>());
        ssd_config_file >> line;
        if (line != "USE_INTERNAL_PARAMS") {
            rapidxml::xml_document<> doc;    // character type defaults to char
            char *temp_string = new char[line.length() + 1];
            strcpy(temp_string, line.c_str());
            doc.parse<0>(temp_string);
            rapidxml::xml_node<> *mqsim_config = doc.first_node("Execution_Parameter_Set");
            if (mqsim_config != nullptr) {
                (*exec_params)->XML_deserialize(mqsim_config);
            } else {
                log_error("%s", "Error in the SSD configuration file!");
                log_error("%s", "Using MQSim's default configuration.");
            }
        } else {
            log_warn("%s", "Using MQSim's default configuration.");
            log_warn("%s", "Writing the default configuration parameters to the expected configuration file.");

            Utils::XmlWriter xmlwriter;
            std::string tmp;
            xmlwriter.Open(ssd_config_file_path);
            (*exec_params)->XML_serialize(xmlwriter);
            xmlwriter.Close();
            log_info("%s", "[====================] Done!");
        }
    }

    ssd_config_file.close();
}


#endif //CONFIG_H
