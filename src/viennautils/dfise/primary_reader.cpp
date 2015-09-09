#include "viennautils/dfise/primary_reader.hpp"

#include <map>

namespace viennautils
{
namespace dfise
{

primary_reader::primary_reader( std::string const & filename
                              , ParsingFunc const & additional_info_parsing_func
                              , ParsingFunc const & data_block_parsing_func
                              )
                              : tp_(filename)
{
  //read header
  tp_.expect("DF-ISE", "invalid/unsupported file header");
  tp_.expect("text", "invalid/unsupported file header");
  
  read_block("Info", boost::bind(&primary_reader::parse_info_block, this, additional_info_parsing_func));
  read_block("Data", boost::bind(data_block_parsing_func, boost::ref(*this)));
}

void primary_reader::read_block(std::string const & name, boost::function<void ()> const & func)
{
  tp_.expect(name, "block has invalid name");
  tp_.expect("{", "expected begin of block");
  func();
  tp_.expect("}", "expected end of block");
}

void primary_reader::parse_info_block(ParsingFunc const & additional_info_parsing_func)
{
  read_attribute("version",     mandatory_info_.version_);
  
  { //TODO this used to use enum_pp (which was sadly kicked because of its C99 dependency)
    std::string tmp; 
    read_attribute("type",        tmp);
    if (tmp == "grid")
    {
      mandatory_info_.type_ = filetype_grid;
    }
    else if (tmp == "dataset")
    {
      mandatory_info_.type_ = filetype_dataset;
    }
    else
    {
      throw make_exception<parsing_error>("encountered unsupported filetype: " + tmp);
    }
  }
  
  read_attribute("dimension",   mandatory_info_.dimension_);
  read_attribute("nb_vertices", mandatory_info_.nb_vertices_);
  read_attribute("nb_edges",    mandatory_info_.nb_edges_);
  read_attribute("nb_faces",    mandatory_info_.nb_faces_);
  read_attribute("nb_elements", mandatory_info_.nb_elements_);
  read_attribute("nb_regions",  mandatory_info_.nb_regions_);
  
  additional_info_parsing_func(boost::ref(*this));
}

} //end of namespace dfise

} //end of namespace viennautils

/*
// code can be used to read parameters in arbitrary order - currently abandoned because fixed order is assumed
template <typename T>
boost::function<void ()> primary_reader::bind_read_attribute(T & target)
{
  return boost::bind(&primary_reader::read_attribute<T>, this, boost::ref(target));
}

template <typename T>
boost::function<void ()> primary_reader::bind_read_array(T & target)
{
  return boost::bind(&primary_reader::read_array<T>, this, boost::ref(target));
}

void primary_reader::parse_info_block()
{
  typedef std::map<std::string, boost::function<void ()> > AttributeHandlingMap;
  AttributeHandlingMap mandatory_attributes;
  mandatory_attributes["version"]     = bind_read_attribute(mandatory_info_.version_);
  mandatory_attributes["type"]        = bind_read_attribute(mandatory_info_.type_);
  mandatory_attributes["dimension"]   = bind_read_attribute(mandatory_info_.dimension_);
  mandatory_attributes["nb_vertices"] = bind_read_attribute(mandatory_info_.nb_vertices_);
  mandatory_attributes["nb_edges"]    = bind_read_attribute(mandatory_info_.nb_edges_);
  mandatory_attributes["nb_faces"]    = bind_read_attribute(mandatory_info_.nb_faces_);
  mandatory_attributes["nb_elements"] = bind_read_attribute(mandatory_info_.nb_elements_);
  mandatory_attributes["nb_regions"]  = bind_read_attribute(mandatory_info_.nb_regions_);
  
  std::set<std::string> already_parsed;
  
  while (!mandatory_attributes.empty())
  {
    std::string const & attribute = tp_.get_next();
    if (attribute == "}")
    {
      std::string missing_attributes;
      for(AttributeHandlingMap::const_iterator it = mandatory_attributes.begin(); it != mandatory_attributes.end(); ++it)
      {
        missing_attributes += it->first + " ";
      }
      throw make_exception<parsing_error>("expected further arguments in Info block: " + missing_attributes);
    }
    else 
    {
      AttributeHandlingMap::iterator it = mandatory_attributes.find(attribute);
      if (it == mandatory_attributes.end())
      {
        if (already_parsed.find(attribute) != already_parsed.end())
        {
          throw make_exception<parsing_error>("Info block argument: " + attribute + " specified twice");
        }
        else
        {
          throw make_exception<parsing_error>("unexpected Info block argument: " + attribute);
        }
      }
      else
      {
        it->second();
        already_parsed.insert(it->first);
        mandatory_attributes.erase(it);
      }
    }
  }
}
*/
