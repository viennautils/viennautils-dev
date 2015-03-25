#ifndef VIENNAUTILS_DFISE_PRIMARY_PARSER_HPP
#define VIENNAUTILS_DFISE_PRIMARY_PARSER_HPP

#include <string>
#include <vector>
#include <fstream>

namespace viennautils
{
namespace dfise
{

class token_parser
{
public:
  explicit token_parser(std::string const & filename);

  bool at_end() const;
  std::string const& get_next();
  void expect(std::string const & expected, std::string const & error_msg);

private:
  typedef std::vector<std::string> Components;

  std::ifstream file_;
  Components normalized_line_;
  Components::iterator current_;

  static bool is_whitespace(char c);
  static bool is_standalone(char c);
  static bool is_comment_token(char c);
  static bool is_string_separator(char c);
  static bool is_backslash(char c);
};

//------------------------------------------------------------------------------------------------
//              Implementation
//------------------------------------------------------------------------------------------------

inline bool token_parser::is_whitespace(char c)
{
  return (c == ' ') || (c == '\t') || (c == '\n');
}

inline bool token_parser::is_standalone(char c)
{
  return (c == '=') || (c == '{') | (c == '[') || (c == '(') | (c == ')') || (c == ']') | (c == '}');
}

inline bool token_parser::is_comment_token(char c)
{
  return c == '#';
}

inline bool token_parser::is_string_separator(char c)
{
  return c == '\"';
}

inline bool token_parser::is_backslash(char c)
{
  return c == '\\';
}

} //end of namespace dfise

} //end of namespace viennautils

#endif
