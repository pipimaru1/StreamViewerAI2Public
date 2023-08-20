// StreamVierwer.cpp : アプリケーションのエントリ ポイントを定義します。
//
#include "stdafx.h"
#include "framework.h"
#include "StreamVierwer.h"
#include "yolov5_engine.h"


#define MAX_LOADSTRING 100

#define _AI_NAMES "coco.names"
#define _AI_ONNX "yolov5s.onnx"
#define _AI_CAMS "cams.txt"
#define _SCORE_THRESHOLD 0.15
#define _NMS_THRESHOLD 0.45
#define _CONFIDENCE_THRESHOLD 0.15
#define _AI_WIDTH 0 //自動
#define _AI_HEIGHT 0 //自動

static std::string url_file; // ストリームURLのリストが記載されたファイル名
static std::string onnx_file; // ストリームURLのリストが記載されたファイル名
static std::string names_file; // ストリームURLのリストが記載されたファイル名

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名
HWND hText;     //テキストボックス
HINSTANCE hInstance;    //テキストボックスのインスタンス

//AI
YoloObjectDetection* _pt_yod = nullptr;		//サムネイル表示の時のAIクラス 複数のカメラでAIを共有、管理クラスからポインタをコピー
int _ai_running = 0;
#define CSVFILE "ai_proccessed.csv"
std::ofstream* pAICSV = nullptr;

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
    
    // コマンドライン引数を取得
    LPWSTR _lpCmdLine = GetCommandLineW();
    // コマンドライン引数を個々の引数に分解
    int nArgs=0;
    LPWSTR* szArglist = CommandLineToArgvW(_lpCmdLine, &nArgs);
    // エラー処理
    if (szArglist == NULL) {
        std::wcerr << L"CommandLineToArgvW failed" << std::endl;
        return 1;
    }

    if (nArgs == 2|| nArgs == 4)
    {
        std::filesystem::path _fp(szArglist[1]);
        url_file = _fp.string();

        if (!std::filesystem::exists(_fp))
        {
            int msgboxID = MessageBox(
                NULL,
                (LPCWSTR)L"設定ファイルがありません",
                (LPCWSTR)L"Stream Viewer",
                MB_ICONWARNING | MB_OK
            );
            return 0;
        }
        if (nArgs == 4)
        {
            std::filesystem::path _fp(szArglist[2]);
            std::filesystem::path _fp2(szArglist[3]);
            onnx_file = _fp.string();
            names_file = _fp2.string();

            if (!std::filesystem::exists(_fp))
            {
                int msgboxID = MessageBox(
                    NULL,
                    (LPCWSTR)_fp.string().c_str(),
                    (LPCWSTR)L"指定されたONXXファイルがありません",
                    MB_ICONWARNING | MB_OK
                );
                return 0;
            }
            if (!std::filesystem::exists(_fp2))
            {
                int msgboxID = MessageBox(
                    NULL,
                    (LPCWSTR)_fp2.string().c_str(),
                    (LPCWSTR)L"指定されたNAMESファイルがありません",
                    MB_ICONWARNING | MB_OK
                );
                return 0;
            }
        }
        else
        {
            onnx_file = _AI_ONNX;
            names_file = _AI_NAMES;
        }
    }
    else
    {
        url_file = _AI_CAMS;
        onnx_file = _AI_ONNX;
        names_file = _AI_NAMES;

        if (!std::filesystem::exists(url_file))
        {
            int msgboxID = MessageBox(
                NULL,
                (LPCWSTR)L"設定ファイルを指定してください\n"
                "StreamViewer.exe cams.txt [onnxfile namesfile]\n"
                "cams.txtの中身\n"
                "コンポサブ,http://ID:PW@192.168.11.1/cgi-bin/mjpeg?resolution=1920x1080"
                "コンポメイン,http://ID:PW@192.168.11.2/cgi-bin/mjpeg?resolution=1920x1080",
                (LPCWSTR)L"Stream Viewer",
                MB_ICONWARNING | MB_OK
            );
            return 0;
        }
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

//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//  目的: メイン ウィンドウのメッセージを処理します。
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //std::vector<std::string> urls;

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
    
    case WM_KEYDOWN:
        if (wParam == VK_SPACE)
        {
            //MessageBox(hWnd, L"スペースキーが押されました", L"キーイベント", MB_OK);
            _next_source = true;
        }
        break;


    case WM_PAINT:
    {
        if (wm_paint_now)
            break;
        else
        {   
            wm_paint_now = true;

            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd, &ps);
            AllPaintBlack(hWnd, hDC);
            EndPaint(hWnd, &ps);

            //描画スレッドがあった場合はクリア
            if (_main_th != nullptr)
            {
                set_cvw_stop(true);
                svw_wait_stop();
                _main_th->join();
                delete _main_th;
                _main_th = nullptr;
            }

            //描画スレッドが雨場合は立ち上げ
            if (_main_th == nullptr)
            {
                set_cvw_stop(false);
                _main_th = new std::thread(&DrawCV2Window, hWnd, _pt_yod, cam_urls, 0, pAICSV);
            }

            wm_paint_now = false;
        }
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
