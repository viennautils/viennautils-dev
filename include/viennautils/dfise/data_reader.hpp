#ifndef VIENNAUTILS_DFISE_DATA_READER_HPP
#define VIENNAUTILS_DFISE_DATA_READER_HPP

#include <string>
#include <vector>
#include <map>

#include "viennautils/dfise/primary_reader.hpp"

namespace viennautils
{
namespace dfise
{

class primary_reader;

class data_reader
{
public:
  struct Dataset
  {
    std::vector<std::string> validity_;
    std::vector<double> values_;
  };
  typedef std::map<std::string, Dataset> DatasetMap;

  data_reader(std::string const & filename);

  DatasetMap const & get_datasets() const {return datasets_;}

private:
  struct DatasetInfo
  {
    std::vector<std::string> dataset_names_;
    std::vector<std::string> functions_;
  };

  void parse_additional_info(primary_reader & preader);
  void parse_data_block(primary_reader & preader);
  void parse_dataset_block(primary_reader & preader, std::vector<std::string>::size_type dataset_index, std::string const & para);
  void parse_dataset_values_block(primary_reader & preader, std::vector<double> & values, std::vector<double>::size_type const & para);

  DatasetInfo dataset_info_;
  DatasetMap datasets_;
};

} //end of namespace dfise

} //end of namespace viennautils

#endif
