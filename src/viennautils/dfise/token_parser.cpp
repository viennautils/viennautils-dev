#include "viennautils/dfise/token_parser.hpp"

#include "viennautils/dfise/parsing_error.hpp"

namespace viennautils
{
namespace dfise
{

token_parser::token_parser( std::string const & filename
                          )
                          : file_(filename.c_str())
                          , normalized_line_()
                          , current_(normalized_line_.begin())
{
  if (!file_)
  {
    throw make_exception<parsing_error>("cannot open file " +  filename);
  }
}

bool token_parser::at_end() const
{
  return !file_.good();
}

std::string const & token_parser::get_next()
{
  while (current_ == normalized_line_.end())
  {
    if (at_end())
    {
      throw make_exception<parsing_error>("unexpectedly reached end of file");
    }
    
    normalized_line_.clear();
    
    std::string line;
    std::getline(file_, line);
    
    std::string::size_type start = 0;
    //skip first whitespaces
    while (start != line.size() && is_whitespace(line[start]))
    {
      ++start;
    }
    
    while (start < line.size())
    {
      if (is_comment_token(line[start]))
      {
        break;
      }
      else if(is_standalone(line[start]))
      {
        normalized_line_.push_back(line.substr(start,1));
        ++start;
      }
      else 
      {
        //reading more than a single char
        std::string::size_type end = start+1;
        
        if (is_string_separator(line[start]))
        {
          //reading a quoted string
          std::string multi_line;
          for (;; ++end)
          {
            while(end >= line.size())
            {
              multi_line += line.substr(start) + "\n";
              
              if (at_end())
              {
                throw make_exception<parsing_error>("unexpectedly reached end of file");
              }
              std::getline(file_, line);
              start = 0;
              end = 0;
            }
            
            if(is_string_separator(line[end]))
            {
              ++end;
              normalized_line_.push_back(multi_line + line.substr(start, end-start));
              break;
            }
            
            if(is_backslash(line[end]))
            {
              //ignore next char
              ++end;
            }
          }
        }
        else
        {
          //reading anything but a quoted string
          while (end < line.size() && !is_comment_token(line[end]) && !is_whitespace(line[end]) && !is_standalone(line[end]))
          {
            ++end;
          }
          normalized_line_.push_back(line.substr(start, end-start));
        }
        
        start = end;
      }
      while (start != line.size() && is_whitespace(line[start]))
      {
        ++start;
      }
    }
    current_ = normalized_line_.begin();
  }
  
  return *(current_++);
}

void token_parser::expect(std::string const & expected, std::string const & error_msg)
{
  std::string const & next = get_next();
  if(next != expected)
  {
    throw make_exception<parsing_error>(error_msg + " expected: " + expected + " got: " + next);
  }
}

} //end of namespace dfise

} //end of namespace viennautils
