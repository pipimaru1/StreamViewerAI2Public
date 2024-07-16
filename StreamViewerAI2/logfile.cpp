#include "stdafx.h"

#include <opencv2\opencv.hpp>
#include <fstream>
#include <filesystem>
#include "framework.h"
#include "logfile.h"

//std::ofstream _logf;

////////////////////////////////////////////////////////////////////////////////////
//ログ関係
////////////////////////////////////////////////////////////////////////////////////
//実体
LogFile LGF;

LogFile::LogFile(const char* fpath)
{
	open(fpath);
}

int LogFile::open(const char* fpath)
{
	_fpath = std::string(fpath);
	//_TimeStump();
	//_LF << "APPLICATION START" << std::endl;
	_LF = std::ofstream(fpath, std::ios_base::out);
	//_LF = std::ofstream(_fpath.c_str(), std::ios_base::out);
	_LF << "START " << get_YYYYMMDD() << " "<< get_hhmmss() <<std::endl;

	return 0;
}

LogFile::~LogFile()
{
	_LF.close();
}

void LogFile::flush()
{
	//↓これが有効だとデバックモードでバグる 落ちないけど無反応。ファイルハンドルが足りなくなる?
	//_LF.close();
	//_LF = std::ofstream(_fpath, std::ios_base::app);
}

std::string TimeStumpStr()
{
	std::time_t rawtime;
	std::tm timeinfo;
	std::time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	std::ostringstream _ai_csv_ostring;
	_ai_csv_ostring << (1900 + timeinfo.tm_year) << "/" 
		<< std::setw(2) << std::setfill('0') << (timeinfo.tm_mon + 1) << "/" 
		<< std::setw(2) << std::setfill('0') << timeinfo.tm_mday << " " 
		<< std::setw(2) << std::setfill('0') << timeinfo.tm_hour << ":" 
		<< std::setw(2) << std::setfill('0') << timeinfo.tm_min << ":" 
		<< std::setw(2) << std::setfill('0') << timeinfo.tm_sec;
	return _ai_csv_ostring.str();
}

std::string str_YYMMDD()
{
	std::time_t rawtime;
	std::tm timeinfo;
	std::time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	std::ostringstream _ai_csv_ostring;
	_ai_csv_ostring << (1900 + timeinfo.tm_year)
		<< std::setw(2) << std::setfill('0') << (timeinfo.tm_mon + 1)
		<< std::setw(2) << std::setfill('0') << timeinfo.tm_mday;
	return _ai_csv_ostring.str();
}


/*
std::string _STRTimeStumpStr()
{
	USES_CONVERSION;
	return A2OLE(TimeStumpStr());
}
*/

int LogFile::TimeStump()
{
	//std::time_t rawtime;
	//std::tm timeinfo;
	//std::time(&rawtime);
	//localtime_s(&timeinfo, &rawtime);
	//long tmpYYYYMMDD = (1900 + timeinfo.tm_year) * 10000 + (timeinfo.tm_mon + 1) * 100 + timeinfo.tm_mday;
	//long tmpHHMMSS = timeinfo.tm_hour * 10000 + timeinfo.tm_min * 100 + timeinfo.tm_sec;

	//_LF << (1900 + timeinfo.tm_year) << "/" << (timeinfo.tm_mon + 1) << "/" << timeinfo.tm_mday << " " << timeinfo.tm_hour << ":" << timeinfo.tm_min << ":" << timeinfo.tm_sec<<std::endl;
	_LF << TimeStumpStr() << std::endl;
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////
//時間関係
////////////////////////////////////////////////////////////////////////////////////
long get_hhmmss()
{
	std::time_t rawtime;
	std::tm timeinfo;
	std::time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	long tmpHHMMSS = timeinfo.tm_hour * 10000 + timeinfo.tm_min * 100 + timeinfo.tm_sec;

	return tmpHHMMSS;
}
long get_YYYYMMDD()
{
	std::time_t rawtime;
	std::tm timeinfo;
	std::time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	long tmpYYYYMMDD = (1900 + timeinfo.tm_year) * 10000 + (timeinfo.tm_mon + 1) * 100 + timeinfo.tm_mday;

	return tmpYYYYMMDD;
}
