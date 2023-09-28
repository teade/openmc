#ifndef N1S_H
#define N1S_H

#include "pugixml.hpp"

#include "openmc/constants.h"
#include "openmc/memory.h" // for unique_ptr
#include "openmc/vector.h" // for vector
#include "n1s_lib_api.h"

namespace openmc{

class n1s_info;

namespace model {

    extern vector<unique_ptr<n1s_info>> n1s_data;

}

class n1s_info{
    
public:
  explicit n1s_info(pugi::xml_node node);

  const vector<double>& x() const { return x_; }
  const vector<double>& p() const { return p_; }
  const vector<double>& t() const { return t_; }
  const std::string& decay_index() const { return decay_index_; }
  const std::string& decay_library() const { return decay_library_; }
  const std::string& metastable_lib() const { return metastable_lib_; }

private:
  vector<double> x_;     //!< tabulated independent variable
  vector<double> p_;     //!< tabulated probability density
  vector<double> t_;     //!< decay times
  std::string decay_index_;
  std::string decay_library_;
  std::string metastable_lib_;
//    std::unique_ptr<double[]> irradiation_times;
//    std::unique_ptr<double[]> irradiation_intensities;
//    std::string path_decay_index;
//    std::string path_decay_files;
//    std::string path_metastable_file;

};

}

#endif // N1S_H