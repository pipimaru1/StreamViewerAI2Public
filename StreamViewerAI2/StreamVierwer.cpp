// StreamVierwer.cpp : アプリケーションのエントリ ポイントを定義します。
//
#include "stdafx.h"
#include "framework.h"
#include "StreamVierwer.h"
#include "yolov5_engine.h"

#include <atlbase.h>
#include <atlconv.h>


#define MAX_LOADSTRING 100

#define _AI_NAMES "default.names"
#define _AI_ONNX "default.onnx"
#define _AI_CAMS "default.txt"
#define _SCORE_THRESHOLD 0.15
#define _NMS_THRESHOLD 0.45
#define _CONFIDENCE_THRESHOLD 0.15
#define _AI_WIDTH 0 //自動
#define _AI_HEIGHT 0 //自動

static std::string url_file; // ストリームURLのリストが記載されたファイル名
static std::string onnx_file; // ストリームURLのリストが記載されたファイル名
static std::string names_file; // ストリームURLのリストが記載されたファイル名

//static LPWSTR url_file; // ストリームURLのリストが記載されたファイル名
//static LPWSTR onnx_file; // ストリームURLのリストが記載されたファイル名
//static LPWSTR names_file; // ストリームURLのリストが記載されたファイル名


// グローバル変数:
HINSTANCE hInst;                    // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];      // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];// メイン ウィンドウ クラス名
HWND hText;                         //テキストボックス
HINSTANCE hInstance;                //テキストボックスのインスタンス

//AI
YoloObjectDetection* _pt_yod = nullptr;		//サムネイル表示の時のAIクラス 複数のカメラでAIを共有、管理クラスからポインタをコピー
int _ai_running = 0;
#define CSVFILE "ai_proccessed.csv"
std::ofstream* pAICSV = nullptr;
volatile bool bSuppressPaint = FALSE;

#define MP4VHA "video_org.mp4"
#define MP4VHAAI "video_ai.mp4"
#define MP4HD "video_hd_org.mp4"
#define MP4HDAI "video_hd_ai.mp4"

volatile int appmode = 0;
#define APPMODE_NETCAM 0
#define APPMODE_USBCAM 1
#define APPMODE_MOVFILE 2

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    pAICSV = new std::ofstream(CSVFILE, std::ios_base::app);

    // TODO: ここにコードを挿入してください。
    /*
    ■引数が一つもない
    ・デフォルトの引数を設定
    ■引数が足りない
    ・リストだけ、
    ■カメラリストファイルが無い
    ・動画モードで起動
    ・メッセージを出す
    ■onnx、namesファイルがない
    ・AI無しで起動する(将来)
    ・いまは終了
    ・メッセージを出す
    */

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
    //初期値の設定
    url_file = _AI_CAMS;
    onnx_file = _AI_ONNX;
    names_file = _AI_NAMES;

    int i = 0;
    for (i = 0; i < nArgs; i++)
    {
        //int msgboxID2 = MessageBox(NULL, szArglist[i], L"test", MB_ICONWARNING | MB_OK);
        if (   wcscmp(szArglist[i], L"-l")==0
            || wcscmp(szArglist[i], L"-list") == 0)
        {
            std::filesystem::path _fp(szArglist[i+1]);
            url_file = _fp.string();
            i++;
        }
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
        else if (wcscmp(szArglist[i], L"-f") == 0
            || wcscmp(szArglist[i], L"-frame") == 0)
        {
            sleep = _wtoi(szArglist[i+1]);
            i++;
        }
        else if (wcscmp(szArglist[i], L"-c") == 0
            || wcscmp(szArglist[i], L"-change") == 0)
        {
            display_time_seconds = _wtoi(szArglist[i+1]);
            i++;
        }
 
        else if (wcscmp(szArglist[i], L"-m") == 0
            || wcscmp(szArglist[i], L"-movie") == 0)
        {
            appmode = APPMODE_MOVFILE;
            int msgboxID = MessageBox(NULL, L"動画ファイル読み込みモードで起動します。\n起動後、ファイルメニューから動画ファイルを指定してください。", L"StreamViewerAI2.exe", MB_ICONWARNING | MB_OK);
        }
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
                << "-l,-list 'カメラリストファイル'" << std::endl
                << "-x,-onnx 'onnxファイル'" << std::endl
                << "-n,-names 'namesファイル'" << std::endl
                << "-c,-change 'カメラ切り替え時間時間[sec]'" << std::endl
                << "-f,-frame 'キャプチャの間隔時間[ms/f]'" << std::endl
                << "-m,-movie 動画ファイル読み込みモード カメラリストファイルは無視" << std::endl
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
            <<"sleep[msec]: " << sleep << std::endl
            <<"flamerate[fps]: "<< display_time_seconds << std::endl;
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
        CA2W wStr(onnx_file.c_str());
        int msgboxID = MessageBox(NULL, wStr, (LPCWSTR)L"ONNXファイルがありません", MB_ICONWARNING | MB_OK);
        return 0;
    }
    if (!std::filesystem::exists(names_file))
    {
        CA2W wStr(names_file.c_str());
        int msgboxID = MessageBox(NULL, wStr, (LPCWSTR)L"NAMESファイルがありません", MB_ICONWARNING | MB_OK);
        return 0;
    }

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_STREAMVIERWER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_STREAMVIERWER));

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

    pAICSV->close();

    return (int) msg.wParam;
}
//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STREAMVIERWER));
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//グローバル変数
//extern std::vector<std::string> urls;
HMENU hMenu;
static bool isFullscreen = false;
static RECT windowRect;
std::thread* _main_th = nullptr;
volatile bool wm_paint_now = false;

