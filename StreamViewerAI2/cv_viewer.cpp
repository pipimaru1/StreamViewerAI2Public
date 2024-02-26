
#include "stdafx.h"

#include "framework.h"
#include "mylib.h"
#include "yolov5_engine.h"
#include "logfile.h"
#include "pose.h"
#include "StreamVierwer.h"

#define DRAW_ALL 0
#define DRAW_UPLEFT 1
#define DRAW_UPRIGHT 2
#define DRAW_DOWNLEFT 3
#define DRAW_DOWNRIGHT 4

//リフレッシュレートと画面切替時間
#define INIT_FPS_MSEC 33 //1500
#define INIT_INTERVAL_TIME_SEC 30 //8

std::atomic<bool> cvw_stop = true;
std::atomic<bool> cvw_set_stop = false;
std::atomic<bool> cvw_file_processing = false;
std::atomic<bool> cvw_file_end = false;

//volatile int display_time_seconds = INIT_INTERVAL_TIME_SEC;// 8;
std::atomic<int>  display_time_seconds = INIT_INTERVAL_TIME_SEC;// 8;
std::atomic<int>  fram_interval_ms = INIT_FPS_MSEC;// 100 sleep
std::atomic<bool> ai_text_output=false;
std::atomic<bool> Next_source = false; //次の画面に行く
std::atomic<bool> drawing_flag=false; //AIの演算が重なるのを防ぐ


int capture_error_count = 0;
#define CAPTURE_ERRROR_COUNT_MAX 0

#define CAPTURE_WAIT_TIME 10

//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::mutex Mutex_DrawCycle_Ai;

my_video_writer mvw_org;	//入力画像の録画クラス
my_video_writer mvw_ai;		//AI画像の録画クラス

int set_display_time_seconds(int _display_time_seconds)
{
	display_time_seconds = _display_time_seconds;
	return display_time_seconds;
}

int get_display_time_seconds()
{
	return display_time_seconds;
}

int set_frame_between_time(int _sleep)
{
	fram_interval_ms = _sleep;
	return fram_interval_ms;
}
int get_frame_between_time()
{
	return fram_interval_ms;
}
bool get_cvw_stop()
{
	return cvw_stop;
}

bool svw_wait_stop()
{
	while (!get_cvw_stop()) //これを入れるとdoeventsで無限
	{
		Sleep(100);
	}
	return get_cvw_stop();
}

int set_score_threshold(float _st, void* _pt_YoloObjectDetection)
{
	//明示的にキャスト 
	YoloObjectDetection* pt_YoloObjectDetection = (YoloObjectDetection*)_pt_YoloObjectDetection;
	if (pt_YoloObjectDetection == nullptr)
		return -1;
	else
		pt_YoloObjectDetection->YP.score_threshold = _st;
	return 0;
}
int set_nms_threshold(float _nms, void* _pt_YoloObjectDetection)
{
	//明示的にキャスト 
	YoloObjectDetection* pt_YoloObjectDetection = (YoloObjectDetection*)_pt_YoloObjectDetection;
	if (pt_YoloObjectDetection == nullptr)
		return -1;
	else
		pt_YoloObjectDetection->YP.nms_threshold = _nms;
	return 0;
}

int set_conf_threshold(float _conf, void* _pt_YoloObjectDetection)
{
	//明示的にキャスト 
	YoloObjectDetection* pt_YoloObjectDetection = (YoloObjectDetection*)_pt_YoloObjectDetection;
	if (pt_YoloObjectDetection == nullptr)
		return -1;
	else
		pt_YoloObjectDetection->YP.confidence_thresgold = _conf;
	return 0;
}

//trueでストップ
bool set_cvw_stop(bool _cvw_set_stop)
{
	cvw_set_stop = _cvw_set_stop;
	return cvw_set_stop;
}

