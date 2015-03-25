#ifndef VIENNAUTILS_ENUM_PP_HPP
#define VIENNAUTILS_ENUM_PP_HPP

#include <boost/lexical_cast.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_enum.hpp>

#include "viennautils/detail/enum_pp.hpp"
#include "viennautils/exception.hpp"

/* Usage:
 * the macro
 * 
 * VIENNAUTILS_ENUM_PP(element_tag, (edge,2),(triangle),(rectangle),(polygon,6));
 * 
 * turns into:
 * 
 * struct element_tag
 * {
 *   enum type
 *   {
 *     edge = 2,
 *     triangle,
 *     rectangle,
 *     polygon = 6
 *   };
 *   
 *   //defined types:
 *   //  ValueSet      : contains all valid integer values of the enum
 *   //  ToStringMap   : maps enum value to string value
 *   //  FromStringMap : maps string value to enum value
 * 
 *   //defined functions:
 *   //  name()             : returns the name of the enum (e.g. "element_tag")
 *   //  value_set()        : returns the associated ValueSet
 *   //  to_string_map()    : returns the associated ToStringMap
 *   //  from_string_map()  : returns the associated FromStringMap
 * };
 * 
 * and provides the following (free!) main template functions (that take the enum type itself and _not_ the struct as a parameter):
 *   name()
 *   is_defined_enum_value()
 *   to_string()
 *   from_string()
 * 
 * as well as additional functions (should typically not be necessary):
 *   value_set()
 *   to_string_map()
 *   from_string_map()
 */

#define VIENNAUTILS_ENUM_PP(enum_name, ...) \
  VIENNAUTILS_X_ENUM_PP(enum_name, __VA_ARGS__)

namespace viennautils
{
namespace enum_pp
{

struct bad_conversion : virtual exception {};

template <typename T>
typename boost::enable_if<boost::is_enum<T>, std::string>::type const & name();

template <typename T, typename IntType>
typename boost::enable_if<boost::is_enum<T>, bool>::type is_valid(IntType value);

template <typename T>
typename boost::enable_if<boost::is_enum<T>, std::string>::type const & to_string(T value);

template <typename T>
typename boost::enable_if<boost::is_enum<T>, T>::type from_string(std::string const & str);

template <typename T>
typename boost::enable_if<boost::is_enum<T>, typename detail::value_set_link<T>::ValueSet>::type const & value_set();

template <typename T>
typename boost::enable_if<boost::is_enum<T>, typename detail::string_map_link<T>::ToStringMap>::type const & to_string_map();

template <typename T>
typename boost::enable_if<boost::is_enum<T>, typename detail::string_map_link<T>::FromStringMap>::type const & from_string_map();

//------------------------------------------------------------------------------------------------
//              Implementation
//------------------------------------------------------------------------------------------------

template <typename T>
typename boost::enable_if<boost::is_enum<T>, std::string>::type const & name()
{
  return detail::name_link<T>::name();
}

template <typename T, typename IntType>
typename boost::enable_if<boost::is_enum<T>, bool>::type is_valid(IntType value)
{
  return value_set<T>().find(static_cast<T>(value)) != value_set<T>().end();
}

template <typename T>
typename boost::enable_if<boost::is_enum<T>, std::string>::type const & to_string(T value)
{
  typename detail::string_map_link<T>::ToStringMap::const_iterator it = to_string_map<T>().find(value);
  if (it == to_string_map<T>().end())
  {
    throw make_exception<bad_conversion>("cannot convert invalid value: " + boost::lexical_cast<std::string>(value) + " of enum: " + name<T>() + " to string");
  }
  return it->second;
}

template <typename T>
typename boost::enable_if<boost::is_enum<T>, T>::type from_string(std::string const & str)
{
  typename detail::string_map_link<T>::FromStringMap::const_iterator it = from_string_map<T>().find(str);
  if (it == from_string_map<T>().end())
  {
    throw make_exception<bad_conversion>("cannot convert invalid string: \"" + str + "\" to enum value for enum: " + name<T>());
  }
  return it->second;
}

template <typename T>
typename boost::enable_if<boost::is_enum<T>, typename detail::value_set_link<T>::ValueSet>::type const & value_set()
{
  return detail::value_set_link<T>::value_set();
}

template <typename T>
typename boost::enable_if<boost::is_enum<T>, typename detail::string_map_link<T>::ToStringMap>::type const & to_string_map()
{
  return detail::string_map_link<T>::to_string_map();
}

template <typename T>
typename boost::enable_if<boost::is_enum<T>, typename detail::string_map_link<T>::FromStringMap>::type const & from_string_map()
{
  return detail::string_map_link<T>::from_string_map();
}

} //end of namespace enum_pp

} //end of namespace viennautils

#endif
