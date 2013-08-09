#ifndef VGS_FILE_GDB_GEODATABASE_HPP
#define VGS_FILE_GDB_GEODATABASE_HPP

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

// namespaces
using namespace std;
using namespace FileGDBAPI;
//ESRI FileGDB share ptr object
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
		wGdbPath = to_wstring(wGdbPath,gdbPath_);
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
	//字符串转码
	std::string valueOf(std::wstring& str){
		std::string ss;
		ss.assign(str.begin(), str.end());
		return ss;
	}
	//字符串转码
	std::string& to_string(std::string& dest, std::wstring const & src)
	{
		//std::setlocale(LC_CTYPE, "");

		size_t const mbs_len = wcstombs(NULL, src.c_str(), 0);
		std::vector<char> tmp(mbs_len + 1);
		wcstombs(&tmp[0], src.c_str(), tmp.size());

		dest.assign(tmp.begin(), tmp.end() - 1);

		return dest;
	}
	//字符串转码
	std::wstring& to_wstring(std::wstring& dest, std::string const & src)
	{
		//std::setlocale(LC_CTYPE, "");
		size_t const wcs_len = mbstowcs(NULL, src.c_str(), 0);
		std::vector<wchar_t> tmp(wcs_len + 1);
		mbstowcs(&tmp[0], src.c_str(), src.size());

		dest.assign(tmp.begin(), tmp.end() - 1);

		return dest;
	}
public :
	//
	filegdb_geodatabase(std::string const& fileGdbPath):gdbPath_(fileGdbPath)
	{
		OpenGdb();
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