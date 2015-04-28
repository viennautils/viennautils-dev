#ifndef VIENNAUTILS_FILESYSTEM_FILESYSTEM_HPP
#define VIENNAUTILS_FILESYSTEM_FILESYSTEM_HPP

#include <string>

namespace viennautils
{

std::string extract_filename(std::string const & path);
std::string extract_path(std::string const & path);

} //end of namespace viennautils

#endif
