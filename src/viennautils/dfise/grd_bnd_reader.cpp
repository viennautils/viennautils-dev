#include "viennautils/dfise/grd_bnd_reader.hpp"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/lexical_cast.hpp>

#include "../../../include/viennautils/dfise/grd_bnd_reader.hpp"

#include "viennautils/dfise/parsing_error.hpp"
#include "viennautils/dfise/primary_reader.hpp"

namespace viennautils
{
namespace dfise
{

grd_bnd_reader::grd_bnd_reader(std::string const & filename)
{
  primary_reader preader( filename
                        , boost::bind(&grd_bnd_reader::parse_additional_info, this, _1)
                        , boost::bind(&grd_bnd_reader::parse_data_block, this, _1)
                        );
  
  edges_.clear();
}

void grd_bnd_reader::parse_additional_info(primary_reader & preader)
{
  switch(preader.get_mandatory_info().type_)
  {
    case primary_reader::filetype_grid:     filetype_ = filetype_grd; break;
    case primary_reader::filetype_boundary: filetype_ = filetype_bnd; break;
    default:
      throw make_exception<parsing_error>( "invalid file type: " + boost::lexical_cast<std::string>(preader.get_mandatory_info().type_)
                                         + " - grd_bnd_reader parses grid files only"
                                         );
  }
  
  dimension_ = preader.get_mandatory_info().dimension_;
  
  preader.read_array("regions", grd_bnd_info_.regions_);
  preader.read_array("materials", grd_bnd_info_.materials_);
}

void grd_bnd_reader::parse_data_block(primary_reader & preader)
{
  preader.read_block              ("CoordSystem", boost::bind(&grd_bnd_reader::parse_coord_system_block, this, boost::ref(preader)));
  preader.read_block<unsigned int>("Vertices",    boost::bind(&grd_bnd_reader::parse_vertices_block,     this, boost::ref(preader), _1));
  preader.read_block<unsigned int>("Edges",       boost::bind(&grd_bnd_reader::parse_edges_block,        this, boost::ref(preader), _1));
  preader.read_block<unsigned int>("Faces",       boost::bind(&grd_bnd_reader::parse_faces_block,        this, boost::ref(preader), _1));
  preader.read_block<unsigned int>("Locations",   boost::bind(&grd_bnd_reader::parse_locations_block,    this, boost::ref(preader), _1));
  preader.read_block<unsigned int>("Elements",    boost::bind(&grd_bnd_reader::parse_elements_block,     this, boost::ref(preader), _1));
  
  for (std::vector<std::string>::size_type i = 0; i < grd_bnd_info_.regions_.size(); ++i)
  {
    preader.read_block<std::string>("Region",     boost::bind(&grd_bnd_reader::parse_region_block,       this, boost::ref(preader), i, _1));
  }
}

void grd_bnd_reader::parse_coord_system_block(primary_reader & preader)
{
  preader.read_array("translate", trans_move_, 3);
  preader.read_array("transform", trans_matrix_, 9);
}

void grd_bnd_reader::parse_vertices_block(primary_reader & preader, unsigned int const & para)
{
  if (para != preader.get_mandatory_info().nb_vertices_)
  {
    throw viennautils::make_exception<parsing_error>("number of vertices in Info block and Vertices block does not match");
  }

  vertices_.resize(preader.get_mandatory_info().nb_vertices_ * preader.get_mandatory_info().dimension_);
  for (VertexIndex i = 0; i < vertices_.size(); ++i)
  {
    preader.read_value(vertices_[i]);
  }
}

void grd_bnd_reader::parse_edges_block(primary_reader & preader, unsigned int const & para)
{
  if (para != preader.get_mandatory_info().nb_edges_)
  {
    throw viennautils::make_exception<parsing_error>("number of edges in Info block and Edges block does not match");
  }
  
  edges_.resize(preader.get_mandatory_info().nb_edges_);
  for(std::vector<Edge>::size_type i = 0; i < edges_.size(); ++i)
  {
    read_vertex_index(preader, edges_[i][0]);
    read_vertex_index(preader, edges_[i][1]);
  }
}

void grd_bnd_reader::parse_faces_block(primary_reader & preader, unsigned int const & para)
{
  if (para != preader.get_mandatory_info().nb_faces_)
  {
    throw viennautils::make_exception<parsing_error>("number of faces in Info block and Faces block does not match");
  }
  
  faces_.resize(preader.get_mandatory_info().nb_faces_);
  for(std::vector<Face>::size_type i = 0; i < faces_.size(); ++i)
  {
    int number_of_edges;
    preader.read_value(number_of_edges);
    if (number_of_edges != 3)
    {
      throw viennautils::make_exception<parsing_error>( "face with " + boost::lexical_cast<std::string>(number_of_edges) + " edges found"
                                                      + ", however only triangular faces (with 3 edges) are supported right now");
    }
    read_edge_index(preader, faces_[i][0]);
    read_edge_index(preader, faces_[i][1]);
    read_edge_index(preader, faces_[i][2]);
  }
}

void grd_bnd_reader::parse_locations_block(primary_reader & preader, unsigned int const & para)
{
  std::string ignore;
  for (unsigned int i = 0; i < para; ++i)
  {
    preader.read_value(ignore);
  }
}

void grd_bnd_reader::parse_elements_block(primary_reader & preader, unsigned int const & para)
{
  if (para != preader.get_mandatory_info().nb_elements_)
  {
    throw viennautils::make_exception<parsing_error>("number of elements in Info block and Elements block does not match");
  }
  
  elements_.resize(preader.get_mandatory_info().nb_elements_);
  for (std::vector<element>::size_type i = 0; i < elements_.size(); ++i)
  {
    unsigned int tag_value;
    preader.read_value(tag_value);
    if (  tag_value != element_tag_line
       && tag_value != element_tag_triangle
       && tag_value != element_tag_quadrilateral
       && tag_value != element_tag_polygon
       && tag_value != element_tag_tetrahedron
       ) //TODO this used to be handled with enum_pp::is_valid (which sadly requires C99 and was thus kicked out)
    {
      throw viennautils::make_exception<parsing_error>("encountered unsupported element tag value: " + boost::lexical_cast<std::string>(tag_value));
    }
    
    elements_[i].tag_ = static_cast<element_tag>(tag_value);
    switch (elements_[i].tag_)
    {
      case element_tag_line:
      {
        //line given by two vertices
        elements_[i].vertex_indices_.resize(2);
        read_vertex_index(preader, elements_[i].vertex_indices_[0]);
        read_vertex_index(preader, elements_[i].vertex_indices_[1]);
        break;
      }
      case element_tag_triangle:
      {
        //triangle given by 3 edge indices (negative indices invert orientation!)
        int edge_index;
        
        //first edge
        read_edge_index(preader, edge_index);
        elements_[i].vertex_indices_.push_back(get_oriented_edge_vertex(edge_index, 0));
        elements_[i].vertex_indices_.push_back(get_oriented_edge_vertex(edge_index, 1));
        
        //second edge
        read_edge_index(preader, edge_index);
        elements_[i].vertex_indices_.push_back(get_oriented_edge_vertex(edge_index, 1));
        
        //ignore third edge - we already have all 3 vertices
        read_edge_index(preader, edge_index);
        break;
      }
      case element_tag_quadrilateral:
      {
        //rectangle given by 4 edge indices (again, negative indicies invert orientation)
        int edge_index;
        
        //first edge
        read_edge_index(preader, edge_index);
        elements_[i].vertex_indices_.push_back(get_oriented_edge_vertex(edge_index, 0));
        elements_[i].vertex_indices_.push_back(get_oriented_edge_vertex(edge_index, 1));
        
        //ignore second edge
        read_edge_index(preader, edge_index);
        
        //thrid edge
        read_edge_index(preader, edge_index);
        elements_[i].vertex_indices_.push_back(get_oriented_edge_vertex(edge_index, 0));
        elements_[i].vertex_indices_.push_back(get_oriented_edge_vertex(edge_index, 1));
        
        //ignore last edge
        read_edge_index(preader, edge_index);
        break;
      }
      case element_tag_polygon:
      {
        //read number of edges
        unsigned int number_of_edges;
        preader.read_value(number_of_edges);
        elements_[i].vertex_indices_.reserve(number_of_edges);
        
        for (unsigned int j = 0; j < number_of_edges; ++j)
        {
          //read one edge at a time and add the first vertex of the edge to the polygon
          int edge_index;
          read_edge_index(preader, edge_index);
          elements_[i].vertex_indices_.push_back(get_oriented_edge_vertex(edge_index, 0));
        }
        break;
      }
      case element_tag_tetrahedron:
      {
        int face_index;
        //first face
        read_face_index(preader, face_index);
        elements_[i].vertex_indices_.push_back(get_oriented_face_vertex(face_index, 0, 0));
        elements_[i].vertex_indices_.push_back(get_oriented_face_vertex(face_index, 0, 1));
        elements_[i].vertex_indices_.push_back(get_oriented_face_vertex(face_index, 1, 1));
        
        //second face (find the last, missing vertex of the tetrahedron in the second face)
        int face_index2;
        read_face_index(preader, face_index2);
        boost::array<VertexIndex,3> candidates;
        candidates[0] = get_oriented_face_vertex(face_index2, 0, 0);
        candidates[1] = get_oriented_face_vertex(face_index2, 0, 1);
        candidates[2] = get_oriented_face_vertex(face_index2, 1, 1);
        
        VertexVector::size_type missing_vertex = 0;
        for (;  missing_vertex < candidates.size() && (  candidates[missing_vertex] == elements_[i].vertex_indices_[0]
                                                      || candidates[missing_vertex] == elements_[i].vertex_indices_[1]
                                                      || candidates[missing_vertex] == elements_[i].vertex_indices_[2]
                                                      )
             ; ++missing_vertex
            );
        if (missing_vertex == candidates.size())
        {
          throw viennautils::make_exception<parsing_error>( "tetrahedron with element index " + boost::lexical_cast<std::string>(i)
                                                          + " seems to have two equal faces"
                                                          + "\nfirst face index: " + boost::lexical_cast<std::string>(face_index)
                                                          + "\nsecond face index: " + boost::lexical_cast<std::string>(face_index2)
                                                          + "\nvertex indices of first face:"
                                                          + "\n" + boost::lexical_cast<std::string>(elements_[i].vertex_indices_[0])
                                                          + "\n" + boost::lexical_cast<std::string>(elements_[i].vertex_indices_[1])
                                                          + "\n" + boost::lexical_cast<std::string>(elements_[i].vertex_indices_[2])
                                                          + "\nvertex indices of second face: "
                                                          + "\n" + boost::lexical_cast<std::string>(candidates[0])
                                                          + "\n" + boost::lexical_cast<std::string>(candidates[1])
                                                          + "\n" + boost::lexical_cast<std::string>(candidates[2])
                                                          );
        }
        
        elements_[i].vertex_indices_.push_back(candidates[missing_vertex]);
        
        //ignore remaining two faces (we should have all the vertices we need)
        read_face_index(preader, face_index);
        read_face_index(preader, face_index);
        break;
      }
      //all possible enum values have to be implemented! warning should alert to missing enum values
    }
  }
}

void grd_bnd_reader::parse_region_block(primary_reader & preader, std::vector<std::string>::size_type region_index, std::string const & para)
{
  std::string const & region_name = grd_bnd_info_.regions_[region_index];
  if (para != region_name)
  {
    throw make_exception<parsing_error>("unexpected region name: " + para + " - expected name: " + region_name);
  }
  
  try
  {
    std::string material;
    preader.read_attribute("material", material);
    if (material != grd_bnd_info_.materials_[region_index])
    {
      throw make_exception<parsing_error>("material parameter does not match Info block");
    }
    regions_[region_name].material_ = grd_bnd_info_.materials_[region_index];

    preader.read_block<std::vector<ElementIndex>::size_type>("Elements", boost::bind(&grd_bnd_reader::parse_region_element_block, this, boost::ref(preader), region_index, _1));
  }
  catch (parsing_error const & e)
  {
    throw make_exception<parsing_error>("while parsing region: " + region_name + " - " + e.what());
  }
}

void grd_bnd_reader::parse_region_element_block(primary_reader & preader, std::vector<std::string>::size_type region_index, std::vector<ElementIndex>::size_type const & para)
{
  std::vector<ElementIndex>& region_elements = regions_[grd_bnd_info_.regions_[region_index]].element_indices_;
  region_elements.resize(para);
  for (std::vector<ElementIndex>::size_type i = 0; i < region_elements.size(); ++i)
  {
    preader.read_value(region_elements[i]);
    if (region_elements[i] >= elements_.size())
    {
      throw make_exception<parsing_error>("element index out of bounds: " + boost::lexical_cast<std::string>(region_elements[i])
                                         + " max: " + boost::lexical_cast<std::string>(elements_.size()-1)
                                         );
    }
  }
}

void grd_bnd_reader::read_vertex_index(primary_reader & preader, VertexIndex & index)
{
  preader.read_value(index);
  if (index >= vertices_.size()/dimension_)
  {
    throw make_exception<parsing_error>( "vertex index out of bounds: " + boost::lexical_cast<std::string>(index)
                                       + " max: " + boost::lexical_cast<std::string>(vertices_.size()/dimension_-1)
                                       );
  }
}

void grd_bnd_reader::read_edge_index(primary_reader & preader, int & index)
{
  preader.read_value(index);
  EdgeVector::size_type actual_edge_index = (index < 0 ? -index-1 : index);
  if (actual_edge_index >= edges_.size())
  {
    throw make_exception<parsing_error>( "edge index out of bounds: " + boost::lexical_cast<std::string>(index)
                                       + " turns into actual edge index of: " + boost::lexical_cast<std::string>(actual_edge_index)
                                       + " max: " + boost::lexical_cast<std::string>(edges_.size()-1)
                                       );
  }
}

void grd_bnd_reader::read_face_index(primary_reader & preader, int & index)
{
  preader.read_value(index);
  FaceVector::size_type actual_face_index = (index < 0 ? -index-1 : index);
  if (actual_face_index >= faces_.size())
  {
    throw make_exception<parsing_error>( "face index out of bounds: " + boost::lexical_cast<std::string>(index)
                                       + " turns into actual face index of: " + boost::lexical_cast<std::string>(actual_face_index)
                                       + " max: " + boost::lexical_cast<std::string>(faces_.size()-1)
                                       );
  }
}

grd_bnd_reader::VertexIndex grd_bnd_reader::get_oriented_edge_vertex(int edge_index, Edge::size_type vertex_index)
{
  EdgeVector::size_type actual_edge_index;
  if (edge_index < 0)
  {
    actual_edge_index = -edge_index-1; //so -1 -> 0, -2 -> 1 etc.
    return edges_[actual_edge_index][1-vertex_index];
  }
  else
  {
    actual_edge_index = edge_index;
    return edges_[actual_edge_index][vertex_index];
  }
}

grd_bnd_reader::VertexIndex grd_bnd_reader::get_oriented_face_vertex(int face_index, EdgeIndex edge_index, Edge::size_type vertex_index)
{
  FaceVector::size_type actual_face_index;
  if (face_index < 0)
  {
    actual_face_index = -face_index-1;
    return get_oriented_edge_vertex(-faces_[actual_face_index][faces_[actual_face_index].size()-edge_index]-1, vertex_index);
  }
  else
  {
    actual_face_index = face_index;
    return get_oriented_edge_vertex(faces_[actual_face_index][edge_index], vertex_index);
  }
}

} //end of namespace dfise

} //end of namespace viennautils
