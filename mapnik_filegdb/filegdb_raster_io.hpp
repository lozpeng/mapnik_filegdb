#ifndef FGDB_RASTER_IO_HPP
#define FGDB_RASTER_IO_HPP
#include <mapnik/noncopyable.hpp>
#include <mapnik/box2d.hpp>

//ESRI FileGDB API
#include <FileGDBAPI.h>
//导入名称空间
using namespace std;
using namespace FileGDBAPI;
using namespace mapnik;

//FGDB Grid格式栅格数据读取类
struct filegdb_raster_io:mapnik::noncopyable{
	public:
	filegdb_raster_io();
	~filegdb_raster_io();

};
#endif //FGDB_RASTER_IO_HPP