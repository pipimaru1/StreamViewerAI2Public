#pragma once
class LogFile
{
public:
	std::ofstream _LF;
	std::string _fpath;

	LogFile(const char *fpath= ".\\log.txt");
	~LogFile();

	int open(const char* fpath);
	void flush();
	int TimeStump();
};

//MFC依存
//void DoEvents(int n=1);

long get_hhmmss();
long get_YYYYMMDD();
std::string str_YYMMDD();

extern  LogFile LGF;

std::string TimeStumpStr();
//const char* TimeStumpStr();
//std::string STRTimeStumpStr();

#ifdef _DEBUG
#define LOGMSG2(msg0,msg1) {LGF._LF << TimeStumpStr() <<","<< __FILE__ <<","<<__LINE__<<","<<msg0 <<","<< msg1 << std::endl;LGF.flush();}
#define LOGMSG(msg0) {LGF._LF << TimeStumpStr() <<","<< __FILE__ <<","<<__LINE__<<","<<msg0 <<std::endl;LGF.flush();}
#else
#define LOGMSG2(msg0,msg1)
#define LOGMSG(msg0)
#endif
//リリースモードでも記録
#define LOGMSG2X(msg0,msg1) {LGF._LF << TimeStumpStr() <<","<< __FILE__ <<","<<__LINE__<<","<<msg0 <<","<< msg1 << std::endl;LGF.flush();}
#define LOGMSGX(msg0) {LGF._LF << TimeStumpStr() <<","<< __FILE__ <<","<<__LINE__<<","<<msg0 <<std::endl;LGF.flush();}


//////////////////////////////////////////////////////////////////////////
//try catchの構文で見にくくなっていたのでマクロ化 重くなるの濫用しないこと
#define TRYCAT(tryal_codes) try\
{\
	tryal_codes;\
}\
catch (cv::Exception e){\
	const char* err_msg = e.what();\
	LGF._LF << TimeStumpStr() << "\t" << "RUNTIME ERROR IN CV:"<<__FILE__<<":"<<__LINE__<<"\n" << err_msg << std::endl;\
}\
catch (std::exception& es)\
{\
	const char* err_msg = es.what();\
	LGF._LF << TimeStumpStr() << "\t"<< "RUNTIME ERROR IN STD:" << __FILE__ << ":" << __LINE__ << "\n" << err_msg << std::endl; \
}
//////////////////////////////////////////////////////////////////////////
#define TRYCAT_CV(tryal_codes) try\
{\
	tryal_codes;\
}\
catch (cv::Exception e){\
	const char* err_msg = e.what();\
	LGF._LF << TimeStumpStr() << "\t" << "RUNTIME ERROR IN CV:"<<__FILE__<<":"<<__LINE__<<"\n" << err_msg << std::endl;\
}
//////////////////////////////////////////////////////////////////////////
#define TRYCAT_STD(tryal_codes) try\
{\
	tryal_codes;\
}\
catch (std::exception e){\
	const char* err_msg = e.what();\
	std::cout << TimeStumpStr() << std::endl << "RUNTIME ERROR IN CV:"<<__FILE__<<":"<<__LINE__<<"\n" << err_msg << std::endl;\
}

//////////////////////////////////////////////////////////////////////////
//try catchを無効にするマクロ
#define _TRYCAT(tryal_codes)  tryal_codes;
#define _TRYCAT_CV(tryal_codes)  tryal_codes;
#define _TRYCAT_STD(tryal_codes)  tryal_codes;