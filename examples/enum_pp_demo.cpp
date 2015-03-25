#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

#include "viennautils/enum_pp.hpp"

VIENNAUTILS_ENUM_PP(element_tag, (edge,1),(triangle),(rectangle,4));

template <typename T>
void foo(T my_tag)
{
  std::cout << "name of enum: " << viennautils::enum_pp::name<T>() << std::endl;
  for ( element_tag::ToStringMap::const_iterator it = viennautils::enum_pp::to_string_map<T>().begin()
      ; it != viennautils::enum_pp::to_string_map<T>().end()
      ; ++it
      )
  {
    std::cout << "value: " << it->first << " name: " << it->second << std::endl;
  }
  
  std::cout << "using value 'rectangle' to retrieve string: " << viennautils::enum_pp::to_string(my_tag) << std::endl;
  std::cout << "using string \"rectangle\" to retrieve value: " << viennautils::enum_pp::from_string<T>("rectangle") << std::endl;
  
  try
  {
    T fail = viennautils::enum_pp::from_string<T>("not a valid enum value");
  }
  catch(std::exception const & e)
  {
    std::cout << "caught expected exception: " << e.what() << std::endl;
  }
}

int main()
{
  //notice how T is of type element_tag::type in foo and not element_tag itself
  //so the enum itself is associated with the functions that it needs
  foo(element_tag::rectangle);
}
