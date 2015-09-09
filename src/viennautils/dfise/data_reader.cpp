#include "viennautils/dfise/data_reader.hpp"

#include <boost/ref.hpp>
#include <boost/bind.hpp>

#include "viennautils/filesystem/filesystem.hpp"
#include "viennautils/dfise/parsing_error.hpp"
#include "viennautils/dfise/primary_reader.hpp"

namespace viennautils
{
namespace dfise
{

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

} //end of anonyomous namespace

struct data_reader::Dataset
{
  std::string name_;
  std::string function_;
  std::vector<std::string> validity_;
  unsigned int dimension_;
  ValueVector values_;
};

data_reader::data_reader( grid_reader const & greader
                        )
                        : dimension_(greader.get_dimension())
                        , vertex_count_(greader.get_vertices().size()/dimension_)
                        , element_count_(greader.get_elements().size())
{
  //find and sort all vertices of every region
  //this is actually redundant information, however it will be needed often when reading additional dataset files
  region_vertex_indices_.reserve(greader.get_regions().size());
  for (grid_reader::RegionMap::const_iterator region_it = greader.get_regions().begin(); region_it != greader.get_regions().end(); ++region_it)
  {
    VertexIndexSet & region_vertices = region_vertex_indices_[region_it->first];
    for ( std::vector<grid_reader::ElementIndex>::const_iterator element_it = region_it->second.element_indices_.begin()
        ; element_it != region_it->second.element_indices_.end()
        ; ++element_it
        )
    {
      //indices in greader are guaranteed to be valid (not out of bounds)
      grid_reader::element const & element = greader.get_elements()[*element_it];
      for ( std::vector<grid_reader::VertexIndex>::const_iterator vertex_it = element.vertex_indices_.begin()
          ; vertex_it != element.vertex_indices_.end()
          ; ++vertex_it
          )
      {
        region_vertices.insert(*vertex_it);
      }
    }
  }
}

void data_reader::read(std::string const & filepath)
{
  try
  {
    //start by reading all datasets in the file using the primary_reader
    DatasetList datasets;
    primary_reader preader( filepath
                          , boost::bind(&data_reader::parse_additional_info, this, _1, boost::ref(datasets))
                          , boost::bind(&data_reader::parse_data_block, this, _1, boost::ref(datasets))
                          );
    
    while (!datasets.empty())
    {
      std::string const & dataset_name = datasets.begin()->name_;
      try
      {
        std::vector<DatasetList::iterator> subset;
        boost::container::flat_set<std::string> total_validities;
        unsigned int dimension = datasets.begin()->dimension_;
        for (DatasetList::iterator dataset_it = datasets.begin(); dataset_it != datasets.end(); ++dataset_it)
        {
          if (dataset_it->name_ == dataset_name)
          {
            if (dataset_it->dimension_ != dimension)
            {
              throw make_exception<parsing_error>("different dimension given at different places");
            }
            subset.push_back(dataset_it);
            for (std::vector<std::string>::const_iterator region_it = dataset_it->validity_.begin(); region_it != dataset_it->validity_.end(); ++region_it)
            {
              if (region_vertex_indices_.find(*region_it) == region_vertex_indices_.end())
              {
                throw make_exception<parsing_error>("invalid validity region: " + *region_it);
              }
              if (total_validities.find(*region_it) != total_validities.end())
              {
                throw make_exception<parsing_error>("region: " + *region_it + " is specified in more than one validity array in a single file for a single dataset");
              }
              total_validities.insert(*region_it);
            }
          }
        }
        
        std::string unique_name = generate_unique_name(dataset_name, filepath);
        if (total_validities.size() == region_vertex_indices_.size())
        {
          //complete dataset
          complete_datasets_[unique_name].first = dimension;
          ValueVector & values = complete_datasets_[unique_name].second;
          if (subset.size() == 1)
          {
            //optimization for datasets that define all their values in one fell swoop
            values = datasets.begin()->values_;
          }
          else
          {
            values.resize(vertex_count_*dimension);
            for (std::vector<DatasetList::iterator>::iterator it = subset.begin(); it != subset.end(); ++it)
            {
              VertexIndexSet combined_indices;
              combine_region_indices((*it)->validity_, combined_indices);
              
              if (combined_indices.size()*dimension != (*it)->values_.size())
              {
                throw make_exception<parsing_error>( "invalid number of values, expected: "
                                                   + boost::lexical_cast<std::string>(combined_indices.size()*dimension)
                                                   + ", got: " + boost::lexical_cast<std::string>((*it)->values_.size())
                                                   );
              }
              
              size_t i = 0;
              for (VertexIndexSet::const_iterator combined_it = combined_indices.begin(); combined_it != combined_indices.end(); ++i, ++combined_it)
              {
                for (size_t j = 0; j < dimension; ++j)
                {
                  values[(*combined_it)*dimension+j] = (*it)->values_[i*dimension+j];
                }
              }
            }
          }
        }
        else
        {
          //partial dataset
          partial_datasets_[unique_name].first = dimension;
          VertexIndexSet total_combined_indices;
          combine_region_indices(std::vector<std::string>(total_validities.begin(), total_validities.end()), total_combined_indices);
          
          VertexIndexVector & vertex_indices = partial_datasets_[unique_name].second.first;
          vertex_indices.reserve(total_combined_indices.size());
          vertex_indices.insert(vertex_indices.begin(), total_combined_indices.begin(), total_combined_indices.end());
          ValueVector & values = partial_datasets_[unique_name].second.second;
          
          values.resize(total_combined_indices.size()*dimension);
          for (std::vector<DatasetList::iterator>::iterator it = subset.begin(); it != subset.end(); ++it)
          {
            VertexIndexSet combined_indices;
            combine_region_indices((*it)->validity_, combined_indices);
            
            if (combined_indices.size()*dimension != (*it)->values_.size())
            {
              throw make_exception<parsing_error>( "invalid number of values, expected: "
                                                  + boost::lexical_cast<std::string>(combined_indices.size()*dimension)
                                                  + ", got: " + boost::lexical_cast<std::string>((*it)->values_.size())
                                                  );
            }
            
            size_t i = 0;
            for (VertexIndexSet::const_iterator combined_it = combined_indices.begin(); combined_it != combined_indices.end(); ++i, ++combined_it)
            {
              size_t offset = (total_combined_indices.find(*combined_it)-total_combined_indices.begin())*dimension;
              for (size_t j = 0; j < dimension; ++j)
              {
                values[offset+j] = (*it)->values_[i*dimension+j];
              }
            }
          }
        }
        
        //remove all datasets that we just unified
        for (std::vector<DatasetList::iterator>::iterator it = subset.begin(); it != subset.end(); ++it)
        {
          datasets.erase(*it);
        }
      }
      catch (parsing_error const & e)
      {
        throw make_exception<parsing_error>("while unifying dataset: " + dataset_name + " - " + e.what());
      }
    }
  }
  catch(parsing_error const & e)
  {
    throw make_exception<parsing_error>("while parsing file: " + filepath + " - " + e.what());
  }
}

void data_reader::parse_additional_info(primary_reader & preader, DatasetList & datasets)
{
  if(preader.get_mandatory_info().type_ != primary_reader::filetype_dataset)
  {
    throw make_exception<parsing_error>( "invalid file type: " + boost::lexical_cast<std::string>(preader.get_mandatory_info().type_)
                                       + " - grid_reader parses dataset files only"
                                       );
  }
  
  if (  preader.get_mandatory_info().dimension_ != dimension_
     || preader.get_mandatory_info().nb_vertices_ != vertex_count_
     || preader.get_mandatory_info().nb_elements_ != element_count_
     || preader.get_mandatory_info().nb_regions_ != region_vertex_indices_.size()
     )
  {
    throw make_exception<parsing_error>("basic information (dimension, number of vertices/elements/regions) mismatch");
  }
  
  std::vector<std::string> names;
  std::vector<std::string> functions;
  preader.read_array("datasets", names);
  preader.read_array("functions", functions);
  if (names.size() != functions.size())
  {
    throw make_exception<parsing_error>("number of datasets and functions in Info block does not match");
  }
  
  for (size_t i = 0; i < names.size(); ++i)
  {
    Dataset tmp;
    tmp.name_ = names[i];
    tmp.function_ = functions[i];
    datasets.push_back(tmp);
  }
}

void data_reader::parse_data_block(primary_reader & preader, DatasetList & datasets)
{
  for (DatasetList::iterator it = datasets.begin(); it != datasets.end(); ++it)
  {
    preader.read_block<std::string>("Dataset", boost::bind(parse_dataset_block, boost::ref(preader), boost::ref(*it), _1));
  }
}

void data_reader::combine_region_indices(std::vector<std::string> const & validity, VertexIndexSet & combined_indices)
{
  if (validity.size() == 1)
  {
    combined_indices = region_vertex_indices_[validity[0]];
  }
  else
  {
    for (std::vector<std::string>::const_iterator it = validity.begin(); it != validity.end(); ++it)
    {
      combined_indices.insert(region_vertex_indices_[*it].begin(), region_vertex_indices_[*it].end());
    }
  }
}

bool data_reader::is_unique(std::string const & dataset_name) const
{
  return (  (partial_datasets_.find(dataset_name) == partial_datasets_.end())
         && (complete_datasets_.find(dataset_name) == complete_datasets_.end())
         );
}

std::string data_reader::generate_unique_name(std::string const & dataset_name, std::string const & filepath) const
{
  if (is_unique(dataset_name))
  {
    return dataset_name;
  }
  
  std::string file_stem = viennautils::filesystem::extract_stem(filepath);
  
  std::string dataset_stem_name = dataset_name + "_" + file_stem;
  if (is_unique(dataset_stem_name))
  {
    return dataset_stem_name;
  }
  
  for (unsigned int i = 2;; ++i)
  {
    std::string candidate = dataset_stem_name + "_" + boost::lexical_cast<std::string>(i);
    if (is_unique(candidate))
    {
      return candidate;
    }
  }
}

void data_reader::parse_dataset_block(primary_reader & preader, Dataset & dataset, std::string const & para)
{
  if (para != dataset.name_)
  {
    throw make_exception<parsing_error>("unexpected dataset name: " + para + " - expected name: " + dataset.name_);
  }
  
  try
  {
    expect(preader, "function", dataset.function_);
    std::string type;
    preader.read_attribute("type", type);
    if (type == "scalar")
    {
      expect(preader, "dimension", "1");
      dataset.dimension_ = 1;
    }
    else if (type == "vector")
    {
      preader.read_attribute("dimension", dataset.dimension_);
    }
    else
    {
      throw make_exception<parsing_error>("unexpected value for attribute: type - got value: " + type);
    }
    expect(preader, "location", "vertex");
    
    preader.read_array("validity", dataset.validity_);
    preader.read_block<std::vector<double>::size_type>("Values", boost::bind(parse_dataset_values_block, boost::ref(preader), boost::ref(dataset.values_), _1));
  }
  catch(parsing_error const & e)
  {
    throw make_exception<parsing_error>("while parsing dataset: " + dataset.name_ + " - " + e.what());
  }
}

void data_reader::parse_dataset_values_block(primary_reader & preader, std::vector<double> & values, std::vector<double>::size_type const & para)
{
  values.resize(para);
  for (std::vector<double>::size_type i = 0; i < values.size(); ++i)
  {
    preader.read_value(values[i]);
  }
}

} //end of namespace dfise

} //end of namespace viennautils
