#ifndef FILEGDB_FEATURESET_HPP
#define FILEGDB_FEATURESET_HPP
// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/value_types.hpp>

// boost
// needed for wrapping the transcoder
#include <boost/scoped_ptr.hpp>
//
#include "filegdb_geodatabase.hpp"

//esri filegdb 
#include "FileGDBAPI.h"

using namespace std;
using namespace FileGDBAPI;
using mapnik::Featureset;
using mapnik::box2d;
using mapnik::feature_ptr;
using mapnik::context_ptr;

//文件型数据库矢量数据集操作类
template <typename filterT>
class filegdb_featureset : public Featureset
{
public:
    // this constructor can have any arguments you need
    //filegdb_featureset(Table* gdbTable,mapnik::box2d<double> const& box);
	//通过传递过滤器进行数据处理
	filegdb_featureset(filterT const& filter,mapnik::box2d<double> const& box,
			Table* gdbTable,int row_limit,std::string const& encoding,std::string where_clause);
	// desctructor
    virtual ~filegdb_featureset();

    // mandatory: you must expose a next() method, called when rendering
    mapnik::feature_ptr next();

private:
	//
	boost::scoped_ptr<mapnik::transcoder> tr_;
    // members are up to you, but these are recommended
	mapnik::box2d<double> box_;
	//
    mapnik::context_ptr ctx_;	
	//查询的数据行
	EnumRows spQueryRows;
	//过滤条件
	filterT filter_;
	//读取限定行
	mapnik::value_integer row_limit_; 
	//数据查询限定条件
	std::string where_clause_;

};
#endif // FILEGDB_FEATURESET_HPP