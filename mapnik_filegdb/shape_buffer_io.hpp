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

//�������ƿռ�
using namespace std;
using namespace FileGDBAPI;
using namespace mapnik;

//�ļ������ݿ��ֶ�������Ϣ
struct fgdb_field_desc
{
    int index_;
    std::string name_;
    char type_;
    int length_;
    int dec_;
};
//�������ݲ���
struct shape_buffer_io:mapnik::noncopyable{
public:
	shape_buffer_io();
	~shape_buffer_io();
	//��ȡ��ǰ�еļ��η�Χ�����۵�����
	static void read_bbox(Row* esri_row,mapnik::box2d<double> & bbox);

	//����esri filegdb row to mapnik feature_ptr
	static mapnik::feature_ptr parse_esri_row(Row* esri_row,context_ptr ctx_,mapnik::transcoder const& tr);

	///
	static void parse_esri_row(mapnik::feature_ptr& feature,Row* esri_row,mapnik::transcoder const& tr);

	//����������
	static void parse_esri_point(ShapeBuffer& shap_buffer,mapnik::geometry_container & geom);
	//����ESRI�������
	static void parse_esri_multi_point(ShapeBuffer& shap_buffer,mapnik::geometry_container& geom);
	//����������
	static void parse_esri_polyline(ShapeBuffer& shap_buffer,mapnik::geometry_container & geom);
	//�������������
	static void parse_esri_polygon(ShapeBuffer& shap_buffer,mapnik::geometry_container & geom);
	static bool parese_esri_geometry(ShapeBuffer& shap_buffer,feature_ptr& feature);
	//�����ֽڻ���ΪҪ��ָ��
	static feature_ptr  parse_esri_geometry(value_integer fid, ShapeBuffer& shap_buffer, context_ptr ctx_);	
	//��Ҫ�������������
	static void add_attributes(Row* esri_row,mapnik::feature_impl & f,mapnik::transcoder const& tr);
};
#endif //FGDB_SHAP_BUFFER_IO_HPP