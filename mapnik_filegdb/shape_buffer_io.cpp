#include "shape_buffer_io.hpp"
// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/global.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/mapped_memory_cache.hpp>
// boost
#include <boost/make_shared.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/cstdint.hpp> // for int16_t and int32_t


//fgdb common
#include "fgdb_common.h"

using mapnik::datasource_exception;
using mapnik::geometry_type;

//nothing to do
shape_buffer_io::shape_buffer_io()
{}
//nothing to do 
shape_buffer_io::~shape_buffer_io(){}


//读取范围,
void shape_buffer_io::read_bbox(Row* esri_row,mapnik::box2d<double> & bbox){
	FileGDBAPI::ShapeBuffer shap_buffer;
	fgdbError hr = esri_row->GetGeometry(shap_buffer);
	if(hr!=S_OK)
	{
		return ;
	}
	//TODO:: 
	//shap_buffer.
}
///
void shape_buffer_io::parse_esri_row(mapnik::feature_ptr& feature,Row* esri_row,mapnik::transcoder const& tr){
	FileGDBAPI::ShapeBuffer shap_buffer;
	fgdbError hr = esri_row->GetGeometry(shap_buffer);

	if(hr != S_OK)return;
	shape_buffer_io::parese_esri_geometry(shap_buffer,feature);
	int32 oid;
	hr = esri_row->GetOID(oid);
	if(hr==S_OK)
		feature->set_id(mapnik::value_integer(oid));
	shape_buffer_io::add_attributes(esri_row,*feature,tr);
}

//解析esri row to mapnik feature_ptr
mapnik::feature_ptr shape_buffer_io::parse_esri_row(Row* esri_row,context_ptr ctx_,mapnik::transcoder const& tr){
	FileGDBAPI::ShapeBuffer shap_buffer;
	fgdbError hr = esri_row->GetGeometry(shap_buffer);
	if(hr!=S_OK)return mapnik::feature_ptr();
	if(shap_buffer.IsEmpty())
		return mapnik::feature_ptr();
	int32 oid;
	hr = esri_row->GetOID(oid);

	if(hr!=S_OK)
		return mapnik::feature_ptr();
	mapnik::value_integer fid(oid);

	//构建要素指针，新建几何数据
	feature_ptr feature = shape_buffer_io::parse_esri_geometry(fid,shap_buffer,ctx_);
	//设置要素编号
	feature->set_id(fid);
	shape_buffer_io::add_attributes(esri_row,*feature,tr);
	return feature;
}
//获取属性数据
void shape_buffer_io::add_attributes(Row* esri_row,mapnik::feature_impl & f,mapnik::transcoder const& tr){
	using namespace boost::spirit;
	FileGDBAPI::FieldInfo fieldInfos;
	fgdbError hr = esri_row->GetFieldInformation(fieldInfos);
	if(hr!=S_OK)
		return ;
	int fldCount = 0;
	hr = fieldInfos.GetFieldCount(fldCount);
	//字段数获取失败或字段数小于等0则退出
	if(hr!=S_OK||fldCount<=0)return;
	for(int i=0;i<fldCount;i++)
	{
		try
		{
			FileGDBAPI::FieldType fldType;
			hr = fieldInfos.GetFieldType(i, fldType);
			//类型获取失败则继续处理下一个字段
			if(hr != S_OK) continue;
			//临时名称
			std::wstring tmpName ;
			//需要用的字段名
			std::string fldName;
			hr = fieldInfos.GetFieldName(i, tmpName);
			//1.
			if(hr != S_OK) continue;
			fldName = to_vstring(fldName, tmpName);
			//循环字段类型读取数据
			switch(fldType)
			{
				case fieldTypeSingle:{
					short vl;
					hr = esri_row->GetShort(tmpName,vl);
					if(hr == S_OK)
					{
						//f.put(fldName,vl);
					}
					break;
				}
				case fieldTypeString:
					{
						std::wstring t_value;
						hr = esri_row->GetString(tmpName,t_value);
						if(hr == S_OK && t_value.length()>=1)
						{
							std::string value;
							value = to_vstring(value,t_value);
							mapnik::util::trim(value);
							f.put_new(fldName,tr.transcode(value.c_str()));
						}
						break;
					}
				case fieldTypeDate:{
						tm tm_val;
						hr = esri_row->GetDate(tmpName,tm_val);
						if(hr == S_OK)
						{
							std::ostringstream os;
							//返回类型为
							os<<tm_val.tm_year<<"-"<<tm_val.tm_mon<<"-"<<tm_val.tm_mday<<" "<<tm_val.tm_hour<<":"<<tm_val.tm_min<<":"<<tm_val.tm_sec;
							std::string tm_str(os.str());
							f.put_new(fldName,tr.transcode(tm_str.c_str()));
						}
						break;
					}
				case fieldTypeDouble:
					{
						double d_val=0.0;
						hr = esri_row->GetDouble(tmpName,d_val);
						if(hr == S_OK)
							f.put_new(fldName,d_val);
						break;
					}
				case fieldTypeSmallInteger:
				case fieldTypeInteger:
					{
						int32 i_val;
						hr = esri_row->GetInteger(tmpName,i_val);
						if(hr == S_OK)
						{
							mapnik::value_integer vi(i_val);
							f.put_new(fldName,vi);
						}
						break;
					}
				case fieldTypeOID:{
						int32 i_oid=0;
						hr = esri_row->GetOID(i_oid);
						if(hr == S_OK && i_oid >= 0)
						{
							mapnik::value_integer vi(i_oid);
							f.put_new(fldName,vi);
						}
						break;
					}
				default:break;
			}
		}
		catch(...)
		{
			
		}
	}
}
//======parse geometry static function
//解析FileGDB中的点要素
void shape_buffer_io::parse_esri_point(ShapeBuffer& shap_buffer,mapnik::geometry_container & geom){
	fgdbError hr;
	PointShapeBuffer* point_buffer = (PointShapeBuffer*)&shap_buffer;
	FileGDBAPI::Point* points = NULL;
	hr = point_buffer->GetPoint(points);
	if(hr!=S_OK)return ;

	double x = points->x;
	double y = points->y;
	std::auto_ptr<geometry_type> point(new geometry_type(mapnik::Point));
	point->move_to(x, y);
	geom.push_back(point);
}
//解析FileGDB中多点数据
void shape_buffer_io::parse_esri_multi_point(ShapeBuffer& shap_buffer,mapnik::geometry_container& geom)
{
	MultiPointShapeBuffer* multi_pnt =(MultiPointShapeBuffer*)&shap_buffer;
	int pnt_num=0;
	fgdbError hr = multi_pnt->GetNumPoints(pnt_num);
	if(hr != S_OK || pnt_num <= 0)return ;
	FileGDBAPI::Point* points = NULL;
	hr = multi_pnt->GetPoints(points);

	if(hr!=S_OK || !points)
		return ;
	//循环每一个点
	for(int i = 0; i < pnt_num; i++)
	{
		std::auto_ptr<geometry_type> point(new geometry_type(mapnik::Point));
		point->move_to(points[i].x,points[i].y);
		geom.push_back(point);
	}
}

