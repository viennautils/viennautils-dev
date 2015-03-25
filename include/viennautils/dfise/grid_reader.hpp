#ifndef VIENNAUTILS_DFISE_GRID_READER_HPP
#define VIENNAUTILS_DFISE_GRID_READER_HPP

#include <string>
#include <vector>
#include <map>

#include <boost/array.hpp>

#include "viennautils/enum_pp.hpp"

namespace viennautils
{
namespace dfise
{

class primary_reader;

class grid_reader
{
public:
  typedef std::vector<double> VertexVector;
  typedef VertexVector::size_type VertexIndex;

  VIENNAUTILS_ENUM_PP(element_tag, (line,1),(triangle,2),(quadrilateral,3));

  struct element
  {
    element_tag::type tag_;
    std::vector<VertexIndex> vertex_indices_;
  };

  typedef std::vector<element> ElementVector;
  typedef std::vector<element>::size_type ElementIndex;

  struct region
  {
    std::string material_;
    std::vector<ElementIndex> element_indices_;
  };
  typedef std::map<std::string, region> RegionMap;

  grid_reader(std::string const & filename);

  unsigned int get_dimension() const {return dimension_;}
  //actually it is the vertex coordinate vector
  VertexVector const & get_vertices() const {return vertices_;}
  ElementVector const & get_elements() const {return elements_;}
  RegionMap const& get_regions() const {return regions_;}
  std::vector<double> get_transform() const {return trans_matrix_;}
  std::vector<double> get_translate() const {return trans_move_;}

private:
  struct GridInfo
  {
    std::vector<std::string> regions_;
    std::vector<std::string> materials_;
  };

  typedef boost::array<VertexIndex, 2> Edge;

  void parse_additional_info(primary_reader & preader);
  void parse_data_block(primary_reader & preader);
  void parse_coord_system_block(primary_reader & preader);
  void parse_vertices_block(primary_reader & preader, unsigned int const & para);
  void parse_edges_block(primary_reader & preader, unsigned int const & para);
  void parse_locations_block(primary_reader & preader, unsigned int const & para);
  void parse_elements_block(primary_reader & preader, unsigned int const & para);
  void parse_region_block(primary_reader & preader, std::vector<std::string>::size_type region_index, std::string const & para);
  void parse_region_element_block(primary_reader & preader, std::vector<std::string>::size_type region_index, std::vector<ElementIndex>::size_type const & para);

  VertexIndex get_oriented_edge_vertex(int edge_index, unsigned int vertex_index);

  GridInfo grid_info_;
  std::vector<Edge> edges_;

  unsigned int dimension_;
  VertexVector vertices_;
  ElementVector elements_;
  RegionMap regions_;
  std::vector<double> trans_matrix_;
  std::vector<double> trans_move_;
};

} //end of namespace dfise

} //end of namespace viennautils

#endif
