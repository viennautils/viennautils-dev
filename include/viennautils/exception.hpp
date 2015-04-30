#ifndef VIENNAUTILS_EXCEPTION_HPP
#define VIENNAUTILS_EXCEPTION_HPP

#include <stdexcept>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/enable_if.hpp>

namespace viennautils
{

struct exception : virtual std::exception
{
public:
  virtual ~exception() throw() {}

  virtual char const* what() const throw()
  {
      return what_.c_str();
  }

  void set_what(std::string const & str)
  {
    what_ = str;
  }

private:
  std::string what_;
};

template <typename T>
typename boost::enable_if<boost::is_base_of<exception, T>, T>::type make_exception(std::string const & str)
{
  T e;
  e.set_what(str);
  return e;
}

} //end of namespace viennautils

#endif