//解析FileGDB中的线要素
void shape_buffer_io::parse_esri_polyline(ShapeBuffer& shap_buffer,mapnik::geometry_container & geom)
{
	MultiPartShapeBuffer* multi_buffer = (MultiPartShapeBuffer*)&shap_buffer;
	//所有的点
	FileGDBAPI::Point* points = NULL;
	fgdbError hr = multi_buffer->GetPoints(points);

	if(hr != S_OK)return ;

	int numParts = 0 ;
	//计算该线图形一共由几个部分组成
	multi_buffer->GetNumParts(numParts);
	//取出来该图形一共有多少个点组成
	int numPnts = 0;
	hr = multi_buffer->GetNumPoints(numPnts);
	//如果计算失败或几何图形的点数<=0则不解析该图形
	if(hr!=S_OK || numPnts<=0)return ;

	int* parts =0;
	hr = multi_buffer->GetParts(parts);
	if(hr != S_OK )return ;

	//如果线只有一个部分组成则单独处理
	if(numParts==1)
	{
		//新建一条线
		std::auto_ptr<geometry_type> line(new geometry_type(mapnik::LineString));

		line->move_to(points[0].x, points[0].y);
		//循环处理每一个点
		for (int i = 1; i < numPnts; ++i)
		{
			line->line_to(points[i].x, points[i].y);
		}
		//将新建立的线放入返回要素的尾部
		geom.push_back(line);
		return;
	}
	else
	{
		int start, end;
		//处理每个段
		for(int p = 0; p<numParts; p++)
		{
			start = parts[p];
			if (p == numParts - 1)
				end = numPnts;
			else
				end = parts[p + 1];
			//新建一条线
			std::auto_ptr<geometry_type> line(new geometry_type(mapnik::LineString));

			line->move_to(points[start].x, points[start].y);
			//循环处理每一个点
			for (int j = start+1; j < end; ++j)
			{
				line->line_to(points[j].x, points[j].y);
			}
			//将新建立的线放入返回要素的尾部
			geom.push_back(line);
		}
	}
}