void wait_drawing()
{
	while (drawing_flag)
	{
		DoEvents();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 0 正常
// 1 キャプチャー画像の異常
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
)
{
	//必要なMat
	cv::Mat image_input;
	cv::Mat image_resized;
	cv::Mat image_post_proccesed;

	//CSV出力
	std::string _ost;
	std::ostringstream _header;

	if (_pt_YoloObjectDetection == nullptr)
	{
		return 1;
	}

	//排他処理 他のスレッドが終わるまで待つ
	while (Mutex_DrawCycle_Ai.try_lock() == false)
	{
		DoEvents();
		Sleep(CAPTURE_WAIT_TIME);
	}

	//画像のリード
	_pt_capture->read(image_input);
	//TRYCAT( _capture.read(image_input));

	//画像データに異常があれば以下の処理を飛ばす
	if (image_input.empty() || image_input.rows == 0 || image_input.cols == 0)
	{
		Mutex_DrawCycle_Ai.unlock();
		return 1;
	}

	//ここでリサイズ
	cv::resize(
		image_input,
		image_resized,
		cv::Size(),
		(double)_pt_YoloObjectDetection->YP.input_width / image_input.cols,
		(double)_pt_YoloObjectDetection->YP.input_height / image_input.rows);

	_header << "AI,\"" << _str_source1 << "\"," << _str_source2 << "," << TimeStumpStr();
	//AIの処理
	_pt_YoloObjectDetection->_pre_process(image_resized);
	image_post_proccesed = _pt_YoloObjectDetection->_post_process(true, image_input.clone(), _header.str().c_str(), _ost);

	Mutex_DrawCycle_Ai.unlock();

	mvw_ai.write(image_post_proccesed);

	//描画
	rec_stump(image_post_proccesed, mvw_ai.output.isOpened());
	DrawPicToHDC(image_post_proccesed, hWnd, hDC, _keep_aspect, _draw_area);//アスペクト比を維持する。

	///////////////////////////////////
	//解析データの保存処理
	_ai_out_put_string = _ai_out_put_string + "\n" + _ost;
	if (ai_text_output)
	{
		//aiの出力をテキストで画面にバーっと出す。ターミネーター、のようにはならなかった。
		MyDrawText(hWnd, hDC, _ai_out_put_string, 5, 50);
	}

	//AIの出力をテキストファイルに保存
	if (_pAICSV != nullptr)
	{
		*_pAICSV << _ost;
		_pAICSV->flush();
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 0 正常
// 1 キャプチャー画像に異常がある
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
)
{
	//必要なMat
	cv::Mat image_input;
	cv::Mat image_resized;
	cv::Mat image_post_proccesed;
	//cv::Mat image_input_gray;

	while (Mutex_DrawCycle_Ai.try_lock() == false)
	{
		DoEvents();
		Sleep(50);
	}
	//画像のリード
	//TRYCAT(_capture.read(image_input));
	TRYCAT(_pt_capture->read(image_input));
	//cvtColor(image_input, image_input_gray, cv::COLOR_BGR2GRAY);

	if (image_input.empty() || image_input.rows == 0 || image_input.cols == 0)
	{
		Mutex_DrawCycle_Ai.unlock();
		return 1;
	}
	image_post_proccesed = _pt_pose->run_pose_multi(image_input, OUTPUT_IMAGE_MODE_GRAY); //中でTRYCATを呼んでいる
	//image_post_proccesed = _pt_pose->run_pose_multi(image_input, OUTPUT_IMAGE_MODE_NORMAL); //中でTRYCATを呼んでいる
	Mutex_DrawCycle_Ai.unlock();

	mvw_ai.write(image_post_proccesed);
	DrawPicToHDC(image_post_proccesed, hWnd, hDC, _keep_aspect, _draw_area, false);//アスペクト比を維持する。
	//DrawPicToHDC(image_post_proccesed, hWnd, hDC, _keep_aspect, _draw_area, true);//アスペクト比を維持する。

	return 0;
}

//1:サイクル　0:サイクルしない
bool timer_cycle_mode=1;
bool PROC_CYCLE(bool _mode)
{
	timer_cycle_mode = _mode;
	return timer_cycle_mode;
}

int fix_cam_number = -1;
int PROC_FIX_CAM(int _cam_number)
{
	fix_cam_number = _cam_number;
	return fix_cam_number;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	int _stump_mode
)
{
	cv::Mat image_input;
	cv::Mat image_resized;
	cv::Mat image_post_proccesed;

	bool _next_source = Next_source; //次の画面に行く
	auto _start = std::chrono::steady_clock::now();

	LOGMSG(_str_source1);
	LOGMSG(_str_source2);

	bool _capture_success = true;
	int _ret = 0;
	
	while (_capture_success)
	{
		if (_DrawCycleMode == DRAWCYCLE_YOLO5 || _DrawCycleMode == DRAWCYCLE_YOLO8)
		{
			_ret=DrawCycle_Core_Yolo(
				_DrawCycleMode,
				hWnd,
				hDC,
				_draw_area,
				_keep_aspect,
				//_capture,
				_pt_capture,
				_pt_YoloObjectDetection,
				//	PoseNet* _pt_pose,
				_str_source1,
				_str_source2,
				_ai_out_put_string,
				_pAICSV
				//	int _display_time_seconds
			);
			if (_ret)
				break;
		}
		else if (_DrawCycleMode == DRAWCYCLE_POSENET) //pose
		{
			_ret = DrawCycle_Core_Pose(
				_DrawCycleMode,
				hWnd,
				hDC,
				_draw_area,
				_keep_aspect,
				//_capture,
				_pt_capture,
				//YoloObjectDetection* _pt_YoloObjectDetection,
				_pt_pose
				//const std::string& _str_source1,
				//const std::string& _str_source2,
				//std::string& _ai_out_put_string,
				//std::ofstream* _pAICSV,
				//int _display_time_seconds
			);
			if (_ret)
				break;
		}
		else
		{
			//_capture >> image_input;
			//*_pt_capture >> image_input;
			_capture_success=_pt_capture->read(image_input);

			if(_stump_mode & FLAG_TIMESTUMP)
				time_stump(image_input,1.0);
			mvw_org.write(image_input);
			rec_stump(image_input, mvw_org.output.isOpened());
			DrawPicToHDC(image_input, hWnd, hDC, _keep_aspect, _draw_area);//アスペクト比を維持する。
			//DrawPicToHDC(image_input, hWnd, hDC, _keep_aspect, _spl_w, _spl_h, _num_w, _num_h);//アスペクト比を維持する。
		}

		////////////////////////////
		//終了処理を優先する
		if (cvw_set_stop == true)
		{
			LOGMSG2("cvw_set_stop==true", _str_source2);
			break;
		}

		////////////////////////////
		//タイマー 時間のチェック
		auto _now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(_now - _start).count() >= _display_time_seconds && _display_time_seconds != 0)
			_next_source = true;
		if (timer_cycle_mode == 0 || display_time_seconds==0)
			_next_source = false; //次の画面に行かないようにする

		////////////////////////////
		//システムに処理を渡し、指定の時間スリープ
		//切り替えの感じが不規則なので描画ルーチンより多めに回す
		int _roop = 4;
		for (int i = 0; i < _roop; i++)
		{
			DoEvents();
			//Sleep(fram_interval_ms / _roop);
			std::this_thread::sleep_for(std::chrono::milliseconds(fram_interval_ms)/ (float)_roop);
		}

		//グローバル変数をチェックし、スペースが押されていたら次のカメラに行く
		//全画面でない場合は無視
		//なんかもっといいアルゴリズムあるんじゃないの?
		if (_draw_area.spl_w != 1 || _draw_area.spl_h != 1)
			Next_source = false;
		if(Next_source)
			_next_source = true;
		if (_next_source)
		{
			_next_source = false;
			Next_source = false;
			break;
		}
	}
	//	BusyAI = false;
	return _capture_success;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//カメラのオープン
int CameraOpenCycle(
	int _DrawCycleMode,
	HWND hWnd,		//Windowハンドル
	//int _draw_area,
	//bool _keep_aspect,
	//cv::VideoCapture& _capture,
	cv::VideoCapture* _pt_capture,
	YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose,
	std::vector<std::vector<std::string>> _urls,
	int _current_url_index,
	bool& _usbcam
//	std::ofstream* _pAICSV
)
{
	bool _cam_on = false;
	//int _current_url_index = 0;
	std::string _ai_out_put_string;
	std::string _ai_file_name = "";

	//HDC hDC = ::GetDC(hWnd);

	//明示的にキャスト 
	if (_DrawCycleMode == DRAWCYCLE_YOLO5 && _pt_YoloObjectDetection != nullptr)
		_ai_file_name = _pt_YoloObjectDetection->YP.onnx_file_name;
	else if (_DrawCycleMode == DRAWCYCLE_YOLO8 && _pt_YoloObjectDetection != nullptr)
		_ai_file_name = _pt_YoloObjectDetection->YP.onnx_file_name;
	else if (_DrawCycleMode == DRAWCYCLE_POSENET && _pt_pose != nullptr)
		_ai_file_name = _pt_pose->get_weightsFile();
//	else
//		LOGMSGX("Unmatch AI model and names file");

	//while文
	while (_cam_on == false)
	{
		//ストップ指示があったら、カメラ切り替えループを出る
		if (cvw_set_stop == true)
		{
			_cam_on = false; //念のため
			break;
		}
		//ウィンドウキャプションの文字列
		std::string _ipurl = _urls[_current_url_index][1];
		std::ostringstream _window_caption;
		_window_caption << _urls[_current_url_index][0] << " " << _ai_file_name;

		std::wstring newTitleW = stringToWstring(_window_caption.str().c_str());
		SetWindowTextW(hWnd, newTitleW.c_str());

		//bool return_cap;
		int _num_usb;
		std::istringstream _iss(_ipurl);
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		//カメラオープンブロック
		{
			std::atomic<bool> _connected;
			//数値変換出来たらUSBカメラ
			if (_iss >> _num_usb)
			{
				//TRYCAT(_pt_capture->open(_num_usb));
				//std::thread tryConnect(_num_usb, _pt_capture, _connected);
				std::thread connectionThread(tryConnect_usb, _num_usb, _pt_capture, std::ref(_connected));
				_usbcam = true;
				catchConnect(_pt_capture, _connected, connectionThread);
				if (!_connected) {
					LOGMSG2X("接続タイムアウト", _num_usb);
					break;
				}
			}//それ以外なら
			else
			{
				//TRYCAT(_capture.open(_ipurl));
				//TRYCAT(_pt_capture->open(_ipurl));
				std::thread connectionThread(tryConnect_url, _ipurl, _pt_capture, std::ref(_connected));
				_usbcam = false;
				catchConnect(_pt_capture, _connected, connectionThread);
				if (!_connected) {
					LOGMSG2X("接続タイムアウト",_ipurl);
					break;
				}
			}
			
		}
		//テキストをクリア
		_ai_out_put_string = "";
		DoEvents();
		//if (_capture.isOpened())
		if (_pt_capture->isOpened())
		{
			_cam_on = true;

			//bool _FPSSET = _capture.set(cv::CAP_PROP_FPS, 1000 / fram_interval_ms);
			//double _FPS = _capture.get(cv::CAP_PROP_FPS);
			bool _FPSSET = _pt_capture->set(cv::CAP_PROP_FPS, 1000 / fram_interval_ms);
			double _FPS = _pt_capture->get(cv::CAP_PROP_FPS);
			LOGMSG2("Camera set FPS ", _FPSSET);
			LOGMSG2("Camera FPS ", _FPS);
		}
		else
		{
			LOGMSG2("Failed to open the stream ", _ipurl);
			LOGMSG2("_urls[_current_url_index][0] = ", _urls[_current_url_index][0]);
			LOGMSG2("_urls[_current_url_index][1] = ", _urls[_current_url_index][1]);
			//std::cerr << "Failed to open the stream: " << _ipurl << std::endl;
			_cam_on = false;
			capture_error_count++;
			if (capture_error_count > CAPTURE_ERRROR_COUNT_MAX)
			{
				LOGMSG2X("Over max count of capture error, go to next camera", _ipurl);
				std::ostringstream _msg_str;
				if (0)
				{
					_msg_str << "指定先のカメラと接続できませんでした。" << std::endl
						<< "カメラリストファイルを見直してください。" << std::endl
						//<< url_file << std::endl
						//<< "カメラ情報" << std::endl これ入れるとバグる　なぜ?
						<< _urls[_current_url_index][0] << std::endl
						<< _urls[_current_url_index][1] << std::endl;

					std::wstring _lpcstr_tmp =  _A2CW(_msg_str.str().c_str());
 					//LPCWSTR _lpcstr_tmp = _A2CW(_msg_str.str().c_str());
					int msgboxID = MessageBoxW(hWnd, _lpcstr_tmp.c_str(), L"StreamViewerAI2.exe", MB_ICONWARNING | MB_OK);
				}
				//cvw_set_stop = true; //ストップはしない
				_cam_on = false;
				break;//次のカメラへ
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//カメラのオープンする
//カメラがオープンしたら、DrawCycleを処理し描画サイクルを処理させる。
//DrawCycleから戻ってきたら次のカメラをオープンする。
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int Count_VideoCapture=0;
int DrawCV2Window(
	int _DrawCycleMode,
	HWND hWnd,		//Windowハンドル
	HDC hDC,
	DrawArea _draw_area,
	bool _keep_aspect,
	YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose,
	std::vector<std::vector<std::string>> _urls,
	std::ofstream* _pAICSV
)
{
	bool _cam_on = false;
	//bool _cycle = true;
	bool _cycle = true;
	bool _usbcam = false;
	cvw_stop = 0;

	std::string _ai_out_put_string;
	std::string _ai_file_name = "";
	int _current_url_index = 0;
	//int Current_url_index = 0;

	//cv::VideoCapture _capture;
	//cv::VideoCapture* _pt_capture;

	//HDC hDC = ::GetDC(hWnd);
	//	ここからカメラ切り替えサイクル
	while (_cycle) //
	{
		//HDC hDC = ::GetDC(hWnd);
		{
			cv::VideoCapture* _pt_capture;
			_pt_capture = new cv::VideoCapture;
			//LOGMSG2("Current_url_index", Current_url_index);
			if (fix_cam_number != -1)
			{
				//カメラ番号の指定があればそれを指定する。スパゲッティだなあ。
				_current_url_index = fix_cam_number;
			}
			int _ret = CameraOpenCycle(
				_DrawCycleMode,
				hWnd,		//Windowハンドル
				//int _draw_area,
				//bool _keep_aspect,
				//_capture,
				_pt_capture,
				_pt_YoloObjectDetection,
				_pt_pose,
				_urls,
				_current_url_index,
				_usbcam
				//	std::ofstream* _pAICSV
			);
			if (_ret)
				Count_VideoCapture++;
			//画面を黒く塗りつぶす
			if (_keep_aspect && fix_cam_number == -1)
			{
				HDC _hDC = GetDC(hWnd);
				AllPaintBlack(hWnd, _hDC, _draw_area);
				ReleaseDC(hWnd, _hDC);
			}
			////////////////////////////////////////////////////////////////
			//描画サイクル
			DrawCycle(
				_DrawCycleMode,
				hWnd,
				hDC,
				_draw_area,
				_keep_aspect,
				//_capture,
				_pt_capture,
				_pt_YoloObjectDetection,
				_pt_pose,
				_urls[_current_url_index][0],
				_urls[_current_url_index][1],
				_ai_out_put_string,
				_pAICSV,
				display_time_seconds,
				FLAG_TIMESTUMP
			);

			//_capture.release();
			_pt_capture->release();

			delete _pt_capture;
			_pt_capture = nullptr;
			if (_ret)
				Count_VideoCapture--;
			LOGMSG2("Count_VideoCapture", Count_VideoCapture);
		}
		_cam_on = false;

		//カメラ切り替えループを出る
		if (cvw_set_stop == true)
			break;

		_current_url_index = (_current_url_index + 1) % (int)_urls.size();
	}
	//処理がストップしたことをメインに伝える
	cvw_stop = true;

	//ReleaseDC(hWnd, hDC);

	mvw_org.release();
	mvw_ai.release();

	//マルチスレッドなので戻り値は無視される
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int DrawCV2Windowf(
	int _DrawCycleMode,
	HWND hWnd,		//Windowハンドル
	HDC hDC,
	DrawArea _draw_area,
	bool _keep_aspect,
	//	void* _pt_ai,	//yolov5処理クラスのインスタンスへのポインタ nullだと無視
	YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose,
	std::string _file_name,
	std::ofstream* _pAICSV
)
{
	bool _cam_on = false;
	cvw_stop = 0;
	cv::VideoCapture _capture;

	std::string _ai_out_put_string;

	//Windowバーにファイル名を表示
	std::string ai_file_name = "";
	if (_DrawCycleMode == DRAWCYCLE_YOLO5 && _pt_YoloObjectDetection!=nullptr)
		ai_file_name = _pt_YoloObjectDetection->YP.onnx_file_name;
	if (_DrawCycleMode == DRAWCYCLE_YOLO8 && _pt_YoloObjectDetection != nullptr)
		ai_file_name = _pt_YoloObjectDetection->YP.onnx_file_name;
	else if(_DrawCycleMode == DRAWCYCLE_POSENET && _pt_pose!=nullptr)
		ai_file_name = _pt_pose->get_weightsFile();

	int current_url_index = 0;

	//ウィンドウキャプションの文字列
	std::ostringstream window_caption;
	window_caption << _file_name << " " << ai_file_name;

	std::wstring newTitleW = stringToWstring(window_caption.str().c_str());
	SetWindowTextW(hWnd, newTitleW.c_str());

	_capture.open(_file_name);

	//画面を黒く塗りつぶす
	AllPaintBlack(hWnd, hDC, _draw_area);
	//テキストをクリア
	_ai_out_put_string = "";
	DoEvents();
	if (_capture.isOpened())
	{
		_cam_on = true;
	}
	else
	{
		std::cerr << "Failed to open the stream: " << _file_name << std::endl;
		_cam_on = false;
	}

	cvw_file_end = false;

	DrawCycle(
		_DrawCycleMode,
		hWnd,
		hDC,
		_draw_area,
		_keep_aspect,
		&_capture,
		_pt_YoloObjectDetection,
		_pt_pose,
		_file_name,
		"",
		_ai_out_put_string,
		_pAICSV,
		0,
		0 //TIMESTUMPなし
	);

	_capture.release();

	//処理が終わったことをwinprocに伝えるためのフラグ
	cvw_file_processing = false;
	cvw_file_end = true;

	//↓これ要るか?
	cvw_stop = true;

	//ReleaseDC(hWnd, hDC);
	mvw_org.release();
	mvw_ai.release();
	return 0;
}

