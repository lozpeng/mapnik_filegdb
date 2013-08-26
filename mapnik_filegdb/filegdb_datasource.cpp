// file plugin
#include "filegdb_datasource.hpp"
#include "filegdb_featureset.hpp"
// boost
#include <boost/make_shared.hpp>

//char encode 
#include <unicode/ucnv.h>
// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/global.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/timer.hpp>
#include <mapnik/value_types.hpp>

using mapnik::datasource;
using mapnik::parameters;
using mapnik::String;
using mapnik::Double;
using mapnik::Integer;
using mapnik::Boolean;
using mapnik::datasource_exception;
using mapnik::filter_in_box;
using mapnik::filter_at_point;
using mapnik::attribute_descriptor;

using FileGDBAPI::Envelope;

DATASOURCE_PLUGIN(filegdb_datasource)
//�ļ������ݿ�򿪷�ʽ����������Ҫ����:
//params["gdbpath"]="e:\\test.gdb"
//params["lyrpath"] ="/F_V_P_PROVINCE_2012";
//params["type"]="filegdb";
//params["where"]="field=value or field2";
//
filegdb_datasource::filegdb_datasource(parameters const& params)
:   datasource(params),
    desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8")),
	row_limit_(*params.get<mapnik::value_integer>("row_limit",0)),
	where_clause(*params.get<std::string>("where","utf-8")),
    extent_()
{
    this->init(params);
}
//��ʼ���ļ����ݿ���Ϣ
void filegdb_datasource::init(mapnik::parameters const& params)
{
	const std::string fgdb_path = *params.get<std::string>("gdbpath");
	//�������ݿ⣬connect to the database,i think the filegdb can open in share mode
	gdb_ = boost::make_shared<filegdb_geodatabase>(fgdb_path);

	//const std::string table_path = *params.get<std::string>("lyrpath");
	esriTablePath_ = *params.get<std::string>("lyrpath");
	//������ı�񣬼�ָ���Ĵ洢�ռ����ݵı��,open the table this data source linked to 
	try
	{
		gdbTable_= new Table;
		bool opened = gdb_->open_table(esriTablePath_,*gdbTable_);
		if(!opened)
		{
			MAPNIK_LOG_ERROR(gdb_) << "ESRI FileGDB Plugin error on open vector table! ";
			delete gdbTable_;
			return ;
		}
	}
	catch(...)
	{
		return ;
	}
	//try to get the geo extent of the filegdb table
	FileGDBAPI::Envelope env;
	fgdbError hr;
	//��ȡʸ�����ݵķ�Χ
	if(( hr= gdbTable_->GetExtent(env)) == S_OK)
	{
		//setup the datasource extent
		extent_.init(env.xMin,env.yMin,env.xMax,env.yMax);
	}
	else
	{
		//stup the extent use the Whole Earth Extent
		extent_.init(-180.0,-90.0,180.0,90.0);
	}
	//��ʼ����Χ֮�󣬽����ֶ���Ϣ�ĳ�ʼ��
	try
	{
		read_fields();
		std::string tab_def ;
		hr = gdbTable_->GetDefinition(tab_def);
		//������������Ķ�����Ϣ
		if(hr == S_OK)
			geo_type = parse_geoType(tab_def);
		else geo_type = "";
	}
	catch(...)
	{
		 MAPNIK_LOG_ERROR(gdb_) << "FileGDB Plugin: error processing field attributes. ";
	}

}
//��xml�ĵ��н�����ESRI���������ַ���
std::string filegdb_datasource::parse_geoType(std::string xml_str){
	xmlDocPtr doc; 
	xmlNodePtr xmlRoot;  
	doc = xmlParseMemory(xml_str.c_str(),xml_str.length());
	//���Ϊ���򷵻ؿ�
	if(!doc)
	{
		xmlCleanupParser();
		return "";
	}
	xmlRoot = xmlDocGetRootElement (doc);  
    if(! xmlRoot)  
    {  
        xmlFreeDoc(doc); 
		xmlCleanupParser();
        return "";  
    } 
	//������������
	std::string geo_type;
	xmlChar *xpath = (xmlChar*) "//GeometryType";
	//
	xmlXPathObjectPtr result;
	xmlNodeSetPtr nodeset;
	result = getNodeSet(doc,xpath);
	if(result)
	{
		nodeset = result->nodesetval;
		 for (int i=0; i < nodeset->nodeNr; i++) {                                             
		    xmlChar * keyword = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode, 1);   
			std::string k((char*)keyword);
			geo_type = k;
			xmlFree(keyword);
			break;
		  }                
		xmlXPathFreeObject(result);
	}
	xmlFreeDoc(doc); 
	xmlCleanupParser();
	return geo_type;
}
//��ȡ�ֶ���Ϣ
void filegdb_datasource::read_fields(void)
{
	#ifdef MAPNIK_STATS
        mapnik::progress_timer __stats2__(std::clog, "filegdb_datasource::init(read_fields)");
	#endif
	
	fgdbError hr;
	FileGDBAPI::FieldInfo fieldInfos;
	if((hr = gdbTable_->GetFieldInformation(fieldInfos)) == S_OK)
	{
		int fldCount;
		if((hr = fieldInfos.GetFieldCount(fldCount)) == S_OK)
		{
			if(fldCount<=0) return ;

			for(int i=0;i<fldCount;i++)
			{
				FileGDBAPI::FieldType fldType;
				hr = fieldInfos.GetFieldType(i, fldType);
				if(hr != S_OK) continue;

				//��ʱ����
				std::wstring tmpName ;
				//��Ҫ�õ��ֶ���
				std::string fldName;
				hr = fieldInfos.GetFieldName(i, tmpName);
				//1.
				if(hr != S_OK) continue;

				fldName = to_vstring(fldName, tmpName);
				switch(fldType)
				{
					case fieldTypeString:
					case fieldTypeDate:
						desc_.add_descriptor(attribute_descriptor(fldName,String));
						break;
					case fieldTypeOID:
						desc_.add_descriptor(attribute_descriptor(fldName,Integer,true,4));
						break;
					case fieldTypeDouble:
						desc_.add_descriptor(attribute_descriptor(fldName,Double,false,8));
						break;
					case fieldTypeSmallInteger:
					case fieldTypeInteger:
						desc_.add_descriptor(attribute_descriptor(fldName,Integer,false,4));
						break;
					default:
						MAPNIK_LOG_WARN(gdb_) << "filegdb_datasource: Field Type System Not Deal With��" << fldType<<": Field Name ="<<fldName;
						break;
				}
			}
		}
	}
}
filegdb_datasource::~filegdb_datasource() {
	//if(gdbTable_)
	//{
	//	try
	//	{
	//		gdb_->close_table(*gdbTable_);
	//		delete gdbTable_;
	//	}
	//	catch(...)
	//	{
	//		//ǿ��ɾ��ָ��ʱ�����ˡ����ǲ�������
	//	}
	//}
}

