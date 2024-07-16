#pragma once

#include "resource.h"

//↓まだ使っていない構造体(2024/2/15)
//引数か複雑化したため、構造体化を検討中
//クラス化した方がいいか悩み中
struct DrawCVWindowParam
{
	int DrawCycleMode;
	HWND hWnd;
	HDC hDC;
	DrawArea draw_eria;
	bool keep_aspect;								//アスペクト比の設定
	YoloObjectDetection* ptYoloOBJECTDECTECTION;	//Yolo
	PoseNet* pt_pose;								//ポーズネットAI
	std::vector<std::vector<std::string>> urls;		//カメラ名とURLの２次元配列
	int camera_number;								//カメラ番号初期値
	std::string file_name;
//	std::ofstream* pAICSV;
};

#define FLAG_TIMESTUMP 0x0001
#define FLAG_RECSTUMP 0x0002
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//描画サイクル
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
	const std::string& _str_location,
	const std::string& _str_camera_url,
	std::string& _ai_out_put_string,
	std::ofstream* _pAICSV,
	int _display_time_seconds,
	int _stump_mode //
);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 物体認識を行う処理をまとめたもの
// 0 正常
// 1 キャプチャー画像の異常
int DrawCycle_Core_Yolo(
	int _DrawCycleMode,
	HWND hWnd,
	HDC hDC,
	DrawArea _draw_area,
	bool _keep_aspect,
	//cv::VideoCapture* _pt_capture,
	cv::Mat& image_input,
	YoloObjectDetection* _pt_YoloObjectDetection,
	const std::string& _str_location,
	const std::string& _str_source2,
	std::string& _ai_out_put_string
	//	int _display_time_seconds
);
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 姿勢認識を行う処理をまとめたもの
// 0 正常
// 1 キャプチャー画像の異状
int DrawCycle_Core_Pose(
	int _DrawCycleMode,
	HWND hWnd,
	HDC hDC,
	DrawArea _draw_area,
	bool _keep_aspect,
	//cv::VideoCapture* _pt_capture,
	cv::Mat& image_input,
	PoseNet* _pt_pose
);

struct CV2WIN
{
	int draw_cycle_mode;
	HWND hWnd;
	HDC hDC;
	DrawArea draw_area;
	bool keep_aspect;
	YoloObjectDetection* ptYoloOBJECTDECTECTION;
	PoseNet* pt_pose;
	std::vector<std::vector<std::string>> urls;
	std::string file_name;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//指定された画面の部分にIPカメラの映像を表示する
// DrawCycleを呼び出す
// 一定時間ごとに入力を切り替える
int DrawCV2Window(
	int _DrawCycleMode,
	HWND hWnd,
	HDC hDC,
	DrawArea _draw_eria,
	bool _keep_aspect,
	YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose,
	std::vector<std::vector<std::string>> urls
);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//指定された画面の部分に動画ファイルの映像を表示する
int DrawCV2Windowf(
	int _DrawCycleMode,
	HWND hWnd,
	HDC hDC,
	DrawArea _draw_eria,
	bool _keep_aspect,
	YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose,
	std::string _file_name
);

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

std::wstring string2wstring(const std::string& s);

//extern HWND hHWND_TEXT;
//extern HINSTANCE hINSTANCE;

extern std::atomic<int> DISPLAY_TIME_SECOND;	// 8;
extern std::atomic<int> FRAME_INTERVAL_MS;// 100 sleep
extern std::atomic<bool> AI_TEXT_OUTPUT;
extern std::atomic<bool> NEXT_SOURCE;
extern std::atomic<bool> CVW_FILE_PROCESSING;
extern std::atomic<bool> CVW_FILE_END;

extern MyVideoWriter mlVIDEOWRITERORG;
extern MyVideoWriter mlVIDEOWRITERAI;

//AIのモードを選択する
#define DRAWCYCLE_YOLO5		1 //yolo
#define DRAWCYCLE_POSENET	2 //posenet
#define DRAWCYCLE_STREAM	3 //AIは使わない
#define DRAWCYCLE_YOLO8		4 //yolo

bool PROC_CYCLE(bool _mode);
int PROC_FIX_CAM(int _cam_number);

extern int COUNT_VIDEOCAPTURE;

//CSVファイル関係　ここで定義するかは疑問 スレッドセーフにするにはグローバル変数にする必要がある。
extern std::ofstream* pAICSV;
extern std::mutex FILE_MUTEX;
//extern std::atomic<bool> fileatomic;

extern std::string AICSVPATH;
int open_ai_csv_file();
extern bool AI_DATA_CSV_WRITE; //ファイルに書き込まないときはfalse オーバーフロー防止 
extern bool AI_DATA_CSV_OVER_WRITE; //上書きするときはtrue 

extern SqlServer SqlServerAi;
extern SqlServer SqlServerImage;
extern bool SQL_WRITE;
extern bool SQL_IMAGEWRITE;
extern int AISQL_IMAGE_WIDTH;
extern int AISQL_IMAGE_HEIGHT;
extern int AISQL_IMAGE_QALITY;
extern std::wstring AISQL_IMAGE_FORMAT; //JPG or PNG


