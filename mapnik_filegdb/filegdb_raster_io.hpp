#ifndef FGDB_RASTER_IO_HPP
#define FGDB_RASTER_IO_HPP
#include <mapnik/noncopyable.hpp>
#include <mapnik/box2d.hpp>

//ESRI FileGDB API
#include <FileGDBAPI.h>
//�������ƿռ�
using namespace std;
using namespace FileGDBAPI;
using namespace mapnik;

//FGDB Grid��ʽդ�����ݶ�ȡ��
struct filegdb_raster_io:mapnik::noncopyable{
	public:
	filegdb_raster_io();
	~filegdb_raster_io();

};
#endif //FGDB_RASTER_IO_HPP