// This name must match the plugin filename, eg 'filegdb.input'
const char * filegdb_datasource::name()
{
    return "filegdb";
}

mapnik::datasource::datasource_t filegdb_datasource::type() const
{
    return datasource::Vector;
}

mapnik::box2d<double> filegdb_datasource::envelope() const
{
    return extent_;
}
//��ȡ����Դ�ļ�����������
boost::optional<mapnik::datasource::geometry_t> filegdb_datasource::get_geometry_type() const
{
	 boost::optional<mapnik::datasource::geometry_t> result;
	//���û�дӱ��в�ѯ�����������ΪҪ�ؼ���ʽ
	if(geo_type.length() <= 0)return Collection;
	//��
	if(geo_type=="esriGeometryPolygon"){
		result.reset(mapnik::datasource::Polygon);
	}
	//��
	if(geo_type=="esriGeometryPoint"|| geo_type=="esriGeometryMultipoint")
		result.reset(mapnik::datasource::Point);
	//��
	if(geo_type=="esriGeometryPolyline"||geo_type=="esriGeometryLine"){
		result.reset(mapnik::datasource::LineString);
	}
	//��֧�ֵ����Ͳ���Ҫ�ؼ���ʽ
    return result;
}

mapnik::layer_descriptor filegdb_datasource::get_descriptor() const
{
    return desc_;
}

mapnik::featureset_ptr filegdb_datasource::features(mapnik::query const& q) const
{
#ifdef MAPNIK_STATS
	 mapnik::progress_timer __stats__(std::clog, "filegdb_datasource::features");
#endif
	 filter_in_box filter(q.get_bbox());
	 return featureset_ptr(new filegdb_featureset<filter_in_box>(filter,q.get_bbox(),gdbTable_,row_limit_,desc_.get_encoding(),where_clause));
}

mapnik::featureset_ptr filegdb_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "filegdb_datasource::features_at_point");
#endif
	filter_at_point filter(pt,tol);
	return featureset_ptr(new filegdb_featureset<filter_at_point>(filter,filter.box_,gdbTable_,row_limit_,desc_.get_encoding(),where_clause));
}
