#include "viennautils/filesystem/filesystem.hpp"

namespace viennautils
{

std::string extract_filename(std::string const & path)
{
  std::string path_delimiters = "\\/";
  
  std::size_t pos = path.find_last_of(path_delimiters);
  
  return (pos == std::string::npos) ? path : path.substr(pos+1);
}

std::string extract_path(std::string const & path)
{
  std::string path_delimiters = "\\/";
  
  std::size_t pos = path.find_last_of(path_delimiters);
  
  return (pos == std::string::npos) ? "" : path.substr(0, pos+1);
}

} //end of namespace viennautils
