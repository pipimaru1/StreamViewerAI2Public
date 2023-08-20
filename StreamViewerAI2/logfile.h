#pragma once
class _LogFile
{
public:
	std::ofstream _LF;
	std::string _fpath;

	_LogFile(const char *fpath= ".\\log.txt");
	~_LogFile();

	int open(const char* fpath);
	void flush();
	int _TimeStump();
};

//MFCˆË‘¶
//void DoEvents(int n=1);

long _get_hhmmss();
long _get_YYYYMMDD();
std::string _str_YYMMDD();

extern  _LogFile _LGF;

std::string _TimeStumpStr();
//const char* _TimeStumpStr();
//std::string _STR_TimeStumpStr();

#define _LOGMSG2(msg0,msg1) {_LGF._LF << _TimeStumpStr() <<","<< __FILE__ <<","<<__LINE__<<","<<msg0 <<","<< msg1 << std::endl;_LGF.flush();}
#define _LOGMSG(msg0) {_LGF._LF << _TimeStumpStr() <<","<< __FILE__ <<","<<__LINE__<<","<<msg0 <<std::endl;_LGF.flush();}


#ifdef _DEBUG
#define LOGMSG2(msg0,msg1) {_LGF._LF << _TimeStumpStr() <<","<< __FILE__ <<","<<__LINE__<<","<<msg0 <<","<< msg1 << std::endl;_LGF.flush();}
#define LOGMSG(msg0) {_LGF._LF << _TimeStumpStr() <<","<< __FILE__ <<","<<__LINE__<<","<<msg0 <<std::endl;_LGF.flush();}
#else
//#define LOGMSG2(msg0,msg1) _LGF._LF << _TimeStumpStr() <<","<< __FILE__ <<","<<__LINE__<<","<<msg0 <<","<< msg1 << std::endl;
//#define LOGMSG(msg0) _LGF._LF << _TimeStumpStr() <<","<< __FILE__ <<","<<__LINE__<<","<<msg0 <<std::endl;
#define LOGMSG2(msg0,msg1)
#define LOGMSG(msg0)
#endif

