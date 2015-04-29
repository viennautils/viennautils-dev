#include "viennautils/filesystem/filesystem.hpp"

namespace viennautils
{
namespace filesystem
{

namespace
{
std::string const path_delimiters = "\\/";

std::string::size_type find_last_path_delimiter(std::string const & path)
{
  return path.find_last_of(path_delimiters);
}

std::string::size_type find_extension_delimiter(std::string const & path)
{
  //we also have to look for path delimiters so that "a.b/file" doesn't yield pos 1 but rather std::string::npos
  std::string::size_type pos = path.find_last_of(path_delimiters + ".");
  
  return (pos == std::string::npos || path[pos] != '.') ? std::string::npos : pos;
}

}

std::string extract_extension(std::string const & path, bool include_delimiter)
{
  return path.substr(find_extension_delimiter(path) + (include_delimiter ? 0 : 1));
}

std::string extract_stem(std::string const & path)
{
  std::string::size_type last_path_delim = find_last_path_delimiter(path);
  std::string::size_type stem_start = (last_path_delim == std::string::npos) ? 0 : last_path_delim+1;
  std::string::size_type ext_delim = find_extension_delimiter(path);
  std::string::size_type stem_len = (ext_delim == std::string::npos) ? std::string::npos : ext_delim-stem_start;
  return path.substr(stem_start, stem_len);
}

std::string extract_filename(std::string const & path)
{
  std::string::size_type last_path_delim = find_last_path_delimiter(path);
  
  return (last_path_delim == std::string::npos) ? path : path.substr(last_path_delim+1);
}

std::string extract_path(std::string const & path, bool include_last_delimiter)
{
  std::string::size_type last_path_delim = find_last_path_delimiter(path);
  
  return (last_path_delim == std::string::npos) ? "" : path.substr(0, last_path_delim + (include_last_delimiter ? 1 : 0));
}

} //end of namespace filesystem
} //end of namespace viennautils
