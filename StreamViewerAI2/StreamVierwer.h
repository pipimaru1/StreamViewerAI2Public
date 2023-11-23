#pragma once

#include "resource.h"

//ä÷êîêÈåæ
int DrawCV2Window(HWND hWnd, void* __pt_yod, std::vector<std::vector<std::string>> urls, int draw_eria, std::ofstream* pAICSV);
int DrawCV2Windowf(HWND hWnd, void* __pt_yod, std::string _file_name, int draw_eria, std::ofstream* pAICSV);
bool set_cvw_stop(bool _s);
bool get_cvw_stop();
bool svw_wait_stop();
int set_score_threshold(float _st, void* __pt_yod);
int set_nms_threshold(float _nms, void* __pt_yod);
int set_conf_threshold(float _conf, void* __pt_yod);


//int set_cvw_status(int _st);
void DoEvents(int n = 1);
int AllPaintBlack(HWND hWnd, HDC hDC);
int set_display_time_seconds(int _display_time_seconds);
int set_frame_between_time(int _sleep);


extern std::vector<std::vector<std::string>> cam_urls;// = readRecordsFromFile(filename);
std::wstring stringToWstring(const std::string& s);

extern HWND hText;
extern HINSTANCE hInstance;

extern volatile int display_time_seconds;	// 8;
extern volatile int sleep; // 100;

extern bool _ai_text_output;
extern volatile bool _next_source;

extern volatile bool cvw_file_processing;
extern volatile bool cvw_file_end;

class my_video_writer
{
	int fourcc;// = cv::VideoWriter::fourcc('M', 'P', '4', 'V');
	double fps;// = 20; //ìÆâÊÇ©ÇÁfpsÇéÊìæ
	cv::VideoWriter output;
	std::string filename;
	cv::Mat frame_resized;
public:
	my_video_writer();
	//my_video_writer(std::string _fname);
	my_video_writer(const char* _fname);

	int write(cv::Mat& frame);
	int open(const char* _fname);
	int open();
	int release();

	int width;// = 720; 
	int height;// = 480;

	bool _VIDEO_REC;
	//bool _VIDEO_RECORDING;
};

extern my_video_writer _mvw_org;
extern my_video_writer _mvw_ai;