std::vector<std::vector<std::string>> readRecordsFromFile(const std::string& filename) 
{
    std::vector<std::vector<std::string>> records;
    std::ifstream inputFile(filename);

    if (inputFile.is_open()) 
    {
        std::string line;
        while (std::getline(inputFile, line)) 
        {
            std::vector<std::string> record;
            std::istringstream lineStream(line);
            std::string item;

            while (std::getline(lineStream, item, ',')) 
            {
                record.push_back(item);
            }
            records.push_back(record);
        }
        inputFile.close();
    }
    else 
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
    return records;
}

static std::vector<RECT> monitors;
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) 
{
    monitors.push_back(*lprcMonitor);
    return TRUE;
}

int set_fullscreen(HWND hWnd,int monitor) 
{
    // システム上のすべてのモニターを列挙
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

    // 2番目のモニターを選択（存在する場合）
    if (monitors.size() > monitor) 
    {
        isFullscreen = true;
        // 元のウィンドウサイズと位置を保存
        GetWindowRect(hWnd, &windowRect);
        hMenu = GetMenu(hWnd);
        SetMenu(hWnd, NULL);

        // 2番目のモニターのスクリーン座標を取得
        RECT monitor_rect = monitors[monitor];

        // ウィンドウを2番目のモニターに移動して全画面表示にする
        SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);

        SetWindowPos(hWnd, HWND_TOP, monitor_rect.left, monitor_rect.top, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);

        // ウィンドウを表示
        ShowWindow(hWnd, SW_SHOW);
        InvalidateRect(hWnd, NULL, TRUE);
    }
    monitors.clear();
    return 0;
}

/*
    wstringをstringへ変換
*/
std::string wstring2string(std::wstring oWString)
{
    // wstring → SJIS
    int iBufferSize = WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, (char*)NULL, 0, NULL, NULL);
    // バッファの取得
    CHAR* cpMultiByte = new CHAR[iBufferSize];
    // wstring → SJIS
    WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, cpMultiByte, iBufferSize, NULL, NULL);
    // stringの生成
    std::string oRet(cpMultiByte, cpMultiByte + iBufferSize - 1);
    // バッファの破棄
    delete[] cpMultiByte;
    // 変換結果を返す
    return(oRet);
}

std::string _video_file_path = "";

