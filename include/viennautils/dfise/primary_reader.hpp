#ifndef VIENNAUTILS_DFISE_GENERIC_READER_HPP
#define VIENNAUTILS_DFISE_GENERIC_READER_HPP

#include <string>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/utility/enable_if.hpp>

#include "viennautils/enum_pp.hpp"
#include "viennautils/dfise/parsing_error.hpp"
#include "viennautils/dfise/token_parser.hpp"

namespace viennautils
{
namespace dfise
{

/* DFISE file extensions:
 * .cel - cells
 * .ten - tensors
 * .bnd - boundary
 * .grd - grid
 * .dat - dataset
 * .plt - plots
 * .pro - properties
 */

class primary_reader
{
public:
  typedef boost::function<void (primary_reader &)> ParsingFunc;

  VIENNAUTILS_ENUM_PP(filetype, (grid),(dataset));

  struct mandatory_info
  {
    std::string version_;
    filetype::type type_;
    unsigned int dimension_;
    unsigned int nb_vertices_;
    unsigned int nb_edges_;
    unsigned int nb_faces_;
    unsigned int nb_elements_;
    unsigned int nb_regions_;
  };

  primary_reader( std::string const & filename
                , ParsingFunc const & additional_info_parsing_func
                , ParsingFunc const & data_block_parsing_func
                );

  mandatory_info const & get_mandatory_info() const {return mandatory_info_;}

  template <typename T>
  void read_value(T & target);

  template <typename T>
  void read_attribute(std::string const & name, T & target);

  //does not clear the array but rather uses push_back to add new values
  template <typename T>
  void read_array(std::string const & name, std::vector<T> & target);

  //does not clear the array but rather uses push_back to add new values
  template <typename T>
  void read_array(std::string const & name, std::vector<T> & target, typename std::vector<T>::size_type size);

  template <typename Para>
  void read_block(std::string const & name, boost::function<void (Para const &)> const & func);

  void read_block(std::string const & name, boost::function<void ()> const & func);

private:
  void parse_info_block(ParsingFunc const & additional_info_parsing_func);

  template <typename T>
  static typename boost::disable_if<boost::is_enum<T>, T>::type convert_to(std::string const & str);

  template <typename T>
  static typename boost::enable_if<boost::is_enum<T>, T>::type convert_to(std::string const & str);

  mandatory_info mandatory_info_;
  token_parser tp_;
};

//------------------------------------------------------------------------------------------------
//              Implementation
//------------------------------------------------------------------------------------------------

template <typename T>
void primary_reader::read_value(T & target)
{
  target = convert_to<T>(tp_.get_next());
}

template <typename T>
void primary_reader::read_attribute(std::string const & name, T & target)
{
  tp_.expect(name, "attribute has invalid name");
  tp_.expect("=", "attribute misses =");
  
  target = convert_to<T>(tp_.get_next());
}

template <typename T>
void primary_reader::read_array(std::string const & name, std::vector<T> & target)
{
  tp_.expect(name, "array has invalid name");
  tp_.expect("=", "attribute misses =");
  
  tp_.expect("[", "attribute is not an array");
  
  for (;;)
  {
    std::string const & token = tp_.get_next();
    if (token == "]")
    {
      break;
    }
    else
    {
      target.push_back(convert_to<T>(token));
    }
  }
}

template <typename T>
void primary_reader::read_array(std::string const & name, std::vector<T> & target, typename std::vector<T>::size_type size)
{
  tp_.expect(name, "array has invalid name");
  tp_.expect("=", "attribute misses =");
  
  tp_.expect("[", "attribute is not an array");
  target.reserve(target.size() + size);
  for (typename std::vector<T>::size_type i = 0; i < size; ++i)
  {
    target.push_back(convert_to<T>(tp_.get_next()));
  }
  tp_.expect("]", "array did not end as expected");
}

template <typename Para>
void primary_reader::read_block(std::string const & name, boost::function<void (Para const & )> const & func)
{
  tp_.expect(name, "block has invalid name");
  
  tp_.expect("(", "expected parameter parenthesis");
  Para p = convert_to<Para>(tp_.get_next());
  tp_.expect(")", "expected parameter to end");
  
  tp_.expect("{", "expected begin of block");
  func(p);
  tp_.expect("}", "expected end of block");
}

template <typename T>
typename boost::disable_if<boost::is_enum<T>, T>::type primary_reader::convert_to(std::string const & str)
{
  try
  {
    return boost::lexical_cast<T>(str);
  }
  catch(boost::bad_lexical_cast const &)
  {
    throw make_exception<parsing_error>("could not convert " + str + " to expected type");
  }
}

template <typename T>
typename boost::enable_if<boost::is_enum<T>, T>::type primary_reader::convert_to(std::string const & str)
{
  try
  {
    return viennautils::enum_pp::from_string<T>(str);
  }
  catch (boost::bad_lexical_cast const &)
  {
    throw make_exception<parsing_error>("could not convert " + str + " to expected type");
  }
}

} //end of namespace dfise

} //end of namespace viennautils

#endif
