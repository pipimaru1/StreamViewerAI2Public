// StreamVierwer.cpp : アプリケーションのエントリ ポイントを定義します。
//
#include "stdafx.h"
#include "framework.h"
#include "mylib.h"
#include "yolov5_engine.h"
#include "pose.h"
#include "StreamVierwer.h"

#include <atlbase.h>
#include <atlconv.h>

#define MAX_LOADSTRING 100

#define _AI_NAMES "default.names"
#define _AI_ONNX "default.onnx"
#define _AI_CAMS "default.txt"
#define _AI_NAMES8 "coco.names"
#define _AI_ONNX8 "yolov8m.onnx"
//#define _SCORE_THRESHOLD 0.10
//#define _NMS_THRESHOLD 0.40
//#define _CONFIDENCE_THRESHOLD 0.10
#define _AI_WIDTH 0 //自動
#define _AI_HEIGHT 0 //自動

static std::string url_file(""); // ストリームURLのリストが記載されたファイル名
static std::string onnx_file(""); // ストリームURLのリストが記載されたファイル名
static std::string names_file(""); // ストリームURLのリストが記載されたファイル名
static std::string onnx_file8(""); // ストリームURLのリストが記載されたファイル名
static std::string names_file8(""); // ストリームURLのリストが記載されたファイル名
static std::string video_file_path = "";

std::string poseweight;
std::string poseproto;

//static LPWSTR url_file; // ストリームURLのリストが記載されたファイル名
//static LPWSTR onnx_file; // ストリームURLのリストが記載されたファイル名
//static LPWSTR names_file; // ストリームURLのリストが記載されたファイル名

// グローバル変数:
HINSTANCE hInst;                    // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];      // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];// メイン ウィンドウ クラス名
//HWND hHWND_TEXT;                         //テキストボックス
//HINSTANCE hINSTANCE;                //テキストボックスのインスタンス

//グローバル変数
//extern std::vector<std::string> urls;
#define VIEWMAX 64 
std::thread* main_th[VIEWMAX] = { 
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr
};
//volatile bool WM_PAINT_NOW = false;
std::atomic<bool> WM_PAINT_NOW = false;

std::vector<std::vector<std::string>> cam_urls; //[0]にはタイトル、[1]にはurlが入っている

//AI
YoloObjectDetection* ptYoloOBJECTDECTECTION = nullptr;		//サムネイル表示の時のAIクラス 複数のカメラでAIを共有、管理クラスからポインタをコピー
int _ai_running = 0;

//volatile bool bSUPPRESS_PAINT = FALSE;
std::atomic<bool> bSUPPRESS_PAINT = FALSE;
//int POSE_RUNNING = 0;

HWND hEdit;
HBRUSH hbrBlackBrush;
PoseNet* ptPOSENET;

//HOMEDRIVE = C:
//HOMEPATH = \Users\km47381
std::string MP4VHA   = "video_org";     //録画する時使用するファイル名
std::string MP4VHAAI = "video_ai";      //録画する時使用するファイル名
std::string MP4HD    = "video_hd_org";  //録画する時使用するファイル名
std::string MP4HDAI  = "video_hd_ai";   //録画する時使用するファイル名
std::string MP4PATH  = "%HOMEDRIVE%%HOMEPATH%\\";     //録画する時使用するファイル名
std::string MP4EXT   = ".mp4";                      //録画する時使用するファイル名

//#define INIFILE "default.ini"
std::string init_file = "default.ini";
#define APPMODE_NETCAM 0
#define APPMODE_USBCAM 1
#define APPMODE_MOVFILE 2

//volatile int appmode = APPMODE_NETCAM;
std::atomic<int> appmode = APPMODE_NETCAM;
int DrawCycleMode = DRAWCYCLE_YOLO5;

#define RWFILE_WRITE 1
#define RWFILE_READ 0
int read_write_file(std::string _inifile_path, int rw);

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE _hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//bool Mode4View = false;
int ViewMode = IDM_VIEW_1;
int WINDOW_MAX = 1;
int GPU_Number = 0;

int AISQL_IMAGE_WIDTH   = 640;
int AISQL_IMAGE_HEIGHT  = 480;
int AISQL_IMAGE_QALITY = 90;
std::wstring AISQL_IMAGE_FORMAT = L"JPG"; //JPG or PNG


