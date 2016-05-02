#ifndef VIENNAUTILS_DFISE_DATA_READER_HPP
#define VIENNAUTILS_DFISE_DATA_READER_HPP

#include <string>
#include <vector>
#include <list>
#include <map>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>

#include "viennautils/dfise/grd_bnd_reader.hpp"

namespace viennautils
{
namespace dfise
{

class primary_reader;

/* data_reader reads and combines datasets specified in .dat files in dfise text format
 * datasets with identical names within in a single file are combined to a single, final dataset (either partial or complete)
 * in case of duplicate validities for the same dataset in a single file, an exception is thrown
 * in case of duplicate datasets across multiple files, additional datasets are created that are named
 *  <dataset_name>_<filename>[_<count>] (count in case of multiple datafiles with the same filename)
 * 
 * example:
 *   file A.dat contains values for dataset D for region X
 *   file B.dat contains values for dataset D for region Y
 * result:
 *   2 partial datasets:
 *   partial dataset with name "D" contains values for region X from A.dat
 *   partial dataset with name "D_B" contains values for region Y from B.dat
 */
class data_reader
{
public:
  typedef std::vector<double> ValueVector;
  typedef std::vector<viennautils::dfise::grd_bnd_reader::VertexIndex> VertexIndexVector;

  //name, dimension, values
  typedef std::map<std::string, std::pair<unsigned int, std::pair<VertexIndexVector, ValueVector> > > PartialDatasetMap;
  typedef std::map<std::string, std::pair<unsigned int, ValueVector> > CompleteDatasetMap;

  data_reader(grd_bnd_reader const & gbreader);

  void read(std::string const & filepath);

  PartialDatasetMap const & get_partial_datasets() const {return partial_datasets_;}
  CompleteDatasetMap const & get_complete_datasets() const {return complete_datasets_;}

private:
  struct Dataset;
  typedef std::list<Dataset> DatasetList;

  typedef boost::container::flat_set<grd_bnd_reader::VertexIndex> VertexIndexSet;
  typedef boost::container::flat_map<std::string, VertexIndexSet> RegionVertexIndicesMap;

  void parse_additional_info(primary_reader & preader, DatasetList & datasets);
  void parse_data_block(primary_reader & preader, DatasetList & datasets);

  void combine_region_indices(std::vector<std::string> const & validity, VertexIndexSet & combined_indices);
  bool is_unique(std::string const & dataset_name) const;
  std::string generate_unique_name(std::string const & dataset_name, std::string const & filepath) const;

  unsigned int dimension_;
  unsigned int vertex_count_;
  unsigned int element_count_;
  RegionVertexIndicesMap region_vertex_indices_;
  PartialDatasetMap partial_datasets_;
  CompleteDatasetMap complete_datasets_;

  static void parse_dataset_block(primary_reader & preader, Dataset & dataset, std::string const & para);
  static void parse_dataset_values_block(primary_reader & preader, std::vector<double> & values, std::vector<double>::size_type const & para);
};

} //end of namespace dfise

} //end of namespace viennautils

#endif
