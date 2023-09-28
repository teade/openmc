#include "openmc/settings.h"

#include <cmath>  // for ceil, pow
#include <limits> // for numeric_limits
#include <string>
#include "n1s_lib_api.h"
#include "openmc/n1s.h"
#include "openmc/xml_interface.h"


namespace openmc {

namespace model {

 vector<unique_ptr<n1s_info>> n1s_data;

}

n1s_info::n1s_info(pugi::xml_node node) 
{

    if (check_for_node(node, "irradiation_times")) {
      auto times = get_node_array<double>(node, "irradiation_times");
      x_ = times;
    }
    if (check_for_node(node, "irradiation_intensity")) {
      auto intensity = get_node_array<double>(node, "irradiation_intensity");
      p_ = intensity;
    }
    if(check_for_node(node, "decay_times")) {
      auto decay_times = get_node_array<double>(node, "decay_times");
      t_ = decay_times;
    }

    c_irrad_sch_maker(x_.size(), &x_[0], &p_[0]); 

    if (check_for_node(node, "decay_files")) {
        for (pugi::xml_node file_node : node.children("decay_files")) {
            std::stringstream ss;
            file_node.print(ss);
            std::cout << ss.str() << std::endl;
            if (check_for_node(file_node, "decay_index")) {
                auto file_name = get_node_value(file_node, "decay_index");
                decay_index_ = file_name;
            }

            if (check_for_node(file_node, "decay_library")) {
                auto file_name = get_node_value(file_node, "decay_library");
                decay_library_ = file_name;
            }

            if (check_for_node(file_node, "metastable_file")) {
                auto file_name = get_node_value(file_node, "metastable_file");
                metastable_lib_ = file_name; 
            }

        }

    c_read_nuclear_data(decay_index_.c_str(), decay_library_.c_str(), metastable_lib_.c_str(), 0);

    }

}

// n1s_info n1s_setup;

}