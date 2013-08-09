#ifndef FGDB_FILE_GDB_GEODATABASE_HPP
#define FGDB_FILE_GDB_GEODATABASE_HPP

//file gdb
#include "FileGDBAPI.h"
//stl
#include <string.h>
#include <iostream>

//boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

//mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>

#ifdef _WINDOWS
#include <Windows.h>
#endif

//中文字符问题
#include <locale.h>
#include "fgdb_common.h"
// namespaces
using namespace std;
using namespace FileGDBAPI;
//ESRI FileGDB shared ptr object


//文件型数据库连接与管理类
class filegdb_geodatabase
{
	//members
private :
	Geodatabase fileGdb_;
	std::string gdbPath_;
	//functions
private:
	//打开文件型空间数据库
	void OpenGdb()
	{
		fgdbError hr;
		std::wstring wGdbPath;
		wGdbPath = to_vwstring(wGdbPath,gdbPath_);
		if ((hr = OpenGeodatabase(wGdbPath, fileGdb_)) != S_OK)
		{
			std::ostringstream s;
			wstring   errorText;
			ErrorInfo::GetErrorDescription(hr, errorText);
			std::string ss;
			ss.assign(errorText.begin(), errorText.end());
			s << "数据库打开失败:" << ss<<"。数据库路径:"<<gdbPath_<<endl;
			throw mapnik::datasource_exception (s.str());
		}
	}
public :	
	//
	filegdb_geodatabase(std::string const& fileGdbPath):gdbPath_(fileGdbPath)
	{
		OpenGdb();
	}
	//关闭一个打开的表
	void close_table(Table& table)
	{
		try
		{
			fgdbError hr;
			if((hr=fileGdb_.CloseTable(table)) !=S_OK)
			{
				std::ostringstream s;
				wstring   errorText;
				ErrorInfo::GetErrorDescription(hr, errorText);
				std::string ss;
				ss.assign(errorText.begin(), errorText.end());
				s << "关闭表时失败:" << ss<<"。数据库路径:"<<gdbPath_<<endl;
				throw mapnik::datasource_exception (s.str());
			}
			else
			{

			}
		}
		catch(...)
		{
		
		}
	}
	//打开矢量数据
	bool open_table(std::string path,Table& table)
	{
		try{
			fgdbError hr;
			std::wstring wPath ;
			wPath = to_vwstring(wPath,path);
			if((hr=fileGdb_.OpenTable(wPath,table)) != S_OK)
			{
				std::ostringstream s;
				wstring   errorText;
				ErrorInfo::GetErrorDescription(hr, errorText);
				std::string ss;
				ss.assign(errorText.begin(), errorText.end());
				s << "数据表打开失败:" << ss<<"。数据库路径:"<<gdbPath_<<"，打开表路径为:"<<path<<endl;
				throw mapnik::datasource_exception (s.str());
			}
			return true;
		}
		catch(...)
		{
			return false;
		}
	}
	std::string get_dbPath()
	{
		return gdbPath_;
	}
	~filegdb_geodatabase()
	{
		fgdbError hr;
		if ((hr = CloseGeodatabase(fileGdb_)) != S_OK)
		{
			wstring   errorText;
			ErrorInfo::GetErrorDescription(hr, errorText);
			std::ostringstream s;
			s<<"An error occurred while closing the geodatabase."<<errorText.c_str()<<endl;
			throw mapnik::datasource_exception (s.str());
		}
	}
};
#endif