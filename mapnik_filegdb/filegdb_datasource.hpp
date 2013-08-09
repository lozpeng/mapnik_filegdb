#ifndef FILEGDB_DATASOURCE_HPP
#define FILEGDB_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>

//libxml2
#include <libxml/parser.h>  
#include <libxml/tree.h>  
#include <libxml/xpath.h>  

// boost
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

// stl
#include <string>
//file gdb
#include "FileGDBAPI.h"

//
#include "filegdb_geodatabase.hpp"

using mapnik::datasource;
using mapnik::parameters;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::coord2d;


inline xmlXPathObjectPtr  getNodeSet(xmlDocPtr doc, xmlChar *xpath){                        
	xmlXPathContextPtr context;                                                         
	xmlXPathObjectPtr result;                                                           

	context = xmlXPathNewContext(doc);                                                  
	if (context == NULL) {                                                              
		printf("Error in xmlXPathNewContext\n");                                          
		return NULL;                                                                      
	}                                                                                   
	result = xmlXPathEvalExpression(xpath, context);                                    
	xmlXPathFreeContext(context);                                                       
	if (result == NULL) {                                                               
		printf("Error in xmlXPathEvalExpression\n");                                      
		return NULL;                                                                      
	}                                                                                   
	if(xmlXPathNodeSetIsEmpty(result->nodesetval)){                                     
		xmlXPathFreeObject(result);                                                       
		printf("No result\n");                                                            
		return NULL;                                                                      
	}                                                                                   
	return result;                                                                      
}

//文件型矢量数据库数据源
class filegdb_datasource : public mapnik::datasource
{
public:
	// constructor
	// arguments must not change
	filegdb_datasource(mapnik::parameters const& params);

	// destructor
	virtual ~filegdb_datasource ();

	// mandatory: type of the plugin, used to match at runtime
	mapnik::datasource::datasource_t type() const;

	// mandatory: name of the plugin
	static const char * name();

	// mandatory: function to query features by box2d
	// this is called when rendering, specifically in feature_style_processor.hpp
	mapnik::featureset_ptr features(mapnik::query const& q) const;

	// mandatory: function to query features by point (coord2d)
	// not used by rendering, but available to calling applications
	mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;

	// mandatory: return the box2d of the datasource
	// called during rendering to determine if the layer should be processed
	mapnik::box2d<double> envelope() const;

	// mandatory: optionally return the overal geometry type of the datasource
	boost::optional<mapnik::datasource::geometry_t> get_geometry_type() const;

	// mandatory: return the layer descriptor
	mapnik::layer_descriptor get_descriptor() const;

private:
	// recommended naming convention of datasource members:
	// name_, type_, extent_, and desc_
	static const std::string name_;
	mapnik::layer_descriptor desc_;
	mapnik::box2d<double> extent_;
	//数据库连接
	boost::shared_ptr<filegdb_geodatabase> gdb_ ;
	//表格对象
	Table* gdbTable_;
	//打开的表格数据的路径
	std::string esriTablePath_;
	//集合数据类型
	geometry_t geo_type_;
	//限制行数
	const int row_limit_;
	//esri几何类型字符串
	std::string geo_type;
private://functions
	// recommended - do intialization in a so-named init function
	// to reduce code in constructor
	void init(mapnik::parameters const& params);
	std::string parse_geoType(std::string xml_str);
	//从数据源中读取字段信息
	void read_fields(void);
};


#endif // FILEGDB_DATASOURCE_HPP
