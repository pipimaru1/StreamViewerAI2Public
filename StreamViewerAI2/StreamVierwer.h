#pragma once

#include "resource.h"

//���܂��g���Ă��Ȃ��\����(2024/2/15)
//�N���X�����������������Y�ݒ�
struct DrawCVWindowParam
{
	int _DrawCycleMode;
	HWND hWnd;
	HDC hDC;
	DrawArea _draw_eria;
	bool _keep_aspect;								//�A�X�y�N�g��̐ݒ�
	YoloObjectDetection* _pt_YoloObjectDetection;	//Yolo
	PoseNet* _pt_pose;								//�|�[�Y�l�b�gAI
	std::vector<std::vector<std::string>> urls;		//�J��������URL�̂Q�����z��
	int _camera_number;								//�J�����ԍ������l
	std::string _file_name;
	std::ofstream* pAICSV;
};

#define FLAG_TIMESTUMP 0x0001
#define FLAG_RECSTUMP 0x0002
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//�`��T�C�N��
bool DrawCycle(
	int _DrawCycleMode,
	HWND hWnd,
	HDC hDC,
	DrawArea _draw_area,
	bool _keep_aspect,
	//cv::VideoCapture& _capture,
	cv::VideoCapture* _pt_capture,
	YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose,
	const std::string& _str_source1,
	const std::string& _str_source2,
	std::string& _ai_out_put_string,
	std::ofstream* _pAICSV,
	int _display_time_seconds,
	int _stump_mode //
);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 0 ����
// 1 �L���v�`���[�摜�ُ̈�
int DrawCycle_Core_Yolo(
	int _DrawCycleMode,
	HWND hWnd,
	HDC hDC,
	DrawArea _draw_area,
	bool _keep_aspect,
	//cv::VideoCapture& _capture,
	cv::VideoCapture* _pt_capture,
	YoloObjectDetection* _pt_YoloObjectDetection,
	//	PoseNet* _pt_pose,
	const std::string& _str_source1,
	const std::string& _str_source2,
	std::string& _ai_out_put_string,
	std::ofstream* _pAICSV
	//	int _display_time_seconds
);
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 0 ����
// 1 �L���v�`���[�摜�ُ̈�
int DrawCycle_Core_Pose(
	int _DrawCycleMode,
	HWND hWnd,
	HDC hDC,
	DrawArea _draw_area,
	bool _keep_aspect,
	//cv::VideoCapture& _capture,
	cv::VideoCapture* _pt_capture,
	//YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose
	//const std::string& _str_source1,
	//const std::string& _str_source2,
	//std::string& _ai_out_put_string,
	//std::ofstream* _pAICSV,
	//int _display_time_seconds
);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//�w�肳�ꂽ��ʂ̕�����IP�J�����̉f����\������
//�X�g���[������
int DrawCV2Window(
	int _DrawCycleMode,
	HWND hWnd,
	HDC hDC,
	DrawArea _draw_eria,
	bool _keep_aspect,
	YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose,
	std::vector<std::vector<std::string>> urls,
	std::ofstream* pAICSV);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//�w�肳�ꂽ��ʂ̕����ɓ���t�@�C���̉f����\������
int DrawCV2Windowf(
	int _DrawCycleMode,
	HWND hWnd,
	HDC hDC,
	DrawArea _draw_eria,
	bool _keep_aspect,
	YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose,
	std::string _file_name,
	std::ofstream* pAICSV);

bool set_cvw_stop(bool _s);
bool get_cvw_stop();
bool svw_wait_stop();
int set_score_threshold(float _st, void* _pt_YoloObjectDetection);
int set_nms_threshold(float _nms, void* _pt_YoloObjectDetection);
int set_conf_threshold(float _conf, void* _pt_YoloObjectDetection);

//int set_cvw_status(int _st);
int AllPaintBlack(HWND hWnd, HDC hDC, DrawArea drawing_position);
int set_display_time_seconds(int _display_time_seconds);
int get_display_time_seconds();
int set_frame_between_time(int _sleep);
int get_frame_between_time();

//extern std::vector<std::vector<std::string>> cam_urls;// = readRecordsFromFile(filename);
std::wstring stringToWstring(const std::string& s);

extern HWND hText;
extern HINSTANCE hInstance;


//extern volatile int display_time_seconds;	// 8;
extern std::atomic<int> display_time_seconds;	// 8;
//extern volatile int sleep; // 100;
//extern volatile int fram_interval_ms;// 100 sleep
extern std::atomic<int> fram_interval_ms;// 100 sleep
extern std::atomic<bool> ai_text_output;
extern std::atomic<bool> Next_source;
extern std::atomic<bool> cvw_file_processing;
extern std::atomic<bool> cvw_file_end;

extern my_video_writer mvw_org;
extern my_video_writer mvw_ai;

//AI�̃��[�h��I������
#define DRAWCYCLE_YOLO5		1 //yolo
#define DRAWCYCLE_POSENET	2 //posenet
#define DRAWCYCLE_STREAM	3 //AI�͎g��Ȃ�
#define DRAWCYCLE_YOLO8		4 //yolo

bool PROC_CYCLE(bool _mode);
int PROC_FIX_CAM(int _cam_number);

extern int Count_VideoCapture;