//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//  目的: メイン ウィンドウのメッセージを処理します。
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //std::vector<std::string> urls;

    OPENFILENAME ofn;       // OPENFILENAME 構造体
    wchar_t szFile[260] = { 0 }; // ファイル名格納用のバッファ

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

            case IDM_INPUTFILE:

                // OPENFILENAME 構造体の初期化
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = NULL;  // ダイアログボックスの親ウィンドウ
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = L"All\0*.*\0MP4\0*.mp4\0MPG\0*.mpg\0MOV\0*.mov\0"; // 表示するファイルの種類
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
                        //int msgboxID = MessageBox(NULL, ofn.lpstrFile, (LPCWSTR)L"ファイルがありました", MB_ICONWARNING | MB_OK);
                        _video_file_path = wstring2string(ofn.lpstrFile);

                        appmode = APPMODE_MOVFILE;

                        UpdateWindow(hWnd);
                        ShowWindow(hWnd, SW_SHOW);
                        InvalidateRect(hWnd, NULL, TRUE);
                        cvw_file_end = false;

                        //DefWindowProc(hWnd, message, wParam, lParam);
                    }
                    else
                    {
                        int msgboxID = MessageBox(NULL, ofn.lpstrFile, (LPCWSTR)L"ファイルがありません", MB_ICONWARNING | MB_OK);
                        appmode = APPMODE_NETCAM;
                    }
                }

                break;

            case IDM_VIDEOREC_VGA:
                //_VIDEO_REC = true;
                _mvw_org.width = 720;
                _mvw_org.height = 480;
                _mvw_ai.width = 720;
                _mvw_ai.height = 480;
                _mvw_org.open(MP4VHA);
                _mvw_ai.open(MP4VHAAI);
                break;
            case IDM_VIDEOREC_HD:
                //_VIDEO_REC = true;
                _mvw_org.width = 1920;
                _mvw_org.height = 1080;
                _mvw_ai.width = 1920;
                _mvw_ai.height = 1080;
                _mvw_org.open(MP4HD);
                _mvw_ai.open(MP4HDAI);
                break;
            case IDM_VIDEOREC_END:
                _mvw_org.release();
                _mvw_ai.release();
                break;

            case IDM_C_001:set_display_time_seconds(1); break;
            case IDM_C_003:set_display_time_seconds(3); break;
            case IDM_C_008:set_display_time_seconds(8); break;
            case IDM_C_015:set_display_time_seconds(15); break;
            case IDM_C_030:set_display_time_seconds(30); break;
            case IDM_C_060:set_display_time_seconds(60); break;
            case IDM_C_120:set_display_time_seconds(120); break;
            case IDM_C_300:set_display_time_seconds(300); break;
            case IDM_C_600:set_display_time_seconds(600); break;

            case IDM_F_0003:set_frame_between_time(3000); break;
            case IDM_F_0005:set_frame_between_time(2000); break;
            case IDM_F_0008:set_frame_between_time(1500); break;
            case IDM_F_0010:set_frame_between_time(1000); break;
            case IDM_F_0020:set_frame_between_time(500); break;
            case IDM_F_0100:set_frame_between_time(100); break;
            case IDM_F_0200:set_frame_between_time(50); break;
            case IDM_F_0300:set_frame_between_time(33); break;
            case IDM_F_0600:set_frame_between_time(16); break;
            case IDM_F_1200:set_frame_between_time(8); break;

            case IDM_AI_SCORE_THRESHOLD_000:set_score_threshold(0.001f, _pt_yod); break;
            case IDM_AI_SCORE_THRESHOLD_010:set_score_threshold(0.1f, _pt_yod); break;
            case IDM_AI_SCORE_THRESHOLD_020:set_score_threshold(0.2f, _pt_yod); break;
            case IDM_AI_SCORE_THRESHOLD_030:set_score_threshold(0.3f, _pt_yod); break;
            case IDM_AI_SCORE_THRESHOLD_040:set_score_threshold(0.4f, _pt_yod); break;
            case IDM_AI_SCORE_THRESHOLD_050:set_score_threshold(0.5f, _pt_yod); break;
            case IDM_AI_SCORE_THRESHOLD_060:set_score_threshold(0.6f, _pt_yod); break;
            case IDM_AI_SCORE_THRESHOLD_070:set_score_threshold(0.7f, _pt_yod); break;
            case IDM_AI_SCORE_THRESHOLD_080:set_score_threshold(0.8f, _pt_yod); break;
            case IDM_AI_SCORE_THRESHOLD_090:set_score_threshold(0.9f, _pt_yod); break;
            case IDM_AI_SCORE_THRESHOLD_100:set_score_threshold(1.0f, _pt_yod); break;

            case IDM_AI_NMS_THRESHOLD_000:set_nms_threshold(0.0f, _pt_yod); break;
            case IDM_AI_NMS_THRESHOLD_010:set_nms_threshold(0.1f, _pt_yod); break;
            case IDM_AI_NMS_THRESHOLD_020:set_nms_threshold(0.2f, _pt_yod); break;
            case IDM_AI_NMS_THRESHOLD_030:set_nms_threshold(0.3f, _pt_yod); break;
            case IDM_AI_NMS_THRESHOLD_040:set_nms_threshold(0.4f, _pt_yod); break;
            case IDM_AI_NMS_THRESHOLD_050:set_nms_threshold(0.5f, _pt_yod); break;
            case IDM_AI_NMS_THRESHOLD_060:set_nms_threshold(0.6f, _pt_yod); break;
            case IDM_AI_NMS_THRESHOLD_070:set_nms_threshold(0.7f, _pt_yod); break;
            case IDM_AI_NMS_THRESHOLD_080:set_nms_threshold(0.8f, _pt_yod); break;
            case IDM_AI_NMS_THRESHOLD_090:set_nms_threshold(0.9f, _pt_yod); break;
            case IDM_AI_NMS_THRESHOLD_100:set_nms_threshold(1.0f, _pt_yod); break;

            case IDM_AI_CONF_THRESHOLD_000:set_conf_threshold(0.001f, _pt_yod); break;
            case IDM_AI_CONF_THRESHOLD_010:set_conf_threshold(0.1f, _pt_yod); break;
            case IDM_AI_CONF_THRESHOLD_020:set_conf_threshold(0.2f, _pt_yod); break;
            case IDM_AI_CONF_THRESHOLD_030:set_conf_threshold(0.3f, _pt_yod); break;
            case IDM_AI_CONF_THRESHOLD_040:set_conf_threshold(0.4f, _pt_yod); break;
            case IDM_AI_CONF_THRESHOLD_050:set_conf_threshold(0.5f, _pt_yod); break;
            case IDM_AI_CONF_THRESHOLD_060:set_conf_threshold(0.6f, _pt_yod); break;
            case IDM_AI_CONF_THRESHOLD_070:set_conf_threshold(0.7f, _pt_yod); break;
            case IDM_AI_CONF_THRESHOLD_080:set_conf_threshold(0.8f, _pt_yod); break;
            case IDM_AI_CONF_THRESHOLD_090:set_conf_threshold(0.9f, _pt_yod); break;
            case IDM_AI_CONF_THRESHOLD_100:set_conf_threshold(1.0f, _pt_yod); break;

            case IDM_TEXTOUTPUT:
            {
                if (_ai_text_output)
                    _ai_text_output = false;
                else
                    _ai_text_output = true;
            }
            break;
                
            case IDM_MULTIMONITOR_1:
                set_fullscreen(hWnd, 0);
                break;
            case IDM_MULTIMONITOR_2:
                set_fullscreen(hWnd,1);
                break;
            case IDM_MULTIMONITOR_3:
                set_fullscreen(hWnd, 2);
                break;
            case IDM_MULTIMONITOR_4:
                set_fullscreen(hWnd, 3);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }
        break;
    case WM_CREATE:
    {
        cam_urls = readRecordsFromFile(url_file);
        _pt_yod = new YoloObjectDetection;
        YoloAIParametors yp;
        yp._onnx_file_name = onnx_file.c_str();
        yp._names_file_name = names_file.c_str();
        yp._input_width = DEFAULT_AI_INPUT_WIDTH;
        yp._input_height = DEFAULT_AI_INPUT_HEIGHT;
        yp._score_threshold = _SCORE_THRESHOLD;
        yp._nms_threshold = _NMS_THRESHOLD;
        yp._confidence_thresgold = _CONFIDENCE_THRESHOLD;

        _ai_running = _pt_yod->init_yolov5(
            yp,
            true, false);
        }
    break;
    //ダブルクリック 全画面<=>ウィンドウ
    case WM_LBUTTONDBLCLK:
    {
        if (isFullscreen) 
        {
            isFullscreen = false;
            //isFullscreen = !isFullscreen;
            set_cvw_stop(false);
            // ウィンドウを元のサイズと位置に戻す
            SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
            SetWindowPos(hWnd, HWND_TOP, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_FRAMECHANGED);
            SetMenu(hWnd, hMenu);
            ShowWindow(hWnd, SW_SHOW);

            InvalidateRect(hWnd, NULL, TRUE);
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
            _next_source = true;
        }
        break;
            
        
    //画面の端をドラッグしてウィンドウの大きさを変更すると大量のWM_PAINTが出て、ハングする可能性
    case WM_ENTERSIZEMOVE:
    //case WM_SIZING:
        // 画面の端をドラッグしたときに WM_PAINT イベントを抑制
        bSuppressPaint = true;
        DefWindowProc(hWnd, message, wParam, lParam);
        break;
    case WM_EXITSIZEMOVE:
    //case WM_SIZE:
        bSuppressPaint = false;

        DefWindowProc(hWnd, message, wParam, lParam);
        break;

    case WM_PAINT:
    {
        //描画中 処理が重複・輻輳しないよう、はじく
        if (wm_paint_now)
        {
            break;
        }
        //画面サイズ変更をしている時の処理
        else if (bSuppressPaint)
        {
            if(0)
                if (_main_th != nullptr)
                {
                    set_cvw_stop(true);
                    svw_wait_stop();
                    _main_th->join();
                    delete _main_th;
                    _main_th = nullptr;
                }
        }
        else
        {   
            wm_paint_now = true;
            //とりあえず黒く塗る
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd, &ps);
            AllPaintBlack(hWnd, hDC);
            EndPaint(hWnd, &ps);

            //IPカメラ
            if (appmode==APPMODE_NETCAM)
            {
                //描画スレッドがあった場合はクリア
                //if(0)
                //if (_main_th != nullptr)
                //{
                //    set_cvw_stop(true);
                //    svw_wait_stop();
                //    _main_th->join();
                //    delete _main_th;
                //    _main_th = nullptr;
                //}

                //描画スレッドがない場合は立ち上げ
                if (_main_th == nullptr)
                {
                    set_cvw_stop(false);
                    _main_th = new std::thread(&DrawCV2Window, hWnd, _pt_yod, cam_urls, 0, pAICSV);
                }
            }
            if (appmode == APPMODE_MOVFILE)
            {
                if (!cvw_file_processing && !cvw_file_end)
                {
                    //falseにするのはDrawCV2Windowfの中
                    cvw_file_processing = true;
                    //描画スレッドがあった場合はクリア
                    if (_main_th != nullptr)
                    {
                        set_cvw_stop(true);
                        svw_wait_stop();
                        _main_th->join();
                        delete _main_th;
                        _main_th = nullptr;
                    }
                    //描画スレッドがない場合は立ち上げ
                    if (_main_th == nullptr)
                    {
                        set_cvw_stop(false);
                        _main_th = new std::thread(&DrawCV2Windowf, hWnd, _pt_yod, _video_file_path, 0, pAICSV);
                    }
                }
            }
            wm_paint_now = false;
        }
        //↓これ要る? 要らないようなので外す 11/18
        //bSuppressPaint = false;
        break;
    }
    case WM_DESTROY:
        //スレッドを停止
        set_cvw_stop(true);
        if(_main_th != nullptr)
            _main_th->join();

        PostQuitMessage(0);

        if (_pt_yod != nullptr)
        {
            delete _pt_yod;
            _pt_yod = nullptr;
        }
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
