#ifndef VIENNAUTILS_FILESYSTEM_FILESYSTEM_HPP
#define VIENNAUTILS_FILESYSTEM_FILESYSTEM_HPP

#include <string>

namespace viennautils
{
namespace filesystem
{

std::string extract_extension(std::string const & path, bool include_delimiter = false);
std::string extract_stem(std::string const & path);
std::string extract_filename(std::string const & path);
std::string extract_path(std::string const & path, bool include_last_delimiter = false);

} //end of namespace filesystem
} //end of namespace viennautils

#endif