//解析FileGDB中的多边形数据
void shape_buffer_io::parse_esri_polygon(ShapeBuffer& shap_buffer,mapnik::geometry_container & geom)
{
	MultiPartShapeBuffer* multi_buffer = (MultiPartShapeBuffer*)&shap_buffer;
	//所有的点
	FileGDBAPI::Point* points = NULL;
	fgdbError hr = multi_buffer->GetPoints(points);

	if(hr != S_OK)return ;

	int numParts = 0 ;
	//计算该线图形一共由几个部分组成
	multi_buffer->GetNumParts(numParts);
	//取出来该图形一共有多少个点组成
	int numPnts = 0;
	hr = multi_buffer->GetNumPoints(numPnts);
	//如果计算失败或几何图形的点数<=0则不解析该图形
	if(hr!=S_OK || numPnts<=0)return ;

	int* parts =0;
	hr = multi_buffer->GetParts(parts);
	if(hr != S_OK )return ;

	//如果多边形只有一个部分组成则单独处理
	if(numParts==1)
	{
		//新建一条线
		std::auto_ptr<geometry_type> polygon(new geometry_type(mapnik::Polygon));

		polygon->move_to(points[0].x, points[0].y);
		//循环处理每一个点
		for (int i = 1; i < numPnts; ++i)
		{
			polygon->line_to(points[i].x, points[i].y);
		}
		polygon ->close_path();
		//将新建立的多边形放入返回要素的尾部
		geom.push_back(polygon);
		return;
	}
	else
	{
		int start, end;
		//处理每个段
		for(int p = 0; p<numParts; p++)
		{
			start = parts[p];

			if (p == numParts - 1)
				end = numPnts;
			else
				end = parts[p + 1];

			//新建一个多边形
			std::auto_ptr<geometry_type> polygon(new geometry_type(mapnik::Polygon));

			polygon->move_to(points[start].x, points[start].y);
			//循环处理每一个点
			for (int j = start+1; j < end; ++j)
			{
				polygon->line_to(points[j].x, points[j].y);
			}
			polygon->close_path();
			//将新建立的多边形放入返回要素的尾部
			geom.push_back(polygon);
		}
	}
}

bool shape_buffer_io::parese_esri_geometry(ShapeBuffer& shap_buffer,feature_ptr& feature)
{
	FileGDBAPI::GeometryType geo_type;
	fgdbError hr = shap_buffer.GetGeometryType(geo_type);
	if(hr!=S_OK)
		return false;
	switch(geo_type)
	{
	case geometryPoint:
		{
			parse_esri_point(shap_buffer,feature->paths());
			break;
		}
	case geometryMultipoint:
		{
			parse_esri_multi_point(shap_buffer,feature->paths());
			break;
		}
	case geometryPolyline:
		{
			parse_esri_polyline(shap_buffer,feature->paths());
			break;
		}
	case geometryPolygon:
		{
			parse_esri_polygon(shap_buffer,feature->paths());
			break;
		}
	default:{
		return false;
			}
	}
	return true;
}

feature_ptr shape_buffer_io::parse_esri_geometry(value_integer fid, ShapeBuffer& shap_buffer, context_ptr ctx_)
{
	fgdbError hr;
	feature_ptr feature(feature_factory::create(ctx_,fid));
	FileGDBAPI::GeometryType geo_type;
	hr = shap_buffer.GetGeometryType(geo_type);
	if(hr!=S_OK)
		return mapnik::feature_ptr();
	switch(geo_type)
	{
	case geometryPoint:
		{
			parse_esri_point(shap_buffer,feature->paths());
			break;
		}
	case geometryMultipoint:
		{
			parse_esri_multi_point(shap_buffer,feature->paths());
			break;
		}
	case geometryPolyline:
		{
			parse_esri_polyline(shap_buffer,feature->paths());
			break;
		}
	case geometryPolygon:
		{
			parse_esri_polygon(shap_buffer,feature->paths());
			break;
		}
	default:break;
	}
	return feature;
}