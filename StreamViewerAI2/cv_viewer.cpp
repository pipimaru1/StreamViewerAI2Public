
#include "stdafx.h"

#include "framework.h"
#include "mylib.h"
#include "yolov5_engine.h"
#include "logfile.h"
#include "pose.h"
#include "StreamVierwer.h"
#include "sqlutil.h"

#define DRAW_ALL 0
#define DRAW_UPLEFT 1
#define DRAW_UPRIGHT 2
#define DRAW_DOWNLEFT 3
#define DRAW_DOWNRIGHT 4

//���t���b�V�����[�g�Ɖ�ʐؑ֎���
#define INIT_FPS_MSEC 33 //1500
#define INIT_INTERVAL_TIME_SEC 30 //8

std::atomic<bool> CVW_STOP = true;
std::atomic<bool> cvw_set_stop = false;
std::atomic<bool> CVW_FILE_PROCESSING = false;
std::atomic<bool> CVW_FILE_END = false;

//volatile int DISPLAY_TIME_SECOND = INIT_INTERVAL_TIME_SEC;// 8;
std::atomic<int>  DISPLAY_TIME_SECOND = INIT_INTERVAL_TIME_SEC;// 8;
std::atomic<int>  FRAME_INTERVAL_MS = INIT_FPS_MSEC;// 100 sleep
std::atomic<bool> AI_TEXT_OUTPUT=false;
std::atomic<bool> NEXT_SOURCE = false; //���̉�ʂɍs��
std::atomic<bool> drawing_flag=false; //AI�̉��Z���d�Ȃ�̂�h��

bool SQL_WRITE = 0;
bool SQL_IMAGEWRITE = 0;
SqlServer SqlServerAi;
SqlServer SqlServerImage;

int capture_error_count = 0;
#define CAPTURE_ERRROR_COUNT_MAX 0

#define CAPTURE_WAIT_TIME 10

//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::mutex Mutex_DrawCycle_Ai;

MyVideoWriter mlVIDEOWRITERORG;	//���͉摜�̘^��N���X
MyVideoWriter mlVIDEOWRITERAI;		//AI�摜�̘^��N���X

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int set_display_time_seconds(int _display_time_seconds)
{
	DISPLAY_TIME_SECOND = _display_time_seconds;
	return DISPLAY_TIME_SECOND;
}

int get_display_time_seconds()
{
	return DISPLAY_TIME_SECOND;
}

int set_frame_between_time(int _sleep)
{
	FRAME_INTERVAL_MS = _sleep;
	return FRAME_INTERVAL_MS;
}
int get_frame_between_time()
{
	return FRAME_INTERVAL_MS;
}
bool get_cvw_stop()
{
	return CVW_STOP;
}

