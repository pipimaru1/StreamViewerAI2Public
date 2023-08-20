#include "stdafx.h"

#include <opencv2\opencv.hpp>
#include <fstream>
#include <filesystem>
#include "framework.h"
#include "logfile.h"

//std::ofstream _logf;

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//ログ関係
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//実体
_LogFile _LGF;

_LogFile::_LogFile(const char* fpath)
{
	open(fpath);
}

int _LogFile::open(const char* fpath)
{
	_fpath = std::string(fpath);
	_LF = std::ofstream(fpath, std::ios_base::out);

	return 0;
}

_LogFile::~_LogFile()
{
	_LF.close();
}

void _LogFile::flush()
{
	//↓これが有効だとデバックモードでバグる 落ちないけど無反応。ファイルハンドルが足りなくなる?
	//_LF.close();
	//_LF = std::ofstream(_fpath, std::ios_base::app);
}

std::string _TimeStumpStr()
{
	std::time_t rawtime;
	std::tm timeinfo;
	std::time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	std::ostringstream _ost;
	_ost << (1900 + timeinfo.tm_year) << "/" 
		<< std::setw(2) << std::setfill('0') << (timeinfo.tm_mon + 1) << "/" 
		<< std::setw(2) << std::setfill('0') << timeinfo.tm_mday << " " 
		<< std::setw(2) << std::setfill('0') << timeinfo.tm_hour << ":" 
		<< std::setw(2) << std::setfill('0') << timeinfo.tm_min << ":" 
		<< std::setw(2) << std::setfill('0') << timeinfo.tm_sec;
	return _ost.str();
}

std::string _str_YYMMDD()
{
	std::time_t rawtime;
	std::tm timeinfo;
	std::time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	std::ostringstream _ost;
	_ost << (1900 + timeinfo.tm_year)
		<< std::setw(2) << std::setfill('0') << (timeinfo.tm_mon + 1)
		<< std::setw(2) << std::setfill('0') << timeinfo.tm_mday;
	return _ost.str();
}



int _LogFile::_TimeStump()
{
	_LF << _TimeStumpStr() << std::endl;
	return 0;
}


//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//時間関係
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
long _get_hhmmss()
{
	std::time_t rawtime;
	std::tm timeinfo;
	std::time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	long tmpHHMMSS = timeinfo.tm_hour * 10000 + timeinfo.tm_min * 100 + timeinfo.tm_sec;

	return tmpHHMMSS;
}
long _get_YYYYMMDD()
{
	std::time_t rawtime;
	std::tm timeinfo;
	std::time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	long tmpYYYYMMDD = (1900 + timeinfo.tm_year) * 10000 + (timeinfo.tm_mon + 1) * 100 + timeinfo.tm_mday;

	return tmpYYYYMMDD;
}
