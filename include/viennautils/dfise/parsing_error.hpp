#ifndef VIENNAUTILS_DFISE_EXCEPTIONS_HPP
#define VIENNAUTILS_DFISE_EXCEPTIONS_HPP

#include "viennautils/exception.hpp"

namespace viennautils
{
namespace dfise
{

struct parsing_error : virtual viennautils::exception {};

} //end of namespace dfise

} //end of namespace viennautils

#endif
