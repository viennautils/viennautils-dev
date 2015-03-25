#ifndef VIENNAUTILS_DETAIL_ENUM_PP_HPP
#define VIENNAUTILS_DETAIL_ENUM_PP_HPP

#include <string>
#include <cassert>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/expand.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/size.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#include <boost/unordered_map.hpp>
#include <boost/container/flat_set.hpp>

namespace viennautils
{
namespace enum_pp
{
namespace detail
{

template <typename T>
class name_link
{
public:
  static std::string const & name()
  {
    //assertion fails if T was not defined by our custom macro
    assert(name_);
    return *name_;
  }

  static void assign(std::string const & name)
  {
    name_ = &name;
  }

private:
  static std::string const * name_;
};

template <typename T>
std::string const * name_link<T>::name_ = NULL;

template <typename T>
struct name_link_assigner
{
  static bool invoke_assign()
  {
    name_link<typename T::type>::assign(T::name());
    return true;
  }

  static bool const invoke_assign_helper_;
};

template <typename T>
bool const name_link_assigner<T>::invoke_assign_helper_ = name_link_assigner<T>::invoke_assign();

template <typename T>
class value_set_link
{
public:
  typedef boost::container::flat_set<T> ValueSet;

  static ValueSet const & value_set()
  {
    assert(value_set_);
    return *value_set_;
  }

  static void assign(ValueSet const & value_set)
  {
    value_set_ = &value_set;
  }

private:
  static ValueSet const * value_set_;
};

template <typename T>
typename value_set_link<T>::ValueSet const * value_set_link<T>::value_set_ = NULL;

template <typename T>
struct value_set_link_assigner
{
  static bool invoke_assign()
  {
    value_set_link<typename T::type>::assign(T::value_set());
    return true;
  }

  static bool const invoke_assign_helper_;
};

template <typename T>
bool const value_set_link_assigner<T>::invoke_assign_helper_ = value_set_link_assigner<T>::invoke_assign();

template <typename T>
class string_map_link
{
public:
  typedef boost::unordered_map<T, std::string> ToStringMap;
  typedef boost::unordered_map<std::string, T> FromStringMap;

  static ToStringMap const & to_string_map()
  {
    //assertion fails if T was not defined by our custom macro
    assert(to_string_map_);
    return *to_string_map_;
  }

  static FromStringMap const & from_string_map()
  {
    //assertion fails if T was not defined by our custom macro
    assert(from_string_map_);
    return *from_string_map_;
  }

  static void assign(ToStringMap const & to_string_map, FromStringMap const & from_string_map)
  {
    to_string_map_ = &to_string_map;
    from_string_map_ = &from_string_map;
  }

private:
  static ToStringMap const * to_string_map_;
  static FromStringMap const * from_string_map_;
};

template <typename T>
typename string_map_link<T>::ToStringMap const * string_map_link<T>::to_string_map_ = NULL;

template <typename T>
typename string_map_link<T>::FromStringMap const * string_map_link<T>::from_string_map_ = NULL;

template <typename T>
struct string_map_link_assigner
{
  static bool invoke_assign()
  {
    string_map_link<typename T::type>::assign(T::to_string_map(), T::from_string_map());
    return true;
  }

  static bool const invoke_assign_helper_;
};

template <typename T>
bool const string_map_link_assigner<T>::invoke_assign_helper_ = string_map_link_assigner<T>::invoke_assign();

} //end of namespace detail

} //end of namespace enum_pp

} //end of namespace viennautils

#define VIENNAUTILS_X_ENUM_PP__NAME_VALUE_1(elem) \
  BOOST_PP_TUPLE_ELEM(0,elem)

#define VIENNAUTILS_X_ENUM_PP__NAME_VALUE_2(elem) \
  BOOST_PP_TUPLE_ELEM(0,elem) = BOOST_PP_TUPLE_ELEM(1,elem)

#define VIENNAUTILS_X_ENUM_PP__NAME_VALUE_SEQ(s,data,elem) \
  BOOST_PP_EXPAND(BOOST_PP_CAT(VIENNAUTILS_X_ENUM_PP__NAME_VALUE_,BOOST_PP_TUPLE_SIZE(elem))(elem))

#define VIENNAUTILS_X_ENUM_PP__NAME_ONLY_SEQ(s,data,elem) \
  BOOST_PP_TUPLE_ELEM(0,elem)

#define VIENNAUTILS_X_ENUM_PP__TO_SEQ(seq_type, ...)                                        \
  BOOST_PP_SEQ_TRANSFORM( BOOST_PP_CAT(BOOST_PP_CAT(VIENNAUTILS_X_ENUM_PP__,seq_type),_SEQ) \
                        , 0                                                                 \
                        , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                             \
                        )

#define VIENNAUTILS_X_ENUM_PP__VALUE_SET(r, data, elem) \
  value_set.insert(elem);

