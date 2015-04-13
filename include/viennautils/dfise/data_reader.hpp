#ifndef VIENNAUTILS_DFISE_DATA_READER_HPP
#define VIENNAUTILS_DFISE_DATA_READER_HPP

#include <string>
#include <vector>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>

#include "viennautils/dfise/grid_reader.hpp"

namespace viennautils
{
namespace dfise
{

class primary_reader;

class data_reader
{
public:
  typedef std::vector<double> ValueVector;
  typedef std::vector<viennautils::dfise::grid_reader::VertexIndex> VertexIndexVector;

  //name, dimension, values
  typedef std::map<std::string, std::pair<unsigned int, std::pair<VertexIndexVector, ValueVector> > > PartialDatasetMap;
  typedef std::map<std::string, std::pair<unsigned int, ValueVector> > CompleteDatasetMap;
  
  data_reader(grid_reader const & greader);

  void read(std::string const & filename, PartialDatasetMap & partial_datasets, CompleteDatasetMap & complete_datasets);

private:
  struct Dataset;
  typedef std::list<Dataset> DatasetList;

  typedef boost::container::flat_set<grid_reader::VertexIndex> VertexIndexSet;
  typedef boost::container::flat_map<std::string, VertexIndexSet> RegionVertexIndicesMap;

  void parse_additional_info(primary_reader & preader, DatasetList & datasets);
  void parse_data_block(primary_reader & preader, DatasetList & datasets);

  void combine_region_indices(std::vector<std::string> const & validity, VertexIndexSet & combined_indices);

  unsigned int dimension_;
  unsigned int vertex_count_;
  unsigned int element_count_;
  RegionVertexIndicesMap region_vertex_indices_;

  static void parse_dataset_block(primary_reader & preader, Dataset & dataset, std::string const & para);
  static void parse_dataset_values_block(primary_reader & preader, std::vector<double> & values, std::vector<double>::size_type const & para);
};

} //end of namespace dfise

} //end of namespace viennautils

#endif
