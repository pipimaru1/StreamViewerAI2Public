#pragma once

#include <atltypes.h>
#include <opencv2\opencv.hpp>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>

class my_video_writer
{
	int fourcc;// = cv::VideoWriter::fourcc('M', 'P', '4', 'V');
	double fps;// = 20; //動画からfpsを取得
	std::string filename;
	//cv::Mat frame_resized;
	std::mutex mtx;
public:
	cv::VideoWriter output;

	my_video_writer();
	//my_video_writer(std::string _fname);
	my_video_writer(const char* _fname);

	int write(cv::Mat& frame);
	int open(const char* _fname);
	int open();
	int release();

	int width;// = 720; 
	int height;// = 480;

};

struct DrawArea
{
	int spl_w;
	int spl_h;
	int num_w;
	int num_h;

	DrawArea()
	{
		spl_w = 1;
		spl_h = 1;
		num_w = 0;
		num_h = 0;
	};
	DrawArea(int _spl_w, int _spl_h,
		int _num_w, int _num_h)
	{
		spl_w = _spl_w;
		spl_h = _spl_h;
		num_w = _num_w;
		num_h = _num_h;
	};
};

//#define DRAW_ALL 0
//#define DRAW_UPLEFT 1
//#define DRAW_UPRIGHT 2
//#define DRAW_DOWNLEFT 3
//#define DRAW_DOWNRIGHT 4
//
//n:横の分割数 m:縦の分割数 i:横列の順番 0が最初 j:縦列の順番 
RECT RectSplit(RECT base, DrawArea _drawarea);

#define GETDTSTR_NORMAL 0
#define GETDTSTR_FMT1 1
//extern volatile PoseNet* _ptg_posenet;
//YYYYMMDDhhmmssを返す
std::string GetCurrentDateTimeString(int _mode = 0);
std::string add_dt_ext(std::string _base_str, std::string _ext);

//画像に録画中スタンプを付ける
int  rec_stump(cv::Mat& _image, bool rec, float _stump_size=1.0);
//画像にタイムスタンプを付ける
int  time_stump(cv::Mat& _image, float stump_size);

void DoEvents(int n = 1);

std::wstring stringToWstring(const std::string& s);
std::string wstring2string(std::wstring oWString);

int MyDrawText(HWND hWnd, HDC hDC, std::string& _text, int x0 = 0, int y0 = 0, bool _add = false);

int AllPaintBlack(HWND hWnd, HDC hDC, DrawArea drawing_position);

bool getLatestFrame(cv::VideoCapture& cap, cv::Mat& frame);

//int DrawPicToHDC(cv::Mat cvImg, HWND hWnd, HDC hDC, bool bMaintainAspectRatio, int spl_w, int spl_h, int num_w, int num_h); //bMaintainAspectRatio=true
int DrawPicToHDC(cv::Mat cvImg, HWND hWnd, HDC hDC, bool bMaintainAspectRatio, DrawArea drawing_position, bool _gray = false); //bMaintainAspectRatio=true

extern std::mutex Mutex_DrawCycle_Draw;

void ToggleFullscreenWithMenu(HWND hwnd);
void ResumeWindow(HWND hWnd);

extern HMENU hMenu_for_fullscreen;
extern RECT windowRect;
extern bool isFullscreen;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
int set_fullscreen(HWND hWnd, int monitor);

extern int CAPOPEN_TIMEOUT;
void tryConnect_url(const std::string& _url, cv::VideoCapture* _pt_capture, std::atomic<bool>& connected);
void tryConnect_usb(int _usb, cv::VideoCapture* _pt_capture, std::atomic<bool>& connected);
int catchConnect(cv::VideoCapture* _pt_capture, std::atomic<bool>& _connected, std::thread& _th);

//LPCWSTR _A2CW(const std::string& ascii)
std::wstring _A2CW(const std::string& ascii);

std::vector<std::vector<std::string>> readRecordsFromFile(const std::string& filename);