#define VIENNAUTILS_X_ENUM_PP__TO_STRING_MAPPING(r, data, elem) \
  to_string_map[elem] = BOOST_PP_STRINGIZE(elem);

#define VIENNAUTILS_X_ENUM_PP__FROM_STRING_MAPPING(r, data, elem) \
  from_string_map[BOOST_PP_STRINGIZE(elem)] = elem;

#ifdef _MSC_VER
  #define VIENNAUTILS_X_ENUM_PP__DO_NOT_OPTIMIZE_AWAY __declspec(dllexport)
#else
  #define VIENNAUTILS_X_ENUM_PP__DO_NOT_OPTIMIZE_AWAY
#endif

#define VIENNAUTILS_X_ENUM_PP(enum_name, ...)                                                          \
  struct enum_name                                                                                     \
  {                                                                                                    \
    enum type                                                                                          \
    {                                                                                                  \
      BOOST_PP_SEQ_ENUM(VIENNAUTILS_X_ENUM_PP__TO_SEQ(NAME_VALUE, __VA_ARGS__))                        \
    };                                                                                                 \
                                                                                                       \
    static std::string const & name()                                                                  \
    {                                                                                                  \
      static std::string const name = BOOST_PP_STRINGIZE(enum_name);                                   \
      return name;                                                                                     \
    }                                                                                                  \
                                                                                                       \
    typedef viennautils::enum_pp::detail::value_set_link<type>::ValueSet ValueSet;                     \
    static ValueSet const & value_set()                                                                \
    {                                                                                                  \
      struct local_scope                                                                               \
      {                                                                                                \
        static ValueSet init()                                                                         \
        {                                                                                              \
          ValueSet value_set;                                                                          \
                                                                                                       \
          BOOST_PP_SEQ_FOR_EACH( VIENNAUTILS_X_ENUM_PP__VALUE_SET                                      \
                               , enum_name                                                             \
                               , VIENNAUTILS_X_ENUM_PP__TO_SEQ(NAME_ONLY, __VA_ARGS__)                 \
                               )                                                                       \
                                                                                                       \
          return value_set;                                                                            \
        }                                                                                              \
      };                                                                                               \
      static ValueSet const value_set = local_scope::init();                                           \
      return value_set;                                                                                \
    }                                                                                                  \
                                                                                                       \
    typedef viennautils::enum_pp::detail::string_map_link<type>::ToStringMap ToStringMap;              \
    static ToStringMap const& to_string_map()                                                          \
    {                                                                                                  \
      struct local_scope                                                                               \
      {                                                                                                \
        static ToStringMap init()                                                                      \
        {                                                                                              \
          ToStringMap to_string_map;                                                                   \
                                                                                                       \
          BOOST_PP_SEQ_FOR_EACH( VIENNAUTILS_X_ENUM_PP__TO_STRING_MAPPING                              \
                                , enum_name                                                            \
                                , VIENNAUTILS_X_ENUM_PP__TO_SEQ(NAME_ONLY, __VA_ARGS__)                \
                                )                                                                      \
                                                                                                       \
          return to_string_map;                                                                        \
        }                                                                                              \
      };                                                                                               \
      static ToStringMap const to_string_map = local_scope::init();                                    \
      return to_string_map;                                                                            \
    }                                                                                                  \
                                                                                                       \
    typedef viennautils::enum_pp::detail::string_map_link<type>::FromStringMap FromStringMap;          \
    static FromStringMap const& from_string_map()                                                      \
    {                                                                                                  \
      struct local_scope                                                                               \
      {                                                                                                \
        static FromStringMap init()                                                                    \
        {                                                                                              \
          FromStringMap from_string_map;                                                               \
                                                                                                       \
          BOOST_PP_SEQ_FOR_EACH( VIENNAUTILS_X_ENUM_PP__FROM_STRING_MAPPING                            \
                                , enum_name                                                            \
                                , VIENNAUTILS_X_ENUM_PP__TO_SEQ(NAME_ONLY, __VA_ARGS__)                \
                                )                                                                      \
                                                                                                       \
          return from_string_map;                                                                      \
        }                                                                                              \
      };                                                                                               \
      static FromStringMap const from_string_map = local_scope::init();                                \
      return from_string_map;                                                                          \
    }                                                                                                  \
                                                                                                       \
  private:                                                                                             \
    VIENNAUTILS_X_ENUM_PP__DO_NOT_OPTIMIZE_AWAY                                                        \
    bool never_called_but_forces_initialization()                                                      \
    {                                                                                                  \
      return viennautils::enum_pp::detail::name_link_assigner<enum_name>::invoke_assign_helper_        \
          && viennautils::enum_pp::detail::value_set_link_assigner<enum_name>::invoke_assign_helper_   \
          && viennautils::enum_pp::detail::string_map_link_assigner<enum_name>::invoke_assign_helper_; \
    }                                                                                                  \
  }

#endif
