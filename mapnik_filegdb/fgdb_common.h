#ifndef FGDB_PLUGIN_COMMON_H
#define FGDB_PLUGIN_COMMON_H

//将Utf-8编码字符串转为gbk编码
static std::string utf8_to_gbk(std::string utf8_src){
	std::string srcstr =utf8_src;
	std::string curLocale = setlocale(LC_ALL, NULL);      
	setlocale(LC_ALL, ".936");
	size_t newSize = srcstr.length() + 1;
	std::wstring unicodestr;
	unicodestr.resize(newSize);
	wmemset((wchar_t*)unicodestr.c_str(), 0, newSize);
	mbstowcs((wchar_t*)unicodestr.c_str(), srcstr.c_str(), newSize);
	std::string newstr;
	newSize = newSize*2 + 1;
	setlocale(LC_ALL, ".950");
	newstr.resize(newSize);
	memset((char*)newstr.c_str(), 0, newSize);
	wcstombs((char*)newstr.c_str(), unicodestr.c_str(), newSize);
	setlocale(LC_ALL, curLocale.c_str());
	return newstr;
}
static std::string gbk_to_utf8(std::string gbk_src)
{
	std::string srcstr = gbk_src;
	std::string curLocale = setlocale(LC_ALL, NULL);      
	setlocale(LC_ALL, ".950");

	size_t newSize = srcstr.length() + 1;
	std::wstring unicodestr;
	unicodestr.resize(newSize);
	wmemset((wchar_t*)unicodestr.c_str(), 0, newSize);
	mbstowcs((wchar_t*)unicodestr.c_str(), srcstr.c_str(), newSize);
	std::string newstr;
	newSize = newSize*2 + 1;
	setlocale(LC_ALL, ".936");
	newstr.resize(newSize);
	memset((char*)newstr.c_str(), 0, newSize);
	wcstombs((char*)newstr.c_str(), unicodestr.c_str(), newSize);
	setlocale(LC_ALL, curLocale.c_str());
	return newstr;
}
//将std::wstring转为std::string
static std::string& to_vstring(std::string& dest, std::wstring const & src)
{
	setlocale(LC_CTYPE, "");

	size_t const mbs_len = wcstombs(NULL, src.c_str(), 0);
	std::vector<char> tmp(mbs_len + 1);
	wcstombs(&tmp[0], src.c_str(), tmp.size());

	dest.assign(tmp.begin(), tmp.end() - 1);

	return dest;
};
// 把一个string转化为wstring
static std::wstring& to_vwstring(std::wstring& dest, std::string const & src)
{
	setlocale(LC_CTYPE, "");

	size_t const wcs_len = mbstowcs(NULL, src.c_str(), 0);
	std::vector<wchar_t> tmp(wcs_len + 1);
	mbstowcs(&tmp[0], src.c_str(), src.size());

	dest.assign(tmp.begin(), tmp.end() - 1);

	return dest;
}
#endif //FGDB_PLUGIN_COMMON_H