//設定ファイルの読み書き 
int read_write_file(std::string _inifile_path, int rw)
{
    if (rw == RWFILE_WRITE)
    {
        std::ofstream inifile;
        //inifile.open(INIFILE, std::ios::out);
        inifile.open(_inifile_path, std::ios::out);

        inifile << "[BASIC SETTINGS]     " << std::endl;
        //DrawCycleMode
        if (DrawCycleMode == DRAWCYCLE_POSENET)     inifile << "AI_MODE =              " << "POSE_NET" << std::endl;
        else if(DrawCycleMode==DRAWCYCLE_YOLO5)     inifile << "AI_MODE =              " << "OBJECT_DETECTION" << std::endl;
        else if (DrawCycleMode == DRAWCYCLE_YOLO8)  inifile << "AI_MODE =              " << "OBJECT_DETECTION_YOLOV8" << std::endl;
        else                                        inifile << "AI_MODE =              " << "STREAM" << std::endl;

        //ViewMode  
        if (ViewMode == IDM_VIEW_4)                 inifile << "VIEW_MODE =            " << "4" << std::endl;
        else if (ViewMode == IDM_VIEW_6)            inifile << "VIEW_MODE =            " << "6" << std::endl;
        else if (ViewMode == IDM_VIEW_9)            inifile << "VIEW_MODE =            " << "9" << std::endl;
        else if (ViewMode == IDM_VIEW_12)           inifile << "VIEW_MODE =            " << "12" << std::endl;
        else if (ViewMode == IDM_VIEW_16)           inifile << "VIEW_MODE =            " << "16" << std::endl;
        else if (ViewMode == IDM_VIEW_36)           inifile << "VIEW_MODE =            " << "36" << std::endl;
        else if (ViewMode == IDM_VIEW_64)           inifile << "VIEW_MODE =            " << "64" << std::endl;
        else                                        inifile << "VIEW_MODE =            " << "1" << std::endl;

        inifile << "[LOADFILE SETTINGS]     " << std::endl;
        inifile << "STREAM_LIST_FILE =     " << url_file << std::endl;
        inifile << "ONNX_FILE =            " << onnx_file << std::endl;
        inifile << "NAMES_FILE =           " << names_file << std::endl;
        inifile << "ONNX_FILE_YOLOV8 =     " << onnx_file8 << std::endl;
        inifile << "NAMES_FILE_YOLOV8 =    " << names_file8 << std::endl;
        inifile << "DISPLAY_TIME[SECOND] = " << DISPLAY_TIME_SECOND << std::endl;
        //inifile << "POSE_WEIGHT_FILE =     \"" << poseweight << "\"" << std::endl;
        //inifile << "POSE_PROTO_FILE =      \"" << poseproto << "\"" << std::endl;
        inifile << "POSE_WEIGHT_FILE =     " << poseweight << std::endl;
        inifile << "POSE_PROTO_FILE =      " << poseproto << std::endl;
        inifile << "SLEEP_FRAME =          " << FRAME_INTERVAL_MS << std::endl;
        inifile << "MOVIE_FILE =           " << video_file_path << std::endl;
 
        inifile << "[RECORD SETTINGS]     " << std::endl;
        inifile << "MP4VHA =               " << MP4VHA << std::endl;    //録画する時使用するファイル名
        inifile << "MP4VHAAI =             " << MP4VHAAI << std::endl;  //録画する時使用するファイル名
        inifile << "MP4HD =                " << MP4HD << std::endl;     //録画する時使用するファイル名
        inifile << "MP4HDAI =              " << MP4HDAI << std::endl;   //録画する時使用するファイル名
        inifile << "MP4PATH =              " << MP4PATH << std::endl;   //録画する時使用するファイル名
        inifile << "WINDOW_MAX =           " << WINDOW_MAX << std::endl;

        inifile << "[YOLO SETTINGS]     " << std::endl;
        inifile << "YOLO_SCORE_THRESHOLD = " << DEFAULT_SCORE_THRESHOLD << std::endl;
        inifile << "YOLO_NMS_THRESHOLD =   " << DEFAULT_NMS_THRESHOLD << std::endl;
        inifile << "YOLO_CONF_THRESHOLD =  " << DEFAULT_CONF_THRESHOLD << std::endl;
        inifile << "GPU_DEVICE_NUMBER =    " << GPU_Number << std::endl;

        inifile << "[OTHER SETTINGS]     " << std::endl;
        inifile << "CAPTURE_TIMEOUT =      " << CAPOPEN_TIMEOUT << std::endl;

        inifile << "[AI DATA SAVE TO CSV]  " << std::endl;
        inifile << "AICSV_WRITE =          " << (int)AI_DATA_CSV_WRITE << std::endl;
        inifile << "AICSV_OVERWRITE =      " << (int)AI_DATA_CSV_OVER_WRITE << std::endl;
        inifile << "AICSV_PATH =           " << AICSVPATH << std::endl;   //AIの認識データを出力CSVファイルを置くパス

        inifile << "[SQL INFOMATIONS]      " << std::endl;
        inifile << "AISQL_SERVER =         " << wstring2string(SqlServerAi.server_name) << std::endl;
        inifile << "AISQL_DBNAME =         " << wstring2string(SqlServerAi.db_name) << std::endl;
        inifile << "AISQL_UID =            " << wstring2string(SqlServerAi.uid) << std::endl;
        inifile << "AISQL_PWD =            " << wstring2string(SqlServerAi.pwd) << std::endl;

        inifile << "[AI DATA SAVE TO SQL]  " << std::endl;
        inifile << "AISQL_WRITE =          " << (int)SQL_WRITE << std::endl;
        inifile << "AISQL_TABLE =          " << wstring2string(SqlServerAi.table) << std::endl;

        inifile << "[IMAGE SAVE TO SQL]    " << std::endl;
        inifile << "AISQL_IMAGEWRITE =     " << (int)SQL_IMAGEWRITE << std::endl;
        inifile << "AISQL_IMAGETABLE =     " << wstring2string(SqlServerImage.table) << std::endl;
        inifile << "AISQL_IMAGEINDEXFLR =  " << wstring2string(SqlServerImage.image_index_folder) << std::endl;

        inifile << "AISQL_IMAGE_WIDTH =    " << AISQL_IMAGE_WIDTH << std::endl;
        inifile << "AISQL_IMAGE_HEIGHT =   " << AISQL_IMAGE_HEIGHT << std::endl;
        inifile << "AISQL_IMAGE_QALITY =   " << AISQL_IMAGE_QALITY << std::endl;
        inifile << "AISQL_IMAGE_FORMAT =   " << wstring2string(AISQL_IMAGE_FORMAT) << std::endl;

        inifile.close();
    }
    else //設定ファイル読み取り
    {
        std::ifstream inifile;
        char buf[2048];
        inifile.open(_inifile_path, std::ios::in);
        if (inifile.is_open())
        {
            while (inifile.getline(buf, 2048))
            {
                std::string paramator_name;
                std::string dummy;
                std::string paramator_value;

                std::istringstream iline(buf);
                iline >> paramator_name;
                iline >> dummy;
                iline >> paramator_value;

                if (paramator_name == "AI_MODE")
                {
                    if (paramator_value == "POSE_NET")              DrawCycleMode = DRAWCYCLE_POSENET;
                    else if (paramator_value == "POSE")             DrawCycleMode = DRAWCYCLE_POSENET;
                    else if (paramator_value == "OBJECT_DETECTION") DrawCycleMode = DRAWCYCLE_YOLO5;
                    else if (paramator_value == "YOLOV")            DrawCycleMode = DRAWCYCLE_YOLO5;
                    else if (paramator_value == "YOLOV5")           DrawCycleMode = DRAWCYCLE_YOLO5;
                    else if (paramator_value == "OBJECT_DETECTION_YOLOV8")  
                                                                    DrawCycleMode = DRAWCYCLE_YOLO8;
                    else if (paramator_value == "YOLOV8")           DrawCycleMode = DRAWCYCLE_YOLO8;
                    else if (paramator_value == "STREAM")           DrawCycleMode = DRAWCYCLE_STREAM;
                    else                                            DrawCycleMode = DRAWCYCLE_STREAM;
                }
                else if (paramator_name == "VIEW_MODE")
                {
                    if (paramator_value == "4")         ViewMode = IDM_VIEW_4;
                    else if (paramator_value == "6")    ViewMode = IDM_VIEW_6;
                    else if (paramator_value == "9")    ViewMode = IDM_VIEW_9;
                    else if (paramator_value == "12")   ViewMode = IDM_VIEW_12;
                    else if (paramator_value == "16")   ViewMode = IDM_VIEW_16;
                    else if (paramator_value == "36")   ViewMode = IDM_VIEW_36;
                    else if (paramator_value == "64")   ViewMode = IDM_VIEW_64;
                    else                                ViewMode = IDM_VIEW_1;
                }
                else if (paramator_name == "WINDOW_MAX")
                {
                    if      (paramator_value == "1")    WINDOW_MAX = 1; //最大化
                    else if (paramator_value == "2")    WINDOW_MAX = 2; //メニューバーを残して最大化
                    else if (paramator_value == "3")    WINDOW_MAX = 3; //マルチディスプレイの1に最大化
                    else if (paramator_value == "4")    WINDOW_MAX = 4; //マルチディスプレイの2に最大化
                    else if (paramator_value == "5")    WINDOW_MAX = 5; //マルチディスプレイの3に最大化
                    else if (paramator_value == "6")    WINDOW_MAX = 6; //マルチディスプレイの4に最大化
                    else                                WINDOW_MAX = 0; //最大化しない
                }
                else if (paramator_name == "STREAM_LIST_FILE")      url_file = paramator_value;
                else if (paramator_name == "ONNX_FILE")             onnx_file = paramator_value;
                else if (paramator_name == "NAMES_FILE")            names_file = paramator_value;
                else if (paramator_name == "ONNX_FILE_YOLOV8")      onnx_file8 = paramator_value;
                else if (paramator_name == "NAMES_FILE_YOLOV8")     names_file8 = paramator_value;
                else if (paramator_name == "DISPLAY_TIME[SECOND]")  DISPLAY_TIME_SECOND = std::stoi(paramator_value);
                else if (paramator_name == "POSE_WEIGHT_FILE")      poseweight = paramator_value;
                else if (paramator_name == "POSE_PROTO_FILE")       poseproto = paramator_value;
                else if (paramator_name == "SLEEP_FRAME")           FRAME_INTERVAL_MS = std::stoi(paramator_value);
                else if (paramator_name == "MOVIE_FIEE")            video_file_path = paramator_value;
                else if (paramator_name == "MP4VHA")                MP4VHA = paramator_value;
                else if (paramator_name == "MP4VHAAI")              MP4VHAAI = paramator_value;
                else if (paramator_name == "MP4HD")                 MP4HD = paramator_value;
                else if (paramator_name == "MP4HDAI")               MP4HDAI = paramator_value;
                else if (paramator_name == "MP4PATH")               MP4PATH = paramator_value;
                else if (paramator_name == "CAPTURE_TIMEOUT")       CAPOPEN_TIMEOUT = std::stoi(paramator_value);
                else if (paramator_name == "YOLO_SCORE_THRESHOLD")  DEFAULT_SCORE_THRESHOLD = std::stof(paramator_value);
                else if (paramator_name == "YOLO_NMS_THRESHOLD")    DEFAULT_NMS_THRESHOLD = std::stof(paramator_value);
                else if (paramator_name == "YOLO_CONF_THRESHOLD")   DEFAULT_CONF_THRESHOLD = std::stof(paramator_value);
                else if (paramator_name == "AICSV_PATH")             AICSVPATH = paramator_value;
                else if (paramator_name == "AICSV_WRITE")           AI_DATA_CSV_WRITE = (bool)std::stoi(paramator_value);
                else if (paramator_name == "AICSV_OVERWRITE")       AI_DATA_CSV_OVER_WRITE = (bool)std::stoi(paramator_value);
                //SQLサーバーの設定 テーブル以外は同じ
                else if (paramator_name == "AISQL_WRITE")           SQL_WRITE = (bool)std::stoi(paramator_value);
                else if (paramator_name == "AISQL_SERVER")          SqlServerImage.server_name  = SqlServerAi.server_name = string2wstring(paramator_value);
                else if (paramator_name == "AISQL_DBNAME")          SqlServerImage.db_name      = SqlServerAi.db_name     = string2wstring(paramator_value);
                else if (paramator_name == "AISQL_UID")             SqlServerImage.uid          = SqlServerAi.uid         = string2wstring(paramator_value);
                else if (paramator_name == "AISQL_PWD")             SqlServerImage.pwd          = SqlServerAi.pwd         = string2wstring(paramator_value);
                else if (paramator_name == "AISQL_TABLE")                                           SqlServerAi.table     = string2wstring(paramator_value);
                else if (paramator_name == "AISQL_IMAGEWRITE")      SQL_IMAGEWRITE = (bool)std::stoi(paramator_value);
                else if (paramator_name == "AISQL_IMAGETABLE")      SqlServerImage.table                                  = string2wstring(paramator_value);
                else if (paramator_name == "AISQL_IMAGEINDEXFLR")   SqlServerImage.image_index_folder                     = string2wstring(paramator_value);

                else if (paramator_name == "AISQL_IMAGE_WIDTH")     AISQL_IMAGE_WIDTH = std::stoi(paramator_value);
                else if (paramator_name == "AISQL_IMAGE_HEIGHT")    AISQL_IMAGE_HEIGHT = std::stoi(paramator_value);
                else if (paramator_name == "AISQL_IMAGE_QALITY")    AISQL_IMAGE_QALITY = std::stoi(paramator_value);
                else if (paramator_name == "AISQL_IMAGE_FORMAT")    AISQL_IMAGE_FORMAT = string2wstring(paramator_value);

                else if (paramator_name == "GPU_DEVICE_NUMBER")     GPU_Number = std::stoi(paramator_value);
            }
        }
    }
    return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE _hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // TODO: ここにコードを挿入してください。
 
    // コマンドライン引数を取得
    LPWSTR _lpCmdLine = GetCommandLineW();
    // コマンドライン引数を個々の引数に分解
    int nArgs = 0;
    LPWSTR* szArglist = CommandLineToArgvW(_lpCmdLine, &nArgs);
    // エラー処理
    if (szArglist == NULL) {
        std::wcerr << L"CommandLineToArgvW failed" << std::endl;
        return 1;
    }
    //コード埋め込み初期値の設定
    url_file = _AI_CAMS;
    onnx_file = _AI_ONNX;
    names_file = _AI_NAMES;
    onnx_file8 = _AI_ONNX8;
    names_file8 = _AI_NAMES8;

    ///////////////////////////////////////////////////////////////////////////////
    //設定ファイルのコマンドライン引数だけ先にゲット
    int i = 0;
    for (i = 0; i < nArgs; i++)
    {
        if (wcscmp(szArglist[i], L"-i") == 0
            || wcscmp(szArglist[i], L"-ini") == 0)
        {
            std::filesystem::path _fp(szArglist[i + 1]);
            init_file = _fp.string();
            i++;
        }
    }

    //前回設定の読み込み
    read_write_file(init_file, RWFILE_READ);

    i = 0;
    for (i = 0; i < nArgs; i++)
    {
        //int msgboxID2 = MessageBox(NULL, szArglist[i], L"test", MB_ICONWARNING | MB_OK);
        ///////////////////////////////////////////////////////////////////////////////
        //カメラ -L -F -C
        if (   wcscmp(szArglist[i], L"-l")==0
            || wcscmp(szArglist[i], L"-list") == 0)
        {
            std::filesystem::path _fp(szArglist[i+1]);
            url_file = _fp.string();
            i++;
        }
        else if (wcscmp(szArglist[i], L"-f") == 0
            || wcscmp(szArglist[i], L"-frame") == 0)
        {
            FRAME_INTERVAL_MS = _wtoi(szArglist[i + 1]);
            i++;
        }
        else if (wcscmp(szArglist[i], L"-c") == 0
            || wcscmp(szArglist[i], L"-change") == 0)
        {
            DISPLAY_TIME_SECOND = _wtoi(szArglist[i + 1]);
            i++;
        }
        ///////////////////////////////////////////////////////////////////////////////
        //YOLO -X -N
        else if (   wcscmp(szArglist[i], L"-x") == 0
            || wcscmp(szArglist[i], L"-onnx") == 0)
        {
            std::filesystem::path _fp(szArglist[i + 1]);
            onnx_file = _fp.string();
            i++;
        }
        else if (   wcscmp(szArglist[i], L"-n") == 0
            || wcscmp(szArglist[i], L"-names") == 0)
        {
            std::filesystem::path _fp(szArglist[i + 1]);
            names_file = _fp.string();
            i++;
        }
        ///////////////////////////////////////////////////////////////////////////////
        //POSE NET -PW -PP
        else if (wcscmp(szArglist[i], L"-pw") == 0
            || wcscmp(szArglist[i], L"-PW") == 0
            || wcscmp(szArglist[i], L"-poseweight") == 0
            || wcscmp(szArglist[i], L"-POSEWEIGHT") == 0)
            {
            std::filesystem::path _fp(szArglist[i + 1]);
            poseweight = _fp.string();

            if (!std::filesystem::exists(poseweight))
            {
                CA2W wStr(poseweight.c_str());
                int msgboxID = MessageBox(NULL, wStr, (LPCWSTR)L"NAMESファイルがありません", MB_ICONWARNING | MB_OK);
                return 0;
            }
            i++;
        }
        else if (wcscmp(szArglist[i], L"-pp") == 0
            || wcscmp(szArglist[i], L"-PP") == 0
            || wcscmp(szArglist[i], L"-poseproto") == 0
            || wcscmp(szArglist[i], L"-POSEPROTO") == 0)
        {
            std::filesystem::path _fp(szArglist[i + 1]);
            std::string _str = _fp.string();
            //_PoseNet.set_protoFile(_str);
            poseproto = _fp.string();
            //_PoseNet.get_weightsFile() = _fp.string();
            //_PoseNet.weightsFile = _fp.string();
            if (!std::filesystem::exists(poseproto))
            {
                CA2W wStr(poseproto.c_str());
                int msgboxID = MessageBox(NULL, wStr, (LPCWSTR)L"NAMESファイルがありません", MB_ICONWARNING | MB_OK);
                return 0;
            }
            i++;
        }
        ///////////////////////////////////////////////////////////////////////////////
        //モード -S -M -P -Y
        else if (wcscmp(szArglist[i], L"-m") == 0
              || wcscmp(szArglist[i], L"-movie") == 0)
        {
            appmode = APPMODE_MOVFILE;
            int msgboxID = MessageBox(NULL, L"動画ファイル読み込みモードで起動します。\n起動後、ファイルメニューから動画ファイルを指定してください。", L"StreamViewerAI2.exe", MB_ICONWARNING | MB_OK);
        }
        else if (wcscmp(szArglist[i], L"-s") == 0
            || wcscmp(szArglist[i], L"-stream") == 0)
        {
            DrawCycleMode = DRAWCYCLE_STREAM;
            i++;
        }
        else if (wcscmp(szArglist[i], L"-p") == 0
            || wcscmp(szArglist[i], L"-pose") == 0)
        {
            DrawCycleMode = DRAWCYCLE_POSENET;
            i++;
        }
        else if (wcscmp(szArglist[i], L"-y") == 0
            || wcscmp(szArglist[i], L"-yolo") == 0)
        {
            DrawCycleMode = DRAWCYCLE_YOLO5;
            i++;
        }
        ///////////////////////////////////////////////////////////////////////////////
        //モード -4
        else if (wcscmp(szArglist[i], L"-1") == 0 || wcscmp(szArglist[i], L"-1view") == 0 || wcscmp(szArglist[i], L"-1VIEW") == 0) { ViewMode = IDM_VIEW_1; }
        else if (wcscmp(szArglist[i], L"-4") == 0 || wcscmp(szArglist[i], L"-4view") == 0 || wcscmp(szArglist[i], L"-4VIEW") == 0) { ViewMode = IDM_VIEW_4; }
        else if (wcscmp(szArglist[i], L"-6") == 0 || wcscmp(szArglist[i], L"-6view") == 0 || wcscmp(szArglist[i], L"-6VIEW") == 0) { ViewMode = IDM_VIEW_6; }
        else if (wcscmp(szArglist[i], L"-9") == 0 || wcscmp(szArglist[i], L"-9view") == 0 || wcscmp(szArglist[i], L"-9VIEW") == 0) { ViewMode = IDM_VIEW_9; }
        else if (wcscmp(szArglist[i], L"-12") == 0 || wcscmp(szArglist[i], L"-12view") == 0 || wcscmp(szArglist[i], L"-12VIEW") == 0) { ViewMode = IDM_VIEW_12; }
        else if (wcscmp(szArglist[i], L"-16") == 0 || wcscmp(szArglist[i], L"-16view") == 0 || wcscmp(szArglist[i], L"-16VIEW") == 0) { ViewMode = IDM_VIEW_16; }
        else if (wcscmp(szArglist[i], L"-36") == 0 || wcscmp(szArglist[i], L"-36view") == 0 || wcscmp(szArglist[i], L"-36VIEW") == 0) { ViewMode = IDM_VIEW_36; }
        else if (wcscmp(szArglist[i], L"-64") == 0 || wcscmp(szArglist[i], L"-64view") == 0 || wcscmp(szArglist[i], L"-64VIEW") == 0) { ViewMode = IDM_VIEW_64; }

        ///////////////////////////////////////////////////////////////////////////////
        else if (wcscmp(szArglist[i], L"-h") == 0
            || wcscmp(szArglist[i], L"-help") == 0)
        {
            std::ostringstream _str_usage("");
            _str_usage <<"引数が１つも無い場合はデフォルト引数で起動します。デフォルト値は下記です。" << std::endl
                << "カメラリスト:  " << _AI_CAMS << std::endl
                << "onnxファイル:  " << _AI_ONNX << std::endl
                << "nmaesファイル: " << _AI_NAMES << std::endl
                << "=======================================" << std::endl
                << "引数を指定する場合" << std::endl
                << "EX: StreamViewerAI2.exe -l cams.txt -x yolov5s.onnx -n coco.names" << std::endl
                << "=======================================" << std::endl
                << "オプション" << std::endl
                << "-i, -ini 設定ファイル" << std::endl
                << "-s,-stream AI処理なし" << std::endl
                << "-y,-yolo 物体認識(YOLO)" << std::endl
                << "-p,-pose 姿勢認識(PoseNet)'" << std::endl
                << "-l,-list 'カメラリストファイル'" << std::endl
                << "-x,-onnx 'onnxファイル'" << std::endl
                << "-n,-names 'namesファイル'" << std::endl
                << "-pw,-poseweight 'PoseNet weight ファイル'" << std::endl
                << "-pp,-poseproto 'Posenet protoファイル'" << std::endl
                << "-c,-change 'カメラ切り替え時間時間[sec]'" << std::endl
                << "-f,-frame 'キャプチャの間隔時間[ms/f]'" << std::endl
                << "-m,-movie 動画ファイル読み込みモード カメラリストファイルは無視" << std::endl
                << "=======================================" << std::endl
                << "-1,-1view,-1VIEW 1画面(デフォルト)" << std::endl
                << "-4,-4view,-4VIEW 4画面" << std::endl
                << "=======================================" << std::endl
                << "カメラリストファイル書き方" << std::endl
                << "カメラ名称,urlアドレス" << std::endl
                << "例" << std::endl
                << "自宅玄関,http://192.168.1.1/cgi-bin/mjpeg?resolution=1920x1080" << std::endl
                << "猫カメラ,http://ID:PW@192.168.1.2/cgi-bin/mjpeg" << std::endl;
            std::string _tmp = _str_usage.str();
            CA2W wStr(_tmp.c_str());
            MessageBox(NULL, wStr, L"StreamViewerAI2.exe", MB_ICONWARNING | MB_OK);
           // MessageBox(NULL,  _str_usage.str(), L"StreamViewerAI2.exe", MB_ICONWARNING | MB_OK);
            return 0;
        }
    }
    if(0)
    {    std::ostringstream _str_usage("");
        _str_usage << url_file << std::endl
            << onnx_file << std::endl
            << names_file << std::endl
            <<"sleep[msec]: " << FRAME_INTERVAL_MS << std::endl
            <<"flamerate[fps]: "<< DISPLAY_TIME_SECOND << std::endl;
        std::string _tmp = _str_usage.str();
        CA2W wStr(_tmp.c_str());
        int msgboxID = MessageBox(NULL, wStr, L"cxxopts", MB_ICONWARNING | MB_OK);
    }

    //fileのチェック
    if (!std::filesystem::exists(url_file))
    {
        //CA2W wStr(url_file.c_str());
        //int msgboxID = MessageBox(NULL, wStr,(LPCWSTR)L"設定ファイルがありません",MB_ICONWARNING | MB_OK );
        appmode = APPMODE_MOVFILE;
        int msgboxID = MessageBox(NULL, L"動画ファイル読み込みモードで起動します。\n起動後、ファイルメニューから動画ファイルを指定してください。", L"StreamViewerAI2.exe", MB_ICONWARNING | MB_OK);
        //return 0;
    }
    if (!std::filesystem::exists(onnx_file))
    {
        std::string _msgstr = "ONNXファイルがありません。\n物体認識AIの動作に必要なファイルです。AIを停止して起動します。\nメニューからonnxファイルを指定してから、AIメニューでAIを起動してください。\n\n見つからないファイル\n" + onnx_file;
        CA2W wStr(_msgstr.c_str());
        int msgboxID = MessageBox(NULL, wStr, L"ファイルが見つかりません",MB_ICONWARNING | MB_OK);
        //CA2W wStr(onnx_file.c_str());
        //int msgboxID = MessageBox(NULL, wStr, (LPCWSTR)L"ONNXファイルがありません。\nストーリミング受信モードで起動します。", MB_ICONWARNING | MB_OK);
        DrawCycleMode = DRAWCYCLE_STREAM;
        //return 0;
    }
    if (!std::filesystem::exists(names_file))
    {
        std::string _msgstr = "namesファイルがありません。\n物体認識AIの動作に必要なファイルです。AIを停止して起動します。\nメニューからonnxファイルを指定してから、AIメニューでAIを起動してください。\n\n見つからないファイル\n" + names_file;
        CA2W wStr(_msgstr.c_str());
        int msgboxID = MessageBox(NULL, wStr, L"ファイルが見つかりません", MB_ICONWARNING | MB_OK);

        //CA2W wStr(names_file.c_str());
        //int msgboxID = MessageBox(NULL, wStr, (LPCWSTR)L"NAMESファイルがありません。\nストーリミング受信モードで起動します。", MB_ICONWARNING | MB_OK);
        //return 0;
        DrawCycleMode = DRAWCYCLE_STREAM;
    }

    open_ai_csv_file();

    // グローバル文字列を初期化する
    LoadStringW(_hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(_hInstance, IDC_STREAMVIERWER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(_hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (_hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(_hInstance, MAKEINTRESOURCE(IDC_STREAMVIERWER));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    //デストラクタを確実に動作させるためのブロック
    {
        std::lock_guard<std::mutex> lock(FILE_MUTEX);
        if (pAICSV != nullptr)
        {
            pAICSV->close();
        }
    }
    return (int) msg.wParam;
}
//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE _hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = _hInstance;
    wcex.hIcon          = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_STREAMVIERWER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_STREAMVIERWER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE _hInstance, int nCmdShow)
{
   hInst = _hInstance; // グローバル変数にインスタンス ハンドルを格納する

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, _hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//ファイル選択ダイアログボックス
int select_file(HWND hWnd, std::string& _file_path, LPCWSTR _file_pattern)
{
    OPENFILENAME ofn;       // OPENFILENAME 構造体
    wchar_t szFile[260] = { 0 }; // ファイル名格納用のバッファ

    // OPENFILENAME 構造体の初期化
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;  // ダイアログボックスの親ウィンドウ
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _file_pattern; // 表示するファイルの種類
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // ファイル選択ダイアログボックスの表示
    if (GetOpenFileName(&ofn) == TRUE)
    {
        // ファイルが選択されたときの処理
        if (std::filesystem::exists(ofn.lpstrFile))
        {
            _file_path = wstring2string(ofn.lpstrFile);
            return 1;
        }
        else
        {
            int msgboxID = MessageBox(NULL, ofn.lpstrFile, (LPCWSTR)L"ファイルがありません", MB_ICONWARNING | MB_OK);
            return 0;
        }
    }
    return 0;
}

#define AI_LOAD 1
#define AI_UNLOAD 2
int load_YoloObjectDetection(int _load);
int load_PoseNet(int _load);

////////////////////////////////////////////////////////////////////////////////
//フラグをセットし、再描画イベントを発生させる
int PROC_START(HWND hWnd)
{
    if (DrawCycleMode ==  DRAWCYCLE_YOLO5)
        load_YoloObjectDetection(AI_LOAD);
    else if (DrawCycleMode == DRAWCYCLE_YOLO8)
        load_YoloObjectDetection(AI_LOAD);
    else if (DrawCycleMode == DRAWCYCLE_POSENET)
        load_PoseNet(AI_LOAD);
    else
        /*no action*/;

    set_cvw_stop(false);
    UpdateWindow(hWnd);
    ShowWindow(hWnd, SW_SHOW);
    InvalidateRect(hWnd, NULL, TRUE);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
int PROC_STOP()
{
    set_cvw_stop(true);
    
    //ビデオキャプチャークラスを閉じる
    while (COUNT_VIDEOCAPTURE > 0)
    {
        //DoEvents();
        Sleep(100);
    }
    //ビデオライタークラスを閉じる
    mlVIDEOWRITERORG.release();
    mlVIDEOWRITERAI.release();
    
    for (int i = 0; i < VIEWMAX; i++)
    {
        if (main_th[i] != nullptr)
        {
            svw_wait_stop();
            if(main_th[i]->joinable())
                main_th[i]->join();
            delete main_th[i];
            main_th[i] = nullptr;
        }
    }

    if (DrawCycleMode == DRAWCYCLE_YOLO5)
        load_YoloObjectDetection(AI_UNLOAD);
    else if (DrawCycleMode == DRAWCYCLE_YOLO8)
        load_YoloObjectDetection(AI_UNLOAD);
    else if (DrawCycleMode == DRAWCYCLE_POSENET)
        load_PoseNet(AI_UNLOAD);
    else
        /*no action*/;
  
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// ① newで新しいyoloオブジェクトを生成
// ② パラメータに値をセット
int load_YoloObjectDetection(int _load)
{
    if (_load == AI_LOAD)
    {
        ptYoloOBJECTDECTECTION = new YoloObjectDetection;
        YoloAIParametors yp;
        if (DrawCycleMode == DRAWCYCLE_YOLO8)
        {
            yp.yolo_version = YOLOV8;
            yp.onnx_file_name = onnx_file8.c_str();
            yp.names_file_name = names_file8.c_str();
        }
        else //YOLOV5
        {
            yp.yolo_version = YOLOV5;
            yp.onnx_file_name = onnx_file.c_str();
            yp.names_file_name = names_file.c_str();
        }

        yp.input_width = DEFAULT_AI_INPUT_WIDTH;
        yp.input_height = DEFAULT_AI_INPUT_HEIGHT;
        yp.score_threshold = (float)DEFAULT_SCORE_THRESHOLD;
        yp.nms_threshold = (float)DEFAULT_NMS_THRESHOLD;
        yp.confidence_thresgold = (float)DEFAULT_CONF_THRESHOLD;
        yp.GPU_Number = GPU_Number;
        _ai_running = ptYoloOBJECTDECTECTION->init_object_detection(yp, true, false);
    }
    else if (_load == AI_UNLOAD)
    {
        if (ptYoloOBJECTDECTECTION != nullptr)
        {
            delete ptYoloOBJECTDECTECTION;
            ptYoloOBJECTDECTECTION = nullptr;
        }
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
int load_PoseNet(int _load)
{
    if (_load == AI_LOAD)
    {
        ptPOSENET = new PoseNet;
        ptPOSENET->init(poseproto, poseweight, USE_GPU);
    }
    else if(_load==AI_UNLOAD)
    { 
        if (ptPOSENET != nullptr)
        {
            delete ptPOSENET;
            ptPOSENET = nullptr;
        }
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// _org_strと_add_strをくっつけてメニュー項目に適用
BOOL set_menu_string(HWND hWnd, UINT _idm, LPCSTR _org_str, std::string& _add_str)
{

    std::ostringstream _menu_wsstr;
    _menu_wsstr << _org_str
        << "("
        << _add_str.c_str()
        << ")" << std::ends;
    std::wstring _menu_str = string2wstring(_menu_wsstr.str());
   
    return ModifyMenu(GetMenu(hWnd), _idm, MF_STRING, _idm, (LPCWSTR)_menu_str.c_str());

}

////////////////////////////////////////////////////////////////////////////////
//メニューにチェックマークを付ける処理
#define DTSM(a) set_display_time_seconds_menuitems(a)
int set_display_time_seconds_menuitems(HWND hWnd)
{
    HMENU hMenu = GetMenu(hWnd);

    CheckMenuItem(hMenu, IDM_C_000, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_C_001, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_C_003, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_C_008, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_C_015, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_C_030, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_C_060, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_C_120, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_C_300, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_C_600, MF_UNCHECKED);

    if (get_display_time_seconds() ==  0)CheckMenuItem(hMenu, IDM_C_000, MF_CHECKED);
    if (get_display_time_seconds() ==  1)CheckMenuItem(hMenu, IDM_C_001, MF_CHECKED);
    if (get_display_time_seconds() ==  3)CheckMenuItem(hMenu, IDM_C_003, MF_CHECKED);
    if (get_display_time_seconds() ==  8)CheckMenuItem(hMenu, IDM_C_008, MF_CHECKED);
    if (get_display_time_seconds() == 15)CheckMenuItem(hMenu, IDM_C_015, MF_CHECKED);
    if (get_display_time_seconds() == 30)CheckMenuItem(hMenu, IDM_C_030, MF_CHECKED);
    if (get_display_time_seconds() == 60)CheckMenuItem(hMenu, IDM_C_060, MF_CHECKED);
    if (get_display_time_seconds() ==120)CheckMenuItem(hMenu, IDM_C_120, MF_CHECKED);
    if (get_display_time_seconds() ==300)CheckMenuItem(hMenu, IDM_C_300, MF_CHECKED);
    if (get_display_time_seconds() ==600)CheckMenuItem(hMenu, IDM_C_600, MF_CHECKED);

//    if (_IDM != 0)
//        CheckMenuItem(hMenu, _IDM, MF_CHECKED);

    DrawMenuBar(hWnd);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//メニューにチェックマークを付ける処理
#define FBTM(a) set_frame_between_time_menuitems(a)
int set_frame_between_time_menuitems(HWND hWnd)
{
    HMENU hMenu = GetMenu(hWnd);

    CheckMenuItem(hMenu, IDM_F_0003, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_F_0005, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_F_0008, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_F_0010, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_F_0020, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_F_0100, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_F_0200, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_F_0300, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_F_0600, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_F_1200, MF_UNCHECKED);

    if (get_frame_between_time() ==3000)CheckMenuItem(hMenu, IDM_F_0003, MF_CHECKED);
    if (get_frame_between_time() ==2000)CheckMenuItem(hMenu, IDM_F_0005, MF_CHECKED);
    if (get_frame_between_time() ==1500)CheckMenuItem(hMenu, IDM_F_0008, MF_CHECKED);
    if (get_frame_between_time() ==1000)CheckMenuItem(hMenu, IDM_F_0010, MF_CHECKED);
    if (get_frame_between_time() ==500 )CheckMenuItem(hMenu, IDM_F_0020, MF_CHECKED);
    if (get_frame_between_time() ==100 )CheckMenuItem(hMenu, IDM_F_0100, MF_CHECKED);
    if (get_frame_between_time() ==50  )CheckMenuItem(hMenu, IDM_F_0200, MF_CHECKED);
    if (get_frame_between_time() ==33  )CheckMenuItem(hMenu, IDM_F_0300, MF_CHECKED);
    if (get_frame_between_time() ==16  )CheckMenuItem(hMenu, IDM_F_0600, MF_CHECKED);
    if (get_frame_between_time() ==8   )CheckMenuItem(hMenu, IDM_F_1200, MF_CHECKED);

    DrawMenuBar(hWnd);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
//メニューにチェックマークを付ける処理
int STHM(HWND hWnd)
{
    HMENU hMenu = GetMenu(hWnd);

    CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_000, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_010, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_020, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_030, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_040, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_050, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_060, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_070, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_080, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_090, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_100, MF_UNCHECKED);

    if (ptYoloOBJECTDECTECTION != nullptr)
    {
        if (ptYoloOBJECTDECTECTION->YP.score_threshold == 0.001f)CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_000, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.score_threshold == 0.1f)CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_010, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.score_threshold == 0.2f)CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_020, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.score_threshold == 0.3f)CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_030, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.score_threshold == 0.4f)CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_040, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.score_threshold == 0.5f)CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_050, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.score_threshold == 0.6f)CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_060, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.score_threshold == 0.7f)CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_070, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.score_threshold == 0.8f)CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_080, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.score_threshold == 0.9f)CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_090, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.score_threshold == 1.0f)CheckMenuItem(hMenu, IDM_AI_SCORE_THRESHOLD_100, MF_CHECKED);
    }
    DrawMenuBar(hWnd);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
//メニューにチェックマークを付ける処理
int SNHM(HWND hWnd)
{
    HMENU hMenu = GetMenu(hWnd);

    CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_000, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_010, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_020, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_030, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_040, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_050, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_060, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_070, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_080, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_090, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_100, MF_UNCHECKED);

    if (ptYoloOBJECTDECTECTION != nullptr)
    {
        if (ptYoloOBJECTDECTECTION->YP.nms_threshold == 0.1f)CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_000, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.nms_threshold == 0.1f)CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_010, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.nms_threshold == 0.2f)CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_020, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.nms_threshold == 0.3f)CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_030, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.nms_threshold == 0.4f)CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_040, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.nms_threshold == 0.5f)CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_050, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.nms_threshold == 0.6f)CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_060, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.nms_threshold == 0.7f)CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_070, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.nms_threshold == 0.8f)CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_080, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.nms_threshold == 0.9f)CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_090, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.nms_threshold == 1.0f)CheckMenuItem(hMenu, IDM_AI_NMS_THRESHOLD_100, MF_CHECKED);
    }

    DrawMenuBar(hWnd);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
//メニューにチェックマークを付ける処理
int SCHM(HWND hWnd)
{
    HMENU hMenu = GetMenu(hWnd);

    CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_000, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_010, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_020, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_030, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_040, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_050, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_060, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_070, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_080, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_090, MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_100, MF_UNCHECKED);

    if (ptYoloOBJECTDECTECTION != nullptr)
    {
        if (ptYoloOBJECTDECTECTION->YP.confidence_thresgold == 0.001f)CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_000, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.confidence_thresgold == 0.1f  )CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_010, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.confidence_thresgold == 0.2f  )CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_020, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.confidence_thresgold == 0.3f  )CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_030, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.confidence_thresgold == 0.4f  )CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_040, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.confidence_thresgold == 0.5f  )CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_050, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.confidence_thresgold == 0.6f  )CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_060, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.confidence_thresgold == 0.7f  )CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_070, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.confidence_thresgold == 0.8f  )CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_080, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.confidence_thresgold == 0.9f  )CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_090, MF_CHECKED);
        if (ptYoloOBJECTDECTECTION->YP.confidence_thresgold == 1.0f  )CheckMenuItem(hMenu, IDM_AI_CONF_THRESHOLD_100, MF_CHECKED);
    }

    DrawMenuBar(hWnd);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
//メニューにファイル名を付ける処理
int set_file_menu_items(HWND hWnd)
{
    set_menu_string(hWnd, IDM_FILE_CAMS, "カメラリストファイルを選択(&L)", url_file);
    set_menu_string(hWnd, IDM_FILE_ONNX, "物体認識AIモデルファイル(onnx)を選択(&O)", onnx_file);
    set_menu_string(hWnd, IDM_FILE_NAMES, "物体認識AI名前ファイル(names)を選択(&N)",  names_file);
    set_menu_string(hWnd, IDM_FILE_ONNX8, "物体追跡AIモデルファイル(onnx)を選択(&O)", onnx_file8);
    set_menu_string(hWnd, IDM_FILE_NAMES8, "物体追跡AI名前ファイル(names)を選択(&N)", names_file8);
    set_menu_string(hWnd, IDM_FILE_POSEWEIGHT, "ポーズAIモデルファイル(caffemodel)を選択(&P)",  poseweight);
    set_menu_string(hWnd, IDM_FILE_POSEPROTO, "ポーズAI定義ファイル(prototxt)を選択(&R)",    poseproto);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//画面分割表示のためにカメラリストの配列を分割する
typedef std::vector<std::vector<std::string>> T_CAM_URLS; //文字列の 2次元配列
std::vector<T_CAM_URLS> splitIntoN(const T_CAM_URLS& _vec, int _n)
{
    std::vector<T_CAM_URLS> _result(_n);
    size_t length = _vec.size() / _n;
    size_t remain = _vec.size() % _n;

    size_t begin = 0;
    size_t end = 0;

    for (int i = 0; i < _n; ++i) {
        end = begin + length + (remain > 0 ? 1 : 0);
        if (remain > 0) remain--;

        _result[i] = T_CAM_URLS(_vec.begin() + begin, _vec.begin() + end);
        begin = end;
    }
    return _result;
}

////////////////////////////////////////////////////////////////////////////////
//カメラ選択メニューにカメラ名を追加する
//IDM_CAM_001
//IDM_CAM_POP_1
#define CAMERA_POS_IN_MAINMENU 2
int set_camera_menu(HWND hWnd,T_CAM_URLS& _cam_urls)
{
    HMENU _hMainMenubar = GetMenu(hWnd); // メインメニュー
    HMENU _hSubMenu = GetSubMenu(_hMainMenubar, CAMERA_POS_IN_MAINMENU); // メインメニューの2番目のサブメニュー　
    HMENU _hPupSubMenu = CreatePopupMenu();//ポップアップメニュー　初期化は仮

    std::wstring _sub_menu_title[10] = {
        L"カメラ　01-10(&0)",
        L"カメラ　11-20(&1)",
        L"カメラ　21-30(&2)",
        L"カメラ　31-40(&3)",
        L"カメラ　41-50(&4)",
        L"カメラ　51-60(&5)",
        L"カメラ　61-70(&6)",
        L"カメラ　71-80(&7)",
        L"カメラ　81-90(&8)",
        L"カメラ　91-100(&9)" };

    int _count=0;
    int _count_max =(int) _cam_urls.size();
    int _count_pop = 0;

    for (int _count = 0; _count < MIN(_count_max,100); _count++)
    {
        //ポップアップメニューの追加のためのハンドル生成
        if (_count % 10 == 0)
        {
            _hPupSubMenu = CreatePopupMenu();
        }

        int _shortcut_key_number = _count % 10+1;
        if (_shortcut_key_number == 10)_shortcut_key_number = 0;

        std::wstringstream _menu_strstream;
        _menu_strstream << "(&"<< _shortcut_key_number <<")";
        _menu_strstream << string2wstring(_cam_urls[_count][0].c_str());
        _menu_strstream << L" (";
        _menu_strstream << string2wstring(_cam_urls[_count][1].c_str());
        _menu_strstream << L")";
        //_menu_strstream << L" "<< _count + IDM_CAM_001; //ID番号の表示
        std::wstring _menu_str = _menu_strstream.str();
        AppendMenu(_hPupSubMenu, MF_STRING, _count + IDM_CAM_001, (LPCWSTR)_menu_str.c_str());

        //ポップアップメニューの追加
        if (_count % 10 == 0)
        {
            MENUITEMINFO mii = { 0 };
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
            mii.wID = IDM_CAM_POP_1 + _count_pop;
            mii.hSubMenu = _hPupSubMenu;
            mii.dwTypeData = _wcsdup(_sub_menu_title[_count_pop].c_str());
            InsertMenuItem(_hSubMenu, _count_pop + IDM_CAM_POP_1, TRUE, &mii);
            _count_pop++;
        }
    }
     return 0;
}

int view_single_cam(HWND hWnd, int _cam_id)
{
    ViewMode = IDM_VIEW_1;
    PROC_STOP();
    PROC_FIX_CAM(_cam_id-1);
    PROC_CYCLE(false);
    PROC_START(hWnd);
    return 0;
}

std::vector<T_CAM_URLS> splitIntoFour(const T_CAM_URLS& vec) 
{
    std::vector<T_CAM_URLS> result(4);
    size_t length = vec.size() / 4;
    size_t remain = vec.size() % 4;

    size_t begin = 0;
    size_t end = 0;

    for (int i = 0; i < 4; ++i) {
        end = begin + length + (remain > 0 ? 1 : 0);
        if (remain > 0) remain--;

        result[i] = T_CAM_URLS(vec.begin() + begin, vec.begin() + end);
        begin = end;
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////
//ウィンドウプロシージャ システムからのコールを処理する。
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //std::vector<std::string> urls;

    //OPENFILENAME ofn;       // OPENFILENAME 構造体
    wchar_t szFile[260] = { 0 }; // ファイル名格納用のバッファ
    HMENU _hMenu; //グローバルにもあるので注意

    switch (message)
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            case IDM_MODE_STREAM:
                appmode = APPMODE_NETCAM; //IPカメラにする とりあえずここに入れるが、モード切替のユーザーインターフェースは検討中
                DrawCycleMode = DRAWCYCLE_STREAM;
                PROC_STOP();
                PROC_START(hWnd);
                break;
            case IDM_MODE_YOLO:
                DrawCycleMode = DRAWCYCLE_YOLO5;
                PROC_STOP();
                PROC_START(hWnd);
                break;
            case IDM_MODE_YOLO8:
                DrawCycleMode = DRAWCYCLE_YOLO8;
                PROC_STOP();
                PROC_START(hWnd);
                break;
            case IDM_MODE_POSE:
                DrawCycleMode = DRAWCYCLE_POSENET;
                PROC_STOP();
                PROC_START(hWnd);
                break;

            case IDM_STREAM_START:
                PROC_START(hWnd);
                break;
            case IDM_STREAM_STOP:
                PROC_STOP();
                break;
            case IDM_VIEW_1:
                ViewMode = IDM_VIEW_1;
                PROC_STOP();
                PROC_FIX_CAM(-1);
                PROC_CYCLE(true);
                PROC_START(hWnd);
                _hMenu = GetMenu(hWnd);
                CheckMenuItem(_hMenu, IDM_VIEW_1, MF_CHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_4, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_6, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_9, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_12, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_16, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_36, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_64, MF_UNCHECKED);
                DrawMenuBar(hWnd);
                break;
            case IDM_VIEW_4:
                ViewMode = IDM_VIEW_4;
                PROC_STOP();
                PROC_FIX_CAM(-1);
                PROC_CYCLE(true);
                PROC_START(hWnd);
                _hMenu = GetMenu(hWnd);
                CheckMenuItem(_hMenu, IDM_VIEW_1, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_4, MF_CHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_6, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_9, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_12, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_16, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_36, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_64, MF_UNCHECKED);
                DrawMenuBar(hWnd);
                break;
            case IDM_VIEW_6:
                ViewMode = IDM_VIEW_6;
                PROC_STOP();
                PROC_FIX_CAM(-1);
                PROC_CYCLE(true);
                PROC_START(hWnd);
                _hMenu = GetMenu(hWnd);
                CheckMenuItem(_hMenu, IDM_VIEW_1, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_4, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_6, MF_CHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_9, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_12, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_16, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_36, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_64, MF_UNCHECKED);
                DrawMenuBar(hWnd);
                break;
            case IDM_VIEW_9:
                ViewMode = IDM_VIEW_9;
                PROC_STOP();
                PROC_FIX_CAM(-1);
                PROC_CYCLE(true);
                PROC_START(hWnd);
                _hMenu = GetMenu(hWnd);
                CheckMenuItem(_hMenu, IDM_VIEW_1, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_4, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_6, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_9, MF_CHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_12, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_16, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_36, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_64, MF_UNCHECKED);
                DrawMenuBar(hWnd);
                break;
            case IDM_VIEW_12:
                ViewMode = IDM_VIEW_12;
                PROC_STOP();
                PROC_FIX_CAM(-1);
                PROC_CYCLE(true);
                PROC_START(hWnd);
                _hMenu = GetMenu(hWnd);
                CheckMenuItem(_hMenu, IDM_VIEW_1, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_4, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_6, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_9, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_12, MF_CHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_16, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_36, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_64, MF_UNCHECKED);
                DrawMenuBar(hWnd);
                break;
            case IDM_VIEW_16:
                ViewMode = IDM_VIEW_16;
                PROC_STOP();
                PROC_FIX_CAM(-1);
                PROC_CYCLE(true);
                PROC_START(hWnd);
                _hMenu = GetMenu(hWnd);
                CheckMenuItem(_hMenu, IDM_VIEW_1, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_4, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_6, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_9, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_12, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_16, MF_CHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_36, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_64, MF_UNCHECKED);
                DrawMenuBar(hWnd);
                break;
            case IDM_VIEW_36:
                ViewMode = IDM_VIEW_36;
                PROC_STOP();
                PROC_FIX_CAM(-1);
                PROC_CYCLE(true);
                PROC_START(hWnd);
                _hMenu = GetMenu(hWnd);
                CheckMenuItem(_hMenu, IDM_VIEW_1, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_4, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_6, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_9, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_12, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_16, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_36, MF_CHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_64, MF_UNCHECKED);
                DrawMenuBar(hWnd);
                break;
            case IDM_VIEW_64:
                ViewMode = IDM_VIEW_64;
                PROC_STOP();
                PROC_FIX_CAM(-1);
                PROC_CYCLE(true);
                PROC_START(hWnd);
                _hMenu = GetMenu(hWnd);
                CheckMenuItem(_hMenu, IDM_VIEW_1, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_4, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_6, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_9, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_12, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_16, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_36, MF_UNCHECKED);
                CheckMenuItem(_hMenu, IDM_VIEW_64, MF_CHECKED);
                DrawMenuBar(hWnd);
                break;


            case IDM_INPUTFILE:
                if (select_file(hWnd, video_file_path, L"MP4\0*.mp4\0MPG\0*.mpg\0MOV\0*.mov\0All\0*.*\0"))
                {
                    appmode = APPMODE_MOVFILE;
                    PROC_START(hWnd);
                    CVW_FILE_END = false;
                }
                else
                    appmode = APPMODE_NETCAM;
                break;
            case IDM_FILE_CAMS:         
                if (select_file(hWnd, url_file, L"TXT\0*.txt\0All\0*.*\0"))
                {
                    PROC_STOP();
                    cam_urls = readRecordsFromFile(url_file);
                    PROC_START(hWnd);
                    set_file_menu_items(hWnd);
                }
                break;
            case IDM_FILE_ONNX:         
                if (select_file(hWnd, onnx_file, L"ONNX\0*.onnx\0All\0*.*\0"))
                {
                    PROC_STOP();
                    //YoloAIParametors yp;
                    //yp._onnx_file_name = onnx_file.c_str();
                    if(ptYoloOBJECTDECTECTION!=nullptr)
                        ptYoloOBJECTDECTECTION->YP.onnx_file_name = onnx_file;
                    set_file_menu_items(hWnd);
                    PROC_START(hWnd);
                }
                break;
            case IDM_FILE_NAMES	: 
                select_file(hWnd, names_file, L"NAMES\0*.names\0All\0*.*\0"); 
                set_file_menu_items(hWnd);
                break;
            case IDM_FILE_ONNX8:
                if (select_file(hWnd, onnx_file8, L"ONNX\0*.onnx\0All\0*.*\0"))
                {
                    PROC_STOP();
                    //YoloAIParametors yp;
                    //yp._onnx_file_name = onnx_file.c_str();
                    if (ptYoloOBJECTDECTECTION != nullptr)
                        ptYoloOBJECTDECTECTION->YP.onnx_file_name = onnx_file8;
                    set_file_menu_items(hWnd);
                    PROC_START(hWnd);
                }
                break;
            case IDM_FILE_NAMES8:
                select_file(hWnd, names_file8, L"NAMES\0*.names\0All\0*.*\0");
                set_file_menu_items(hWnd);
                break;
            case IDM_FILE_POSEWEIGHT:
                select_file(hWnd, poseweight, L"CAFFEMODEL\0*.caffemodel\0All\0*.*\0"); 
                set_file_menu_items(hWnd);
                break;
            case IDM_FILE_POSEPROTO:    
                select_file(hWnd, poseproto, L"PROTOTXT\0*.prototxt\0All\0*.*\0"); 
                set_file_menu_items(hWnd);
                break;

            case IDM_VIDEOREC_VGA:
                mlVIDEOWRITERORG.width = 720;
                mlVIDEOWRITERORG.height = 480;
                mlVIDEOWRITERAI.width = 720;
                mlVIDEOWRITERAI.height = 480;
                mlVIDEOWRITERORG.open(add_dt_ext(MP4PATH + MP4VHA, MP4EXT).c_str());
                mlVIDEOWRITERAI.open(add_dt_ext(MP4PATH + MP4VHAAI, MP4EXT).c_str());

                break;
            case IDM_VIDEOREC_HD:
                mlVIDEOWRITERORG.width = 1920;
                mlVIDEOWRITERORG.height = 1080;
                mlVIDEOWRITERAI.width = 1920;
                mlVIDEOWRITERAI.height = 1080;
                mlVIDEOWRITERORG.open(add_dt_ext(MP4PATH + MP4HD, MP4EXT).c_str());
                mlVIDEOWRITERAI.open(add_dt_ext(MP4PATH + MP4HDAI, MP4EXT).c_str());
                break;
            case IDM_VIDEOREC_END:
                mlVIDEOWRITERORG.release();
                mlVIDEOWRITERAI.release();
                break;

            case IDM_C_000:set_display_time_seconds(  0); DTSM(hWnd); break;
            case IDM_C_001:set_display_time_seconds(  1); DTSM(hWnd); break;
            case IDM_C_003:set_display_time_seconds(  3); DTSM(hWnd); break;
            case IDM_C_008:set_display_time_seconds(  8); DTSM(hWnd); break;
            case IDM_C_015:set_display_time_seconds( 15); DTSM(hWnd); break;
            case IDM_C_030:set_display_time_seconds( 30); DTSM(hWnd); break;
            case IDM_C_060:set_display_time_seconds( 60); DTSM(hWnd); break;
            case IDM_C_120:set_display_time_seconds(120); DTSM(hWnd); break;
            case IDM_C_300:set_display_time_seconds(300); DTSM(hWnd); break;
            case IDM_C_600:set_display_time_seconds(600); DTSM(hWnd); break;

            case IDM_F_60SEC:set_frame_between_time(60000); FBTM(hWnd); break;
            case IDM_F_30SEC:set_frame_between_time(30000); FBTM(hWnd); break;
            case IDM_F_10SEC:set_frame_between_time(10000); FBTM(hWnd); break;
            case IDM_F_0003:set_frame_between_time(3000); FBTM(hWnd); break;
            case IDM_F_0005:set_frame_between_time(2000); FBTM(hWnd); break;
            case IDM_F_0008:set_frame_between_time(1500); FBTM(hWnd); break;
            case IDM_F_0010:set_frame_between_time(1000); FBTM(hWnd); break;
            case IDM_F_0020:set_frame_between_time(500 ); FBTM(hWnd); break;
            case IDM_F_0100:set_frame_between_time(100 ); FBTM(hWnd); break;
            case IDM_F_0200:set_frame_between_time(50  ); FBTM(hWnd); break;
            case IDM_F_0300:set_frame_between_time(33  ); FBTM(hWnd); break;
            case IDM_F_0600:set_frame_between_time(16  ); FBTM(hWnd); break;
            case IDM_F_1200:set_frame_between_time(8   ); FBTM(hWnd); break;

            case IDM_AI_SCORE_THRESHOLD_000:set_score_threshold(0.001f, ptYoloOBJECTDECTECTION); STHM(hWnd); break;
            case IDM_AI_SCORE_THRESHOLD_010:set_score_threshold(0.1f  , ptYoloOBJECTDECTECTION); STHM(hWnd); break;
            case IDM_AI_SCORE_THRESHOLD_020:set_score_threshold(0.2f  , ptYoloOBJECTDECTECTION); STHM(hWnd); break;
            case IDM_AI_SCORE_THRESHOLD_030:set_score_threshold(0.3f  , ptYoloOBJECTDECTECTION); STHM(hWnd); break;
            case IDM_AI_SCORE_THRESHOLD_040:set_score_threshold(0.4f  , ptYoloOBJECTDECTECTION); STHM(hWnd); break;
            case IDM_AI_SCORE_THRESHOLD_050:set_score_threshold(0.5f  , ptYoloOBJECTDECTECTION); STHM(hWnd); break;
            case IDM_AI_SCORE_THRESHOLD_060:set_score_threshold(0.6f  , ptYoloOBJECTDECTECTION); STHM(hWnd); break;
            case IDM_AI_SCORE_THRESHOLD_070:set_score_threshold(0.7f  , ptYoloOBJECTDECTECTION); STHM(hWnd); break;
            case IDM_AI_SCORE_THRESHOLD_080:set_score_threshold(0.8f  , ptYoloOBJECTDECTECTION); STHM(hWnd); break;
            case IDM_AI_SCORE_THRESHOLD_090:set_score_threshold(0.9f  , ptYoloOBJECTDECTECTION); STHM(hWnd); break;
            case IDM_AI_SCORE_THRESHOLD_100:set_score_threshold(1.0f  , ptYoloOBJECTDECTECTION); STHM(hWnd); break;

            case IDM_AI_NMS_THRESHOLD_000:set_nms_threshold(0.0f, ptYoloOBJECTDECTECTION); SNHM(hWnd); break;
            case IDM_AI_NMS_THRESHOLD_010:set_nms_threshold(0.1f, ptYoloOBJECTDECTECTION); SNHM(hWnd); break;
            case IDM_AI_NMS_THRESHOLD_020:set_nms_threshold(0.2f, ptYoloOBJECTDECTECTION); SNHM(hWnd); break;
            case IDM_AI_NMS_THRESHOLD_030:set_nms_threshold(0.3f, ptYoloOBJECTDECTECTION); SNHM(hWnd); break;
            case IDM_AI_NMS_THRESHOLD_040:set_nms_threshold(0.4f, ptYoloOBJECTDECTECTION); SNHM(hWnd); break;
            case IDM_AI_NMS_THRESHOLD_050:set_nms_threshold(0.5f, ptYoloOBJECTDECTECTION); SNHM(hWnd); break;
            case IDM_AI_NMS_THRESHOLD_060:set_nms_threshold(0.6f, ptYoloOBJECTDECTECTION); SNHM(hWnd); break;
            case IDM_AI_NMS_THRESHOLD_070:set_nms_threshold(0.7f, ptYoloOBJECTDECTECTION); SNHM(hWnd); break;
            case IDM_AI_NMS_THRESHOLD_080:set_nms_threshold(0.8f, ptYoloOBJECTDECTECTION); SNHM(hWnd); break;
            case IDM_AI_NMS_THRESHOLD_090:set_nms_threshold(0.9f, ptYoloOBJECTDECTECTION); SNHM(hWnd); break;
            case IDM_AI_NMS_THRESHOLD_100:set_nms_threshold(1.0f, ptYoloOBJECTDECTECTION); SNHM(hWnd); break;

            case IDM_AI_CONF_THRESHOLD_000:set_conf_threshold(0.001f, ptYoloOBJECTDECTECTION); SCHM(hWnd); break;
            case IDM_AI_CONF_THRESHOLD_010:set_conf_threshold(0.1f  , ptYoloOBJECTDECTECTION); SCHM(hWnd); break;
            case IDM_AI_CONF_THRESHOLD_020:set_conf_threshold(0.2f  , ptYoloOBJECTDECTECTION); SCHM(hWnd); break;
            case IDM_AI_CONF_THRESHOLD_030:set_conf_threshold(0.3f  , ptYoloOBJECTDECTECTION); SCHM(hWnd); break;
            case IDM_AI_CONF_THRESHOLD_040:set_conf_threshold(0.4f  , ptYoloOBJECTDECTECTION); SCHM(hWnd); break;
            case IDM_AI_CONF_THRESHOLD_050:set_conf_threshold(0.5f  , ptYoloOBJECTDECTECTION); SCHM(hWnd); break;
            case IDM_AI_CONF_THRESHOLD_060:set_conf_threshold(0.6f  , ptYoloOBJECTDECTECTION); SCHM(hWnd); break;
            case IDM_AI_CONF_THRESHOLD_070:set_conf_threshold(0.7f  , ptYoloOBJECTDECTECTION); SCHM(hWnd); break;
            case IDM_AI_CONF_THRESHOLD_080:set_conf_threshold(0.8f  , ptYoloOBJECTDECTECTION); SCHM(hWnd); break;
            case IDM_AI_CONF_THRESHOLD_090:set_conf_threshold(0.9f  , ptYoloOBJECTDECTECTION); SCHM(hWnd); break;
            case IDM_AI_CONF_THRESHOLD_100:set_conf_threshold(1.0f  , ptYoloOBJECTDECTECTION); SCHM(hWnd); break;

            case IDM_CAM_001: view_single_cam(hWnd, 1); break;
            case IDM_CAM_002: view_single_cam(hWnd, 2); break;
            case IDM_CAM_003: view_single_cam(hWnd, 3); break;
            case IDM_CAM_004: view_single_cam(hWnd, 4); break;
            case IDM_CAM_005: view_single_cam(hWnd, 5); break;
            case IDM_CAM_006: view_single_cam(hWnd, 6); break;
            case IDM_CAM_007: view_single_cam(hWnd, 7); break;
            case IDM_CAM_008: view_single_cam(hWnd, 8); break;
            case IDM_CAM_009: view_single_cam(hWnd, 9); break;
            case IDM_CAM_010: view_single_cam(hWnd, 10); break;
            case IDM_CAM_011: view_single_cam(hWnd, 11); break;
            case IDM_CAM_012: view_single_cam(hWnd, 12); break;
            case IDM_CAM_013: view_single_cam(hWnd, 13); break;
            case IDM_CAM_014: view_single_cam(hWnd, 14); break;
            case IDM_CAM_015: view_single_cam(hWnd, 15); break;
            case IDM_CAM_016: view_single_cam(hWnd, 16); break;
            case IDM_CAM_017: view_single_cam(hWnd, 17); break;
            case IDM_CAM_018: view_single_cam(hWnd, 18); break;
            case IDM_CAM_019: view_single_cam(hWnd, 19); break;
            case IDM_CAM_020: view_single_cam(hWnd, 20); break;
            case IDM_CAM_021: view_single_cam(hWnd, 21); break;
            case IDM_CAM_022: view_single_cam(hWnd, 22); break;
            case IDM_CAM_023: view_single_cam(hWnd, 23); break;
            case IDM_CAM_024: view_single_cam(hWnd, 24); break;
            case IDM_CAM_025: view_single_cam(hWnd, 25); break;
            case IDM_CAM_026: view_single_cam(hWnd, 26); break;
            case IDM_CAM_027: view_single_cam(hWnd, 27); break;
            case IDM_CAM_028: view_single_cam(hWnd, 28); break;
            case IDM_CAM_029: view_single_cam(hWnd, 29); break;
            case IDM_CAM_030: view_single_cam(hWnd, 30); break;
            case IDM_CAM_031: view_single_cam(hWnd, 31); break;
            case IDM_CAM_032: view_single_cam(hWnd, 32); break;
            case IDM_CAM_033: view_single_cam(hWnd, 33); break;
            case IDM_CAM_034: view_single_cam(hWnd, 34); break;
            case IDM_CAM_035: view_single_cam(hWnd, 35); break;
            case IDM_CAM_036: view_single_cam(hWnd, 36); break;
            case IDM_CAM_037: view_single_cam(hWnd, 37); break;
            case IDM_CAM_038: view_single_cam(hWnd, 38); break;
            case IDM_CAM_039: view_single_cam(hWnd, 39); break;
            case IDM_CAM_040: view_single_cam(hWnd, 40); break;
            case IDM_CAM_041: view_single_cam(hWnd, 41); break;
            case IDM_CAM_042: view_single_cam(hWnd, 42); break;
            case IDM_CAM_043: view_single_cam(hWnd, 43); break;
            case IDM_CAM_044: view_single_cam(hWnd, 44); break;
            case IDM_CAM_045: view_single_cam(hWnd, 45); break;
            case IDM_CAM_046: view_single_cam(hWnd, 46); break;
            case IDM_CAM_047: view_single_cam(hWnd, 47); break;
            case IDM_CAM_048: view_single_cam(hWnd, 48); break;
            case IDM_CAM_049: view_single_cam(hWnd, 49); break;
            case IDM_CAM_050: view_single_cam(hWnd, 50); break;
            case IDM_CAM_051: view_single_cam(hWnd, 51); break;
            case IDM_CAM_052: view_single_cam(hWnd, 52); break;
            case IDM_CAM_053: view_single_cam(hWnd, 53); break;
            case IDM_CAM_054: view_single_cam(hWnd, 54); break;
            case IDM_CAM_055: view_single_cam(hWnd, 55); break;
            case IDM_CAM_056: view_single_cam(hWnd, 56); break;
            case IDM_CAM_057: view_single_cam(hWnd, 57); break;
            case IDM_CAM_058: view_single_cam(hWnd, 58); break;
            case IDM_CAM_059: view_single_cam(hWnd, 59); break;
            case IDM_CAM_060: view_single_cam(hWnd, 60); break;
            case IDM_CAM_061: view_single_cam(hWnd, 61); break;
            case IDM_CAM_062: view_single_cam(hWnd, 62); break;
            case IDM_CAM_063: view_single_cam(hWnd, 63); break;
            case IDM_CAM_064: view_single_cam(hWnd, 64); break;
            case IDM_CAM_065: view_single_cam(hWnd, 65); break;
            case IDM_CAM_066: view_single_cam(hWnd, 66); break;
            case IDM_CAM_067: view_single_cam(hWnd, 67); break;
            case IDM_CAM_068: view_single_cam(hWnd, 68); break;
            case IDM_CAM_069: view_single_cam(hWnd, 69); break;
            case IDM_CAM_070: view_single_cam(hWnd, 70); break;
            case IDM_CAM_071: view_single_cam(hWnd, 71); break;
            case IDM_CAM_072: view_single_cam(hWnd, 72); break;
            case IDM_CAM_073: view_single_cam(hWnd, 73); break;
            case IDM_CAM_074: view_single_cam(hWnd, 74); break;
            case IDM_CAM_075: view_single_cam(hWnd, 75); break;
            case IDM_CAM_076: view_single_cam(hWnd, 76); break;
            case IDM_CAM_077: view_single_cam(hWnd, 77); break;
            case IDM_CAM_078: view_single_cam(hWnd, 78); break;
            case IDM_CAM_079: view_single_cam(hWnd, 79); break;
            case IDM_CAM_080: view_single_cam(hWnd, 80); break;
            case IDM_CAM_081: view_single_cam(hWnd, 81); break;
            case IDM_CAM_082: view_single_cam(hWnd, 82); break;
            case IDM_CAM_083: view_single_cam(hWnd, 83); break;
            case IDM_CAM_084: view_single_cam(hWnd, 84); break;
            case IDM_CAM_085: view_single_cam(hWnd, 85); break;
            case IDM_CAM_086: view_single_cam(hWnd, 86); break;
            case IDM_CAM_087: view_single_cam(hWnd, 87); break;
            case IDM_CAM_088: view_single_cam(hWnd, 88); break;
            case IDM_CAM_089: view_single_cam(hWnd, 89); break;
            case IDM_CAM_090: view_single_cam(hWnd, 90); break;
            case IDM_CAM_091: view_single_cam(hWnd, 91); break;
            case IDM_CAM_092: view_single_cam(hWnd, 92); break;
            case IDM_CAM_093: view_single_cam(hWnd, 93); break;
            case IDM_CAM_094: view_single_cam(hWnd, 94); break;
            case IDM_CAM_095: view_single_cam(hWnd, 95); break;
            case IDM_CAM_096: view_single_cam(hWnd, 96); break;
            case IDM_CAM_097: view_single_cam(hWnd, 97); break;
            case IDM_CAM_098: view_single_cam(hWnd, 98); break;
            case IDM_CAM_099: view_single_cam(hWnd, 99); break;
            case IDM_CAM_100: view_single_cam(hWnd, 100); break;

                
            //テキストを表示 現在は機能をoff
            case IDM_TEXTOUTPUT:
            {
                if (AI_TEXT_OUTPUT)
                    AI_TEXT_OUTPUT = false;
                else
                {
                    AI_TEXT_OUTPUT = true;
                    if (0) // テキストを表示 現在は機能をoff
                    {
                        hEdit = CreateWindowEx(
                            WS_EX_CLIENTEDGE,
                            L"EDIT",
                            L"",
                            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
                            10, 10, 500, 300,
                            hWnd,
                            (HMENU)IDC_TEXTWINDOW,
                            GetModuleHandle(NULL),
                            NULL);

                        // エディットコントロールのテキストと背景色を設定
                        //HDC _hdc = GetDC(hEdit);
                        //SetTextColor((HDC)_hdc, RGB(255, 255, 255)); // テキスト色を白に
                        //SetBkColor((HDC)_hdc, RGB(0, 0, 0));         // 背景色を黒に
                        //ReleaseDC(hEdit, _hdc);
                        //SetTextColor((HDC)wParam, RGB(255, 255, 255)); // テキスト色を白に
                        //SetBkColor((HDC)wParam, RGB(0, 0, 0));         // 背景色を黒に

                        //UpdateWindow(hEdit);

                        //SendMessage(hEdit, WM_SETFONT, (WPARAM)hFnt, MAKELPARAM(FALSE, 0));
                        //SendMessage(hEdit, WM_SE, (WPARAM)hFnt, MAKELPARAM(FALSE, 0));
                        ShowWindow(hWnd, SW_SHOW);
                        UpdateWindow(hWnd);

                        if (hEdit == NULL) {
                            MessageBox(hWnd, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR);
                        }
                        else {
                            // テキストボックスにテキストを設定
                            std::string text = "0123456789abcdefghijklmnopqrstuvwxyz\n";
                            std::string repeatedText;
                            for (int i = 0; i < 10; ++i) {
                                repeatedText += text;
                            }
                            //SetWindowText(hEdit, string2wstring(repeatedText.c_str()));
                            SetWindowText(hEdit, string2wstring(repeatedText).c_str());
                        }
                    }
                }
            }
            break;
            case WM_CTLCOLOREDIT:
                SetBkColor((HDC)wParam, RGB(0, 0, 0));
                return (INT_PTR)hbrBlackBrush;
                
            case IDM_MULTIMONITOR_1:
                set_fullscreen(hWnd, 0);
                WINDOW_MAX = 3;
                isFULLSCREEN = true;
                break;
            case IDM_MULTIMONITOR_2:
                set_fullscreen(hWnd,1);
                WINDOW_MAX = 4;
                isFULLSCREEN = true;
                break;
            case IDM_MULTIMONITOR_3:
                set_fullscreen(hWnd, 2);
                WINDOW_MAX = 5;
                isFULLSCREEN = true;
                break;
            case IDM_MULTIMONITOR_4:
                set_fullscreen(hWnd, 3);
                WINDOW_MAX = 6;
                isFULLSCREEN = true;
                break;
            case IDM_MULTIMONITOR_1M:
                ToggleFullscreenWithMenu(hWnd);
                WINDOW_MAX = 2;
                isFULLSCREEN = true;
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }
        break;
    //////////////////////////////////////////////////////////////////
    //WM_CREATE
    case WM_CREATE:
    {
        //winmainの最初の方に移動
        if(0)
            read_write_file(init_file, RWFILE_READ);
        //カメラリストを読み込む       
        cam_urls = readRecordsFromFile(url_file);

        set_camera_menu(hWnd, cam_urls);

        ///////////////////////////////////////////////////////////////////////
        //poseweight, poseprotoの両方に値が入っていれば DRAWCYCLE_POSENETをON
        if (DrawCycleMode == DRAWCYCLE_POSENET)
        {
            if (poseweight != "" && poseproto != "")
            {
                load_PoseNet(AI_LOAD);
            }
            else
                DrawCycleMode = DRAWCYCLE_STREAM;
        }
        else if (DrawCycleMode == DRAWCYCLE_YOLO5)
        {
            load_YoloObjectDetection(AI_LOAD);
        }
        else if (DrawCycleMode == DRAWCYCLE_YOLO8)
        {
            load_YoloObjectDetection(AI_LOAD);
        }
        else
            /*STREAM*/;

        ///////////////////////////////////////////////////////////////////////
        //メニュー表示を更新
        set_file_menu_items(hWnd);
        set_display_time_seconds_menuitems(hWnd);
        set_frame_between_time_menuitems(hWnd);
        ///////////////////////////////////////////////////////////////////////
        //メニュー表示を更新 ptYoloのインスタンスが無いと更新されないのでここ
        STHM(hWnd);
        SNHM(hWnd);
        SCHM(hWnd);

        if(WINDOW_MAX==1)           ShowWindow(hWnd, SW_MAXIMIZE); // ウィンドウを最大化
        else if (WINDOW_MAX == 2) { ToggleFullscreenWithMenu(hWnd); isFULLSCREEN = true; }
        else if (WINDOW_MAX == 3)   set_fullscreen(hWnd, 0);
        else if (WINDOW_MAX == 4)   set_fullscreen(hWnd, 1);
        else if (WINDOW_MAX == 5)   set_fullscreen(hWnd, 2);
        else if (WINDOW_MAX == 6)   set_fullscreen(hWnd, 3);

        // ブラシの作成（背景色を黒に設定）
        hbrBlackBrush = CreateSolidBrush(RGB(0, 0, 0));
    }
    break;
    //ダブルクリック 全画面<=>ウィンドウ
    case WM_LBUTTONDBLCLK:
    {
        if (isFULLSCREEN) 
        {
            WINDOW_MAX = 0;
            isFULLSCREEN = false;
            //isFULLSCREEN = !isFULLSCREEN;
            set_cvw_stop(false);
            // ウィンドウを元のサイズと位置に戻す
            ResumeWindow(hWnd);

            //SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
            //SetWindowPos(hWnd, HWND_TOP, mlWINDOWRECT.left, mlWINDOWRECT.top, mlWINDOWRECT.right - mlWINDOWRECT.left, mlWINDOWRECT.bottom - mlWINDOWRECT.top, SWP_FRAMECHANGED);
            //SetMenu(hWnd, hMENU_FOR_FULLSCRENN);
            //ShowWindow(hWnd, SW_SHOW);
            //InvalidateRect(hWnd, NULL, TRUE);
        }
        else
        {
            HMENU hPopupMenu = CreatePopupMenu();
            if (hPopupMenu) {
                // メニュー項目を追加
                AppendMenu(hPopupMenu, MF_STRING, IDM_MULTIMONITOR_1, TEXT("モニター 1"));
                AppendMenu(hPopupMenu, MF_STRING, IDM_MULTIMONITOR_2, TEXT("モニター 2"));
                AppendMenu(hPopupMenu, MF_STRING, IDM_MULTIMONITOR_3, TEXT("モニター 3"));
                AppendMenu(hPopupMenu, MF_STRING, IDM_MULTIMONITOR_4, TEXT("モニター 4"));
                AppendMenu(hPopupMenu, MF_STRING, IDM_MULTIMONITOR_1M, TEXT("メニューバーを残して最大化"));

                // クリック位置を取得
                POINT pt;
                pt.x = LOWORD(lParam);
                pt.y = HIWORD(lParam);

                // ポップアップメニューを表示
                ClientToScreen(hWnd, &pt);
                TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
                DestroyMenu(hPopupMenu);
            }
        }
    }
    break;
    //スペースキーを押したときの処理 次のカメラに切替
    case WM_KEYDOWN:
        if (wParam == VK_SPACE)
        {
            //MessageBox(hWnd, L"スペースキーが押されました", L"キーイベント", MB_OK);
            NEXT_SOURCE = true;
        }
        break;
            
        
    //画面の端をドラッグしてウィンドウの大きさを変更すると大量のWM_PAINTが出て、ハングする可能性
    case WM_ENTERSIZEMOVE:
    //case WM_SIZING:
        // 画面の端をドラッグしたときに WM_PAINT イベントを抑制
        bSUPPRESS_PAINT = true;
        DefWindowProc(hWnd, message, wParam, lParam);
        break;
    case WM_EXITSIZEMOVE:
    //case WM_SIZE:
        bSUPPRESS_PAINT = false;

        DefWindowProc(hWnd, message, wParam, lParam);
        break;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //WM_PAINT
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    case WM_PAINT:
    {
        //描画中 処理が重複・輻輳しないよう、はじく
        if (WM_PAINT_NOW)
        {
            break;
        }
        //画面サイズ変更をしている時の処理
        else if (bSUPPRESS_PAINT)
        {
            if (0)
                PROC_STOP();
        }
        else
        {   
            WM_PAINT_NOW = true;
            //とりあえず黒く塗る
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd, &ps);
            AllPaintBlack(hWnd, hDC, DrawArea(1,1,0,0));
            EndPaint(hWnd, &ps);

            //IPカメラ
            if (appmode == APPMODE_NETCAM)
            {
                //HDC hDC = ::GetDC(hWnd);
                if (ViewMode == IDM_VIEW_1)
                {
                    //PROC_FIX_CAM(-1);
                    PROC_CYCLE(true);
                    //描画スレッドがない場合は立ち上げ
                    if (main_th[0] == nullptr)
                    {
                        set_cvw_stop(false);
                        main_th[0] = new std::thread(
                            &DrawCV2Window,
                            DrawCycleMode,
                            hWnd,
                            hDC,
                            DrawArea(1,1,0,0),
                            true,
                            ptYoloOBJECTDECTECTION,
                            ptPOSENET,
                            cam_urls
                            //pAICSV
                        );
                    }
                }
                //画面分割の処理 分割した画面ごとにスレッドを起動する。
                else if(ViewMode == IDM_VIEW_4)//Mode4View==true
                {
                    //std::vector<std::vector<std::vector<std::string>>> _cam_urls_list = splitIntoFour(cam_urls);
                    std::vector<std::vector<std::vector<std::string>>> _cam_urls_list = splitIntoN(cam_urls,4);
                    if (main_th[0] == nullptr) main_th[0] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(2, 2, 0, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[0]);
                    if (main_th[1] == nullptr) main_th[1] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(2, 2, 1, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[1]);
                    if (main_th[2] == nullptr) main_th[2] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(2, 2, 0, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[2]);
                    if (main_th[3] == nullptr) main_th[3] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(2, 2, 1, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[3]);
                }
                else if (ViewMode == IDM_VIEW_6)//Mode4View==true
                {
                    std::vector<std::vector<std::vector<std::string>>> _cam_urls_list = splitIntoN(cam_urls, 6);
                    if (main_th[0] == nullptr) main_th[0] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 2, 0, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[0]);
                    if (main_th[1] == nullptr) main_th[1] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 2, 0, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[1]);
                    if (main_th[2] == nullptr) main_th[2] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 2, 1, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[2]);
                    if (main_th[3] == nullptr) main_th[3] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 2, 1, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[3]);
                    if (main_th[4] == nullptr) main_th[4] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 2, 2, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[4]);
                    if (main_th[5] == nullptr) main_th[5] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 2, 2, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[5]);
                }
                else if (ViewMode == IDM_VIEW_9)//Mode4View==true
                {
                    std::vector<std::vector<std::vector<std::string>>> _cam_urls_list = splitIntoN(cam_urls, 9);
                    if (main_th[0] == nullptr) main_th[0] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 3, 0, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[0]);
                    if (main_th[1] == nullptr) main_th[1] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 3, 1, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[1]);
                    if (main_th[2] == nullptr) main_th[2] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 3, 2, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[2]);
                    if (main_th[3] == nullptr) main_th[3] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 3, 0, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[3]);
                    if (main_th[4] == nullptr) main_th[4] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 3, 1, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[4]);
                    if (main_th[5] == nullptr) main_th[5] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 3, 2, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[5]);
                    if (main_th[6] == nullptr) main_th[6] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 3, 0, 2), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[6]);
                    if (main_th[7] == nullptr) main_th[7] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 3, 1, 2), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[7]);
                    if (main_th[8] == nullptr) main_th[8] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(3, 3, 2, 2), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[8]);
                }
                else if (ViewMode == IDM_VIEW_12)//Mode4View==true
                {
                    std::vector<std::vector<std::vector<std::string>>> _cam_urls_list = splitIntoN(cam_urls, 12);
                    if (main_th[0] == nullptr)  main_th[0]  = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 0, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[0]);
                    if (main_th[1] == nullptr)  main_th[1]  = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 0, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[1]);
                    if (main_th[2] == nullptr)  main_th[2]  = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 0, 2), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[2]);
                    if (main_th[3] == nullptr)  main_th[3]  = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 1, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[3]);
                    if (main_th[4] == nullptr)  main_th[4]  = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 1, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[4]);
                    if (main_th[5] == nullptr)  main_th[5]  = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 1, 2), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[5]);
                    if (main_th[6] == nullptr)  main_th[6]  = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 2, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[6]);
                    if (main_th[7] == nullptr)  main_th[7]  = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 2, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[7]);
                    if (main_th[8] == nullptr)  main_th[8]  = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 2, 2), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[8]);
                    if (main_th[9] == nullptr)  main_th[9]  = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 3, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[9]);
                    if (main_th[10] == nullptr) main_th[10] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 3, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[10]);
                    if (main_th[11] == nullptr) main_th[11] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 3, 3, 2), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[11]);
                }
                else if (ViewMode == IDM_VIEW_16)//Mode4View==true
                {
                    std::vector<std::vector<std::vector<std::string>>> _cam_urls_list = splitIntoN(cam_urls, 16);
                    if (main_th[0]  == nullptr) main_th[0] =  new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 0, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[0]);
                    if (main_th[1]  == nullptr) main_th[1] =  new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 1, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[1]);
                    if (main_th[2]  == nullptr) main_th[2] =  new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 2, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[2]);
                    if (main_th[3]  == nullptr) main_th[3] =  new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 3, 0), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[3]);
                    if (main_th[4]  == nullptr) main_th[4] =  new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 0, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[4]);
                    if (main_th[5]  == nullptr) main_th[5] =  new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 1, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[5]);
                    if (main_th[6]  == nullptr) main_th[6] =  new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 2, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[6]);
                    if (main_th[7]  == nullptr) main_th[7] =  new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 3, 1), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[7]);
                    if (main_th[8]  == nullptr) main_th[8] =  new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 0, 2), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[8]);
                    if (main_th[9]  == nullptr) main_th[9] =  new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 1, 2), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[9]);
                    if (main_th[10] == nullptr) main_th[10] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 2, 2), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[10]);
                    if (main_th[11] == nullptr) main_th[11] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 3, 2), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[11]);
                    if (main_th[12] == nullptr) main_th[12] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 0, 3), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[12]);
                    if (main_th[13] == nullptr) main_th[13] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 1, 3), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[13]);
                    if (main_th[14] == nullptr) main_th[14] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 2, 3), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[14]);
                    if (main_th[15] == nullptr) main_th[15] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(4, 4, 3, 3), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[15]);
                }
                //ここからはforルーチンで
                else if (ViewMode == IDM_VIEW_36)//Mode4View==true
                {
                    std::vector<std::vector<std::vector<std::string>>> _cam_urls_list = splitIntoN(cam_urls, 36);
                    int k = 0;
                    for (int i=0; i < 6; i++)
                        for (int j=0; j < 6; j++)
                        {
                            if (main_th[k] == nullptr)
                                main_th[k] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(6, 6, i, j), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[k]);
                            k++;
                        }
                }
                else if (ViewMode == IDM_VIEW_64)//Mode4View==true
                {
                    std::vector<std::vector<std::vector<std::string>>> _cam_urls_list = splitIntoN(cam_urls, 64);
                    int k = 0;
                    for (int i=0; i < 8; i++)
                        for (int j=0; j < 8; j++)
                        {
                            if (main_th[k] == nullptr)
                                main_th[k] = new std::thread(&DrawCV2Window, DrawCycleMode, hWnd, hDC, DrawArea(8, 8, i, j), false, ptYoloOBJECTDECTECTION, ptPOSENET, _cam_urls_list[k]);
                            k++;
                        }
                }
            }
            if (appmode == APPMODE_MOVFILE)
            {
                //多重起動を避ける
                //if (!CVW_FILE_PROCESSING && CVW_FILE_END)
                if (!CVW_FILE_PROCESSING)
                {
                    //falseにするのはDrawCV2Windowfの中
                    CVW_FILE_PROCESSING = true;
                    //描画スレッドがあった場合はクリア
                    PROC_STOP();
                   
                    //描画スレッドがない場合は立ち上げ
                    if (main_th[0] == nullptr)
                    {
                        set_cvw_stop(false);
                        main_th[0] = new std::thread(&DrawCV2Windowf,
                            DrawCycleMode,
                            hWnd,
                            hDC,
                            DrawArea(1, 1, 0, 0),
                            true,
                            ptYoloOBJECTDECTECTION,
                            ptPOSENET,
                            video_file_path
                            //pAICSV
                        );
                    }
                }
            }
            //EndPaint(hWnd, &ps);
            //ReleaseDC(hWnd, hDC);
            WM_PAINT_NOW = false;
        }
        //↓これ要る? 要らないようなので外す 11/18
        //bSUPPRESS_PAINT = false;
        break;
    }
    case WM_DESTROY:
        //スレッドを停止
        PROC_STOP();
        PostQuitMessage(0);
        
        //AIをアンロード 関数の中で判断
        load_YoloObjectDetection(AI_UNLOAD);
        load_PoseNet(AI_UNLOAD);

        read_write_file(init_file, RWFILE_WRITE);
        //↓テスト
        //read_write_file(init_file, RWFILE_READ);

        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
