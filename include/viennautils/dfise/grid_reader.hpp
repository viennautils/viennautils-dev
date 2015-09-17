#ifndef VIENNAUTILS_DFISE_GRID_READER_HPP
#define VIENNAUTILS_DFISE_GRID_READER_HPP

#include <string>
#include <vector>
#include <map>

#include <boost/array.hpp>

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

  enum element_tag
  {
    element_tag_line = 1,
    element_tag_triangle = 2,
    element_tag_quadrilateral = 3,
    element_tag_tetrahedron = 5
  };

  struct element
  {
    element_tag tag_;
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
  RegionMap const & get_regions() const {return regions_;}
  std::vector<double> get_transform() const {return trans_matrix_;}
  std::vector<double> get_translate() const {return trans_move_;}

private:
  struct GridInfo
  {
    std::vector<std::string> regions_;
    std::vector<std::string> materials_;
  };

  typedef boost::array<VertexIndex, 2> Edge;
  typedef std::vector<Edge> EdgeVector;
  typedef EdgeVector::size_type EdgeIndex;
  typedef boost::array<int, 3> Face;
  typedef std::vector<Face> FaceVector;
  
  void parse_additional_info(primary_reader & preader);
  void parse_data_block(primary_reader & preader);
  void parse_coord_system_block(primary_reader & preader);
  void parse_vertices_block(primary_reader & preader, unsigned int const & para);
  void parse_edges_block(primary_reader & preader, unsigned int const & para);
  void parse_faces_block(primary_reader & preader, unsigned int const & para);
  void parse_locations_block(primary_reader & preader, unsigned int const & para);
  void parse_elements_block(primary_reader & preader, unsigned int const & para);
  void parse_region_block(primary_reader & preader, std::vector<std::string>::size_type region_index, std::string const & para);
  void parse_region_element_block(primary_reader & preader, std::vector<std::string>::size_type region_index, std::vector<ElementIndex>::size_type const & para);

  void read_vertex_index(primary_reader & preader, VertexIndex & index);
  //edge indices can be signed indicating the orientation of the edge
  void read_edge_index(primary_reader & preader, int & index);
  //face indices can be signed indicating the orientation of the face
  void read_face_index(primary_reader & preader, int & index);

  VertexIndex get_oriented_edge_vertex(int edge_index, Edge::size_type vertex_index);
  VertexIndex get_oriented_face_vertex(int face_index, EdgeIndex edge_index, Edge::size_type vertex_index);

  GridInfo grid_info_;
  EdgeVector edges_;
  FaceVector faces_;

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