bool svw_wait_stop()
{
	while (!get_cvw_stop()) //����������doevents�Ŗ���
	{
		Sleep(100);
	}
	return get_cvw_stop();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define AICSVFILE "AI_"
#define AICSVEXT ".CSV"
std::string AICSVPATH = ".\\";
std::mutex FILE_MUTEX;
//std::atomic<bool> fileatomic = false;
std::ofstream* pAICSV = nullptr;
extern bool AI_DATA_CSV_WRITE=true; //�t�@�C���ɏ������܂Ȃ��Ƃ���false �I�[�o�[�t���[�h�~ 
extern bool AI_DATA_CSV_OVER_WRITE=false; //�㏑������Ƃ���true �ǉ��������݂�false
long _csv_create_date=0;

int open_ai_csv_file()
{
	//�X���b�h�Z�[�t��
	std::lock_guard<std::mutex> lock(FILE_MUTEX);
	long _csv_create_date_new=GetCurrentDateLong();

	if (_csv_create_date != _csv_create_date_new)
	{
		_csv_create_date = _csv_create_date_new;
		if (pAICSV != nullptr)
		{
			pAICSV->close();
			delete pAICSV;
			pAICSV = nullptr;
		}
		std::string CSVFILE = AICSVPATH + AICSVFILE + GetCurrentDateString() + AICSVEXT;
		if (AI_DATA_CSV_OVER_WRITE)
			pAICSV = new std::ofstream(CSVFILE, std::ios_base::out); //�㏑��
		else
			pAICSV = new std::ofstream(CSVFILE, std::ios_base::app); //�ǉ���������

	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int set_score_threshold(float _st, void* _pt_YoloObjectDetection)
{
	//�����I�ɃL���X�g 
	YoloObjectDetection* ptYoloOBJECTDECTECTION = (YoloObjectDetection*)_pt_YoloObjectDetection;
	if (ptYoloOBJECTDECTECTION == nullptr)
		return -1;
	else
		ptYoloOBJECTDECTECTION->YP.score_threshold = _st;
	return 0;
}
int set_nms_threshold(float _nms, void* _pt_YoloObjectDetection)
{
	//�����I�ɃL���X�g 
	YoloObjectDetection* ptYoloOBJECTDECTECTION = (YoloObjectDetection*)_pt_YoloObjectDetection;
	if (ptYoloOBJECTDECTECTION == nullptr)
		return -1;
	else
		ptYoloOBJECTDECTECTION->YP.nms_threshold = _nms;
	return 0;
}

int set_conf_threshold(float _conf, void* _pt_YoloObjectDetection)
{
	//�����I�ɃL���X�g 
	YoloObjectDetection* ptYoloOBJECTDECTECTION = (YoloObjectDetection*)_pt_YoloObjectDetection;
	if (ptYoloOBJECTDECTECTION == nullptr)
		return -1;
	else
		ptYoloOBJECTDECTECTION->YP.confidence_thresgold = _conf;
	return 0;
}

//true�ŃX�g�b�v
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

int sql_image(
	const std::string& _str_location,
	const std::string& _str_camera_url,
	cv::Mat image_input
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
	//cv::VideoCapture* _pt_capture,
	cv::Mat& image_input,
	YoloObjectDetection* _pt_YoloObjectDetection,
	const std::string& _str_location,
	const std::string& _str_camera_url,
	std::string& _ai_out_put_string
)
{
	//�K�v��Mat
	//cv::Mat image_input;
	cv::Mat image_resized;
	cv::Mat image_post_proccesed;

	//CSV�o��
	std::string _string_ai_csv;
	//std::ostringstream _header;

	if (_pt_YoloObjectDetection == nullptr)
	{
		return 1;
	}

	//�r������ ���̃X���b�h���I���܂ő҂�
	while (Mutex_DrawCycle_Ai.try_lock() == false)
	{
		DoEvents();
		Sleep(CAPTURE_WAIT_TIME);
	}

	//�摜�̃��[�h
	//_pt_capture->read(image_input);
	//TRYCAT( _capture.read(image_input));

	//�摜�f�[�^�Ɉُ킪����Έȉ��̏������΂�
	if (image_input.empty() || image_input.rows == 0 || image_input.cols == 0)
	{
		Mutex_DrawCycle_Ai.unlock();
		return 1;
	}

	//�����Ń��T�C�Y
	cv::resize(
		image_input,
		image_resized,
		cv::Size(),
		(double)_pt_YoloObjectDetection->YP.input_width / image_input.cols,
		(double)_pt_YoloObjectDetection->YP.input_height / image_input.rows);

	AiRecordHeader _ai_rec_header;
	_ai_rec_header.Category = L"AI";
	_ai_rec_header.Location = string2wstring(_str_location);
	_ai_rec_header.StreamURL = string2wstring(getIPAddress(_str_camera_url));
	_ai_rec_header.Timestamp = string2wstring(TimeStumpStr());

	//_header << "AI,\"" << _str_location << "\"," << _str_camera_url << "," << TimeStumpStr();
	std::vector<AiSqlOutput> _sql_ai_data;

	//AI�̏���
	_pt_YoloObjectDetection->_pre_process(image_resized);
	//image_post_proccesed = _pt_YoloObjectDetection->_post_process(true, image_input.clone(), _header.str().c_str(), _string_ai_csv);
	image_post_proccesed = _pt_YoloObjectDetection->_post_process(true, image_input.clone(), _ai_rec_header, _string_ai_csv, _sql_ai_data);

	Mutex_DrawCycle_Ai.unlock();

	mlVIDEOWRITERAI.write(image_post_proccesed);

	//�`��
	rec_stump(image_post_proccesed, mlVIDEOWRITERAI.output.isOpened());
	DrawPicToHDC(image_post_proccesed, hWnd, hDC, _keep_aspect, _draw_area);//�A�X�y�N�g����ێ�����B

	///////////////////////////////////
	//��̓f�[�^�̕ۑ�����
	_ai_out_put_string = _ai_out_put_string + "\n" + _string_ai_csv;
	if (AI_TEXT_OUTPUT)
	{
		//ai�̏o�͂��e�L�X�g�ŉ�ʂɃo�[���Əo���B�^�[�~�l�[�^�[�A�̂悤�ɂ͂Ȃ�Ȃ������B
		MyDrawText(hWnd, hDC, _ai_out_put_string, 5, 50);
	}
	///////////////////////////////////
	//��̓f�[�^�̕ۑ�����SQL
	//�T�[�o�[�ƃf�[�^�i�[��̐ݒ�
	if (SQL_WRITE) 
	{
		if(_sql_ai_data.size()!=0)
			Sql_Write(SqlServerAi, _sql_ai_data, _sql_ai_data.size());
	}
	//sql_image(_str_location, _str_camera_url, image_input);

	///////////////////////////////////
	//AI�̏o�͂��e�L�X�g�t�@�C���ɕۑ�
	//�X���b�h�Z�[�t��
	if (AI_DATA_CSV_WRITE)
	{
		std::lock_guard<std::mutex> lock(FILE_MUTEX);
		if (pAICSV != nullptr)
		{
			if (pAICSV->is_open())
			{
				*pAICSV << _string_ai_csv;
				pAICSV->flush();
			}

		}
	}
	return 0;
}

std::mutex MUTEX_SQL;

int sql_image(
	const std::string& _str_location,
	const std::string& _str_camera_url,
	cv::Mat image_input
){
	//std::atomic<bool> _sql_image;
	std::lock_guard<std::mutex> lock(MUTEX_SQL);

	if (image_input.empty() || image_input.rows == 0 || image_input.cols == 0)
	{
		return 0;
	}

//	if (0)
	if (SQL_IMAGEWRITE)
	{
		SqlImageOutput _sql_image_data;
		_sql_image_data.StreamURL = string2wstring(getIPAddress(_str_camera_url));
		_sql_image_data.Category = L"AI";
		_sql_image_data.EventID = EventID;
		_sql_image_data.Location = string2wstring(_str_location);
		_sql_image_data.Timestamp = string2wstring(TimeStumpStr());
		//_sql_image_data.image = image_input.clone(); //	�N���[�����Ȃ��Ă��������ȂƂ͎v�����ꉞ ���T�C�Y����Ȃ炱��
		//cv::resize(image_input, _sql_image_data.image, cv::Size(320, 240));//���̃N���X�̃T�C�Y�Ȃ�s������
		//cv::resize(image_input, _sql_image_data.image, cv::Size(640, 480));
		cv::resize(image_input, _sql_image_data.image, cv::Size(AISQL_IMAGE_WIDTH,AISQL_IMAGE_HEIGHT));

		_sql_image_data.ImageWidth = _sql_image_data.image.cols;
		_sql_image_data.ImageHeight = _sql_image_data.image.rows;

		if (AISQL_IMAGE_FORMAT == L"JPG")
		{
			_sql_image_data.CompressOption = { cv::IMWRITE_JPEG_QUALITY, AISQL_IMAGE_QALITY };
			_sql_image_data.CompressType = L"JPEG";
		}
		else
		{
			_sql_image_data.CompressOption = { cv::IMWRITE_PNG_COMPRESSION, 9 };
			_sql_image_data.CompressType = L"PNG";
		}


		//Sql_ImageWriteW(SqlServerImage, _sql_image_data);
		Sql_ImageWriteChunksW(SqlServerImage, _sql_image_data);
		//Sql_ImageWriteS(SqlServerImage, _sql_image_data);
	}
	//�b��[�u SQL����肭�����Ȃ�������ŁA�C���f�b�N�X�t�H���_�ɕۑ�
	if (SQL_IMAGEWRITE)
	{
		//SqlImageOutput _sql_image_data;
		cv::Mat _image;
		std::wstring _StreamURL = string2wstring(getIPAddress(_str_camera_url));
		cv::resize(image_input, _image, cv::Size(1280, 960));
		std::wstring _filepath = SqlServerImage.image_index_folder + _StreamURL + L".jpg";
		cv::imwrite(W2S(_filepath.c_str()), _image);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 0 ����
// 1 �L���v�`���[�摜�Ɉُ킪����
int DrawCycle_Core_Pose(
	int _DrawCycleMode,
	HWND hWnd,
	HDC hDC,
	DrawArea _draw_area,
	bool _keep_aspect,
	//cv::VideoCapture* _pt_capture,
	cv::Mat& image_input,
	PoseNet* _pt_pose
)
{
	//�K�v��Mat
	//cv::Mat image_input;
	cv::Mat image_resized;
	cv::Mat image_post_proccesed;
	//cv::Mat image_input_gray;

	while (Mutex_DrawCycle_Ai.try_lock() == false)
	{
		DoEvents();
		Sleep(50);
	}
	//�摜�̃��[�h
	//TRYCAT(_capture.read(image_input));
	//TRYCAT(_pt_capture->read(image_input));
	//cvtColor(image_input, image_input_gray, cv::COLOR_BGR2GRAY);

	if (image_input.empty() || image_input.rows == 0 || image_input.cols == 0)
	{
		Mutex_DrawCycle_Ai.unlock();
		return 1;
	}
	image_post_proccesed = _pt_pose->run_pose_multi(image_input, OUTPUT_IMAGE_MODE_GRAY); //����TRYCAT���Ă�ł���
	//image_post_proccesed = _pt_pose->run_pose_multi(image_input, OUTPUT_IMAGE_MODE_NORMAL); //����TRYCAT���Ă�ł���
	Mutex_DrawCycle_Ai.unlock();

	mlVIDEOWRITERAI.write(image_post_proccesed);
	DrawPicToHDC(image_post_proccesed, hWnd, hDC, _keep_aspect, _draw_area, false);//�A�X�y�N�g����ێ�����B
	//DrawPicToHDC(image_post_proccesed, hWnd, hDC, _keep_aspect, _draw_area, true);//�A�X�y�N�g����ێ�����B

	return 0;
}

//1:�T�C�N���@0:�T�C�N�����Ȃ�
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
	const std::string& _str_location,
	const std::string& _str_camera_url,
	std::string& _ai_out_put_string,
//	std::ofstream* _pAICSV,
	int _display_time_seconds,
	int _stump_mode
)
{
	cv::Mat image_input;
	cv::Mat image_resized;
	cv::Mat image_post_proccesed;

	bool _next_source = NEXT_SOURCE; //���̉�ʂɍs��
	auto _start = std::chrono::steady_clock::now();

	LOGMSG(_str_location);
	LOGMSG(_str_camera_url);

	bool _capture_success = true;
	int _ret = 0;
	
	while (_capture_success)
	{
		_capture_success=_pt_capture->read(image_input);
		
		if (_DrawCycleMode == DRAWCYCLE_YOLO5 || _DrawCycleMode == DRAWCYCLE_YOLO8)
		{
			_ret=DrawCycle_Core_Yolo(
				_DrawCycleMode,
				hWnd,
				hDC,
				_draw_area,
				_keep_aspect,
				//_pt_capture,
				image_input,
				_pt_YoloObjectDetection,
				_str_location,
				_str_camera_url,
				_ai_out_put_string
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
				//_pt_capture,
				image_input,
				_pt_pose
			);
			if (_ret)
				break;
		}
		else
		{
			//_capture_success=_pt_capture->read(image_input);

			if(_stump_mode & FLAG_TIMESTUMP)
				time_stump(image_input,1.0);
			//sql_image(_str_location, _str_camera_url, image_input);//sql�T�[�o�[�ɕۑ�
			mlVIDEOWRITERORG.write(image_input);
			rec_stump(image_input, mlVIDEOWRITERORG.output.isOpened());
			DrawPicToHDC(image_input, hWnd, hDC, _keep_aspect, _draw_area);//�A�X�y�N�g����ێ�����B
			//DrawPicToHDC(image_input, hWnd, hDC, _keep_aspect, _spl_w, _spl_h, _num_w, _num_h);//�A�X�y�N�g����ێ�����B
		}

		////////////////////////////
		//�I��������D�悷��
		if (cvw_set_stop == true)
		{
			LOGMSG2("cvw_set_stop==true", _str_camera_url);
			break;
		}

		////////////////////////////
		//�^�C�}�[ ���Ԃ̃`�F�b�N
		auto _now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(_now - _start).count() >= _display_time_seconds && _display_time_seconds != 0)
			_next_source = true;
		if (timer_cycle_mode == 0 || DISPLAY_TIME_SECOND==0)
			_next_source = false; //���̉�ʂɍs���Ȃ��悤�ɂ���

		////////////////////////////
		//�V�X�e���ɏ�����n���A�w��̎��ԃX���[�v
		//�؂�ւ��̊������s�K���Ȃ̂ŕ`�惋�[�`����葽�߂ɉ�
		int _roop = 4;
		for (int i = 0; i < _roop; i++)
		{
			DoEvents();
			//Sleep(FRAME_INTERVAL_MS / _roop);
			std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_INTERVAL_MS)/ (float)_roop);
		}

		//�O���[�o���ϐ����`�F�b�N���A�X�y�[�X��������Ă����玟�̃J�����ɍs��
		//�S��ʂłȂ��ꍇ�͖���
		//�Ȃ񂩂����Ƃ����A���S���Y������񂶂�Ȃ���?
		if (_draw_area.spl_w != 1 || _draw_area.spl_h != 1)
			NEXT_SOURCE = false;
		if(NEXT_SOURCE)
			_next_source = true;
		if (_next_source)
		{
			_next_source = false;
			NEXT_SOURCE = false;
			break;
		}
	}
	//�J������؂�ւ��鎞�ɁASQL�T�[�o�[�ɕۑ�
	sql_image(_str_location, _str_camera_url, image_input);

	//	BusyAI = false;
	return _capture_success;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//�J�����̃I�[�v��
int CameraOpenCycle(
	int _DrawCycleMode,
	HWND hWnd,		//Window�n���h��
	cv::VideoCapture* _pt_capture,
	YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose,
	std::vector<std::vector<std::string>> _urls,
	int _current_url_index,
	bool& _usbcam
)
{
	bool _cam_on = false;
	std::string _ai_out_put_string;
	std::string _ai_file_name = "";

	//�����I�ɃL���X�g 
	if (_DrawCycleMode == DRAWCYCLE_YOLO5 && _pt_YoloObjectDetection != nullptr)
		_ai_file_name = _pt_YoloObjectDetection->YP.onnx_file_name;
	else if (_DrawCycleMode == DRAWCYCLE_YOLO8 && _pt_YoloObjectDetection != nullptr)
		_ai_file_name = _pt_YoloObjectDetection->YP.onnx_file_name;
	else if (_DrawCycleMode == DRAWCYCLE_POSENET && _pt_pose != nullptr)
		_ai_file_name = _pt_pose->get_weightsFile();
//	else
//		LOGMSGX("Unmatch AI model and names file");

	while (_cam_on == false)
	{
		//�X�g�b�v�w������������A�J�����؂�ւ����[�v���o��
		if (cvw_set_stop == true)
		{
			_cam_on = false; //�O�̂���
			break;
		}
		//�E�B���h�E�L���v�V�����̕�����
		std::string _ipurl = _urls[_current_url_index][1];
		std::ostringstream _window_caption;
		_window_caption << _urls[_current_url_index][0] << " " << _ai_file_name;

		std::wstring newTitleW = string2wstring(_window_caption.str().c_str());
		SetWindowTextW(hWnd, newTitleW.c_str());

		//bool return_cap;
		int _num_usb;
		std::istringstream _iss(_ipurl);
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		//�J�����I�[�v���u���b�N
		{
			std::atomic<bool> _connected;
			//���l�ϊ��o������USB�J����
			if (_iss >> _num_usb)
			{
				//TRYCAT(_pt_capture->open(_num_usb));
				//std::thread tryConnect(_num_usb, _pt_capture, _connected);
				std::thread connectionThread(tryConnect_usb, _num_usb, _pt_capture, std::ref(_connected));
				_usbcam = true;
				catchConnect(_pt_capture, _connected, connectionThread);
				if (!_connected) {
					LOGMSG2X("�ڑ��^�C���A�E�g", _num_usb);
					break;
				}
			}//����ȊO�Ȃ�
			else
			{
				//TRYCAT(_capture.open(_ipurl));
				//TRYCAT(_pt_capture->open(_ipurl));
				std::thread connectionThread(tryConnect_url, _ipurl, _pt_capture, std::ref(_connected));
				_usbcam = false;
				catchConnect(_pt_capture, _connected, connectionThread);
				if (!_connected) {
					LOGMSG2X("�ڑ��^�C���A�E�g",_ipurl);
					break;
				}
			}
			
		}
		//�e�L�X�g���N���A
		_ai_out_put_string = "";
		DoEvents();
		//if (_capture.isOpened())
		if (_pt_capture->isOpened())
		{
			_cam_on = true;

			//bool _FPSSET = _capture.set(cv::CAP_PROP_FPS, 1000 / FRAME_INTERVAL_MS);
			//double _FPS = _capture.get(cv::CAP_PROP_FPS);
			bool _FPSSET = _pt_capture->set(cv::CAP_PROP_FPS, 1000 / FRAME_INTERVAL_MS);
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
					_msg_str << "�w���̃J�����Ɛڑ��ł��܂���ł����B" << std::endl
						<< "�J�������X�g�t�@�C�����������Ă��������B" << std::endl
						//<< url_file << std::endl
						//<< "�J�������" << std::endl ��������ƃo�O��@�Ȃ�?
						<< _urls[_current_url_index][0] << std::endl
						<< _urls[_current_url_index][1] << std::endl;

					std::wstring _lpcstr_tmp =  _A2CW(_msg_str.str().c_str());
 					//LPCWSTR _lpcstr_tmp = _A2CW(_msg_str.str().c_str());
					int msgboxID = MessageBoxW(hWnd, _lpcstr_tmp.c_str(), L"StreamViewerAI2.exe", MB_ICONWARNING | MB_OK);
				}
				//cvw_set_stop = true; //�X�g�b�v�͂��Ȃ�
				_cam_on = false;
				break;//���̃J������
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//�J�����̃I�[�v������
//�J�������I�[�v��������ADrawCycle���������`��T�C�N��������������B
//DrawCycle����߂��Ă����玟�̃J�������I�[�v������B
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int COUNT_VIDEOCAPTURE=0;
int DrawCV2Window(
	int _DrawCycleMode,
	HWND hWnd,		//Window�n���h��
	HDC hDC,
	DrawArea _draw_area,
	bool _keep_aspect,
	YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose,
	std::vector<std::vector<std::string>> _urls
	//std::ofstream* _pAICSV
)
{
	bool _cam_on = false;
	//bool _cycle = true;
	bool _cycle = true;
	bool _usbcam = false;
	CVW_STOP = 0;

	std::string _ai_out_put_string;
	std::string _ai_file_name = "";
	int _current_url_index = 0;

	//	��������J�����؂�ւ��T�C�N��
	while (_cycle) //
	{
		// hDC�̃X�R�[�v�͈͂����肷�邽�߃u���b�N�� 
		// �u���b�N�����O���ƁA��O���������A������Ȃǂ����ꍇ�A�n���h���s���ɂȂ�A�A�v���P�[�V������������������\��������B
		{
			cv::VideoCapture* _pt_capture;
			_pt_capture = new cv::VideoCapture;
			//LOGMSG2("Current_url_index", Current_url_index);
			if (fix_cam_number != -1)
			{
				//�J�����ԍ��̎w�肪����΂�����w�肷��B�X�p�Q�b�e�B���Ȃ��B
				_current_url_index = fix_cam_number;
			}
			int _ret = CameraOpenCycle(
				_DrawCycleMode,
				hWnd,		//Window�n���h��
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
				COUNT_VIDEOCAPTURE++;
			//��ʂ������h��Ԃ�
			if (_keep_aspect && fix_cam_number == -1)
			{
				HDC _hDC = GetDC(hWnd);
				AllPaintBlack(hWnd, _hDC, _draw_area);
				ReleaseDC(hWnd, _hDC);
			}
			//�b��[�u�ł����ɂ����
			//AI��CSV�o�̓t�@�C���̍X�V�����B�����ς��ƃt�@�C����ς���B
			open_ai_csv_file();
			////////////////////////////////////////////////////////////////
			//�`��T�C�N��
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
				//_pAICSV,
				DISPLAY_TIME_SECOND,
				FLAG_TIMESTUMP
			);

			//_capture.release();
			_pt_capture->release();

			delete _pt_capture;
			_pt_capture = nullptr;
			if (_ret)
				COUNT_VIDEOCAPTURE--;
			LOGMSG2("COUNT_VIDEOCAPTURE", COUNT_VIDEOCAPTURE);
		}
		_cam_on = false;

		//�J�����؂�ւ����[�v���o��
		if (cvw_set_stop == true)
			break;

		_current_url_index = (_current_url_index + 1) % (int)_urls.size();
	}
	//�������X�g�b�v�������Ƃ����C���ɓ`����
	CVW_STOP = true;

	//ReleaseDC(hWnd, hDC);

	mlVIDEOWRITERORG.release();
	mlVIDEOWRITERAI.release();

	//�}���`�X���b�h�Ȃ̂Ŗ߂�l�͖��������
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int DrawCV2Windowf(
	int _DrawCycleMode,
	HWND hWnd,		//Window�n���h��
	HDC hDC,
	DrawArea _draw_area,
	bool _keep_aspect,
	//	void* _pt_ai,	//yolov5�����N���X�̃C���X�^���X�ւ̃|�C���^ null���Ɩ���
	YoloObjectDetection* _pt_YoloObjectDetection,
	PoseNet* _pt_pose,
	std::string _file_name
	//std::ofstream* _pAICSV
)
{
	bool _cam_on = false;
	CVW_STOP = 0;
	cv::VideoCapture _capture;

	std::string _ai_out_put_string;

	//Window�o�[�Ƀt�@�C������\��
	std::string ai_file_name = "";
	if (_DrawCycleMode == DRAWCYCLE_YOLO5 && _pt_YoloObjectDetection!=nullptr)
		ai_file_name = _pt_YoloObjectDetection->YP.onnx_file_name;
	if (_DrawCycleMode == DRAWCYCLE_YOLO8 && _pt_YoloObjectDetection != nullptr)
		ai_file_name = _pt_YoloObjectDetection->YP.onnx_file_name;
	else if(_DrawCycleMode == DRAWCYCLE_POSENET && _pt_pose!=nullptr)
		ai_file_name = _pt_pose->get_weightsFile();

	int current_url_index = 0;

	//�E�B���h�E�L���v�V�����̕�����
	std::ostringstream window_caption;
	window_caption << _file_name << " " << ai_file_name;

	std::wstring newTitleW = string2wstring(window_caption.str().c_str());
	SetWindowTextW(hWnd, newTitleW.c_str());

	_capture.open(_file_name);

	//��ʂ������h��Ԃ�
	AllPaintBlack(hWnd, hDC, _draw_area);
	//�e�L�X�g���N���A
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

	CVW_FILE_END = false;

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
		//_pAICSV,
		0,
		0 //TIMESTUMP�Ȃ�
	);

	_capture.release();

	//�������I��������Ƃ�winproc�ɓ`���邽�߂̃t���O
	CVW_FILE_PROCESSING = false;
	CVW_FILE_END = true;

	//������v�邩?
	CVW_STOP = true;

	//ReleaseDC(hWnd, hDC);
	mlVIDEOWRITERORG.release();
	mlVIDEOWRITERAI.release();
	return 0;
}

