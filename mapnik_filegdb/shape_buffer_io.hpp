#ifndef FGDB_SHAP_BUFFER_IO_HPP
#define FGDB_SHAP_BUFFER_IO_HPP
//STL
#include <string.h>
//ESRI FileGDB API
#include <FileGDBAPI.h>
//mapnik geometry
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/noncopyable.hpp>

// boost
#include <boost/shared_ptr.hpp>

//导入名称空间
using namespace std;
using namespace FileGDBAPI;
using namespace mapnik;

//文件型数据库字段描述信息
struct fgdb_field_desc
{
    int index_;
    std::string name_;
    char type_;
    int length_;
    int dec_;
};
//几何数据操作
struct shape_buffer_io:mapnik::noncopyable{
public:
	shape_buffer_io();
	~shape_buffer_io();
	//读取当前行的几何范围，无论点线面
	static void read_bbox(Row* esri_row,mapnik::box2d<double> & bbox);

	//解析esri filegdb row to mapnik feature_ptr
	static mapnik::feature_ptr parse_esri_row(Row* esri_row,context_ptr ctx_,mapnik::transcoder const& tr);

	///
	static void parse_esri_row(mapnik::feature_ptr& feature,Row* esri_row,mapnik::transcoder const& tr);

	//解析点数据
	static void parse_esri_point(ShapeBuffer& shap_buffer,mapnik::geometry_container & geom);
	//解析ESRI多点数据
	static void parse_esri_multi_point(ShapeBuffer& shap_buffer,mapnik::geometry_container& geom);
	//解析线数据
	static void parse_esri_polyline(ShapeBuffer& shap_buffer,mapnik::geometry_container & geom);
	//解析多边形数据
	static void parse_esri_polygon(ShapeBuffer& shap_buffer,mapnik::geometry_container & geom);
	static bool parese_esri_geometry(ShapeBuffer& shap_buffer,feature_ptr& feature);
	//解析字节缓存为要素指针
	static feature_ptr  parse_esri_geometry(value_integer fid, ShapeBuffer& shap_buffer, context_ptr ctx_);	
	//给要素添加属性数据
	static void add_attributes(Row* esri_row,mapnik::feature_impl & f,mapnik::transcoder const& tr);
};
#endif //FGDB_SHAP_BUFFER_IO_HPP