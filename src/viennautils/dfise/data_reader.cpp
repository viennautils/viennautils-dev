#include "viennautils/dfise/data_reader.hpp"

#include "viennautils/dfise/parsing_error.hpp"
#include "viennautils/dfise/primary_reader.hpp"

namespace viennautils
{
namespace dfise
{

data_reader::data_reader(std::string const & filename)
{
  primary_reader preader( filename
                        , boost::bind(&data_reader::parse_additional_info, this, _1)
                        , boost::bind(&data_reader::parse_data_block, this, _1)
                        );
}

void data_reader::parse_additional_info(primary_reader & preader)
{
  if(preader.get_mandatory_info().type_ != primary_reader::filetype::dataset)
  {
    throw make_exception<parsing_error>( "invalid file type: " + enum_pp::to_string(preader.get_mandatory_info().type_)
                                       + " - grid_reader parses " + enum_pp::to_string(primary_reader::filetype::grid) + "  files only"
                                       );
  }
  preader.read_array("datasets", dataset_info_.dataset_names_);
  preader.read_array("functions", dataset_info_.functions_);
}

void data_reader::parse_data_block(primary_reader & preader)
{
  for (std::vector<std::string>::size_type i = 0; i < dataset_info_.dataset_names_.size(); ++i)
  {
    preader.read_block<std::string>("Dataset", boost::bind(&data_reader::parse_dataset_block, this, boost::ref(preader), i, _1));
  }
}

namespace
{
void expect(primary_reader & preader, std::string const & attribute_name, std::string const & expected_value)
{
  std::string value;
  preader.read_attribute(attribute_name, value);
  if (value != expected_value)
  {
    throw make_exception<parsing_error>("unexpected value for attribute: " + attribute_name + " expected: " + expected_value + " but got: " + value + " instead");
  }
}
}

void data_reader::parse_dataset_block(primary_reader & preader, std::vector<std::string>::size_type dataset_index, std::string const & para)
{
  std::string const & dataset_name = dataset_info_.dataset_names_[dataset_index];
  if (para != dataset_name)
  {
    throw make_exception<parsing_error>("unexpected dataset name: " + para + " - expected name: " + dataset_name);
  }
  
  //datareader currently only supports the following settings
  expect(preader, "function", dataset_info_.functions_[dataset_index]);
  expect(preader, "type", "scalar");
  expect(preader, "dimension", "1");
  expect(preader, "localion", "vertex");
  
  Dataset & dataset = datasets_[dataset_name];
  preader.read_array("validity", dataset.validity_);
  
  preader.read_block<std::vector<double>::size_type>("Values", boost::bind(&data_reader::parse_dataset_values_block, this, boost::ref(preader), boost::ref(dataset.values_), _1));
}

void data_reader::parse_dataset_values_block(primary_reader & preader, std::vector<double> & values, std::vector<double>::size_type const & para)
{
  values.resize(values.size() + para);
  for (std::vector<double>::size_type i = values.size() - para; i < values.size(); ++i)
  {
    preader.read_value(values[i]);
  }
}

} //end of namespace dfise

} //end of namespace viennautils
