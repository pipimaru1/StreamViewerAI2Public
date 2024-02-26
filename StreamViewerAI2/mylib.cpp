////////////////////////////////////////////////////////////////////////////////////////////////////////////
//便利関数を入れる
//見つけたらここに集約

#include "stdafx.h"
#include "framework.h"
#include "logfile.h"
#include "mylib.h"

#include <cstdlib> // getenv関数のために必要


//LPCWSTR _A2CW(const std::string& ascii)
std::wstring _A2CW(const std::string& ascii)
{
    int len = MultiByteToWideChar(CP_ACP, 0, ascii.c_str(), -1, NULL, 0);
    if (len == 0) {
        // Handle the error
        return L"";
    }

    std::wstring wide(len, L'\0');
    if (!MultiByteToWideChar(CP_ACP, 0, ascii.c_str(), -1, &wide[0], len)) {
        // Handle the error
        return L"";
    }

    return wide.c_str();
}


//時間を文字列で出力
std::string GetCurrentDateTimeString(int _mode)
{
    // 現在のローカル日時を取得
    SYSTEMTIME st;
    GetLocalTime(&st);

    // 文字列ストリームを使用してフォーマットする
    std::ostringstream oss;
    if (_mode == GETDTSTR_FMT1)
    {
        oss << std::setw(4) << st.wYear << "/";
        oss << std::setw(2) << std::setfill('0') << st.wMonth << "/";
        oss << std::setw(2) << std::setfill('0') << st.wDay << " ";
        oss << std::setw(2) << std::setfill('0') << st.wHour << ":";
        oss << std::setw(2) << std::setfill('0') << st.wMinute << ":";
        oss << std::setw(2) << std::setfill('0') << st.wSecond;
    }
    else
    {
        oss << std::setw(4) << st.wYear;
        oss << std::setw(2) << std::setfill('0') << st.wMonth;
        oss << std::setw(2) << std::setfill('0') << st.wDay;
        oss << std::setw(2) << std::setfill('0') << st.wHour;
        oss << std::setw(2) << std::setfill('0') << st.wMinute;
        oss << std::setw(2) << std::setfill('0') << st.wSecond;
    }
    return oss.str();
}

// 環境変数を含むパスを展開する
bool GetExpandEnvironmentPath(const std::string& path, std::string& dest)
{
    if (path.empty()) 
    {
        LOGMSG2("path is empty", path);
        return false;
    }
    // 展開後の文字列の長さを取得 (ヌル文字を含む)
    DWORD requiredSize = ExpandEnvironmentStringsA(path.c_str(), nullptr, 0);
    if (requiredSize == 0) 
    {
        LOGMSG2("環境変数展開後のパス長さがゼロ(環境変数名間違い)", path);
        return false;
    }

    // 必要なサイズでdestをresize (ヌル文字を除外)
    dest.resize(requiredSize - 2);

    // 実際に環境変数を展開
    DWORD copiedSize = ExpandEnvironmentStringsA(path.c_str(), &dest.front(), requiredSize);
    if (copiedSize == 0 || copiedSize > requiredSize) {
        // エラー処理
        LOGMSG2("環境変数展開異常", path);
        return false;
    }
    // 成功
    return true;
}
std::string add_dt_ext(std::string _base_str, std::string _ext)
{
    std::string _dest;
    GetExpandEnvironmentPath(_base_str, _dest);
  
    std::ostringstream _oss;
    _oss << _dest;
    _oss << "_";
    _oss << GetCurrentDateTimeString();
    _oss << _ext;
    LOGMSG2("録画ファイル", _oss.str());
    return _oss.str();
}

//画像に録画中スタンプを付ける
int  rec_stump(cv::Mat& _image, bool rec, float stump_size)
{
     if (_image.empty())
        return 0;
    if (_image.cols==0)
        return 0;
    if (_image.rows == 0)
        return 0;

    if (rec)
    {
        //cv::Size(_image);
        int X1 = (int)(_image.cols - 80* stump_size);
        int Y1 = (int)(30* stump_size);

        _TRYCAT_CV(cv::putText(_image, "REC", cv::Point(X1, Y1), cv::FONT_HERSHEY_SIMPLEX, stump_size, cv::Scalar(0, 0, 255), int(4 * stump_size)));
    }
    return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//画像にタイムスタンプを付ける
int  time_stump(cv::Mat& _image, float stump_size)
{
    if (_image.empty())
        return 0;
    if (_image.cols == 0)
        return 0;
    if (_image.rows == 0)
        return 0;

    //文字列追加
    std::ostringstream _ost;
    _ost << GetCurrentDateTimeString(GETDTSTR_FMT1);// << std::ends;
    long _error = 0;
    cv::putText(_image, _ost.str(), cv::Point(5, 25), cv::FONT_HERSHEY_SIMPLEX, stump_size, cv::Scalar(0, 0, 0), 5);
    cv::putText(_image, _ost.str(), cv::Point(5, 25), cv::FONT_HERSHEY_SIMPLEX, stump_size, cv::Scalar(0, 255, 0), 2);
    return _error;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void DoEvents(int n)
{
    MSG msg;
    for (int i = 0; i < n; i++)
    {
        while (::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::wstring stringToWstring(const std::string& s)
{
    //int requiredSize = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    int requiredSize = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, NULL, 0);
    std::wstring result(requiredSize - 1, 0);
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, &result[0], requiredSize);
    //MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &result[0], requiredSize);

    return result;
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


#pragma warning(disable : 4996)
std::wstring test_stringToWstring(const std::string& str)
{
    if (str.empty())
        return std::wstring();
    size_t charsNeeded = mbstowcs(nullptr, str.c_str(), 0); // 必要なワイド文字の数を取得

    if (charsNeeded == static_cast<size_t>(-1))
        return std::wstring(); // 変換エラー

    std::vector<wchar_t> buffer(charsNeeded + 1);
    mbstowcs(&buffer[0], str.c_str(), buffer.size());
    return std::wstring(&buffer[0], charsNeeded);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//経験上、HDCをやたらゲットするとおかしくなるので生成しない
int AllPaintBlack(HWND hWnd, HDC hDC, DrawArea drawing_position)
{
    //画面を黒く塗りつぶす
    RECT _rect0;
    RECT _rect1;
    GetClientRect(hWnd, &_rect0);

    _rect1 = RectSplit(_rect0, drawing_position);

    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hDC, &_rect1, hBrush);
    DeleteObject(hBrush);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// バッファ内のフレームを読み捨てて、最新のフレームを取得する関数
bool getLatestFrame(cv::VideoCapture& cap, cv::Mat& frame) {
    cv::Mat temp;
    bool isNewFrameAvailable = false;

    // バッファ内のフレームをすべて読み捨てる
    while (cap.read(temp)) {
        isNewFrameAvailable = true;
    }

    // 最後に読み込んだフレームが最新のフレーム
    if (isNewFrameAvailable) {
        temp.copyTo(frame);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int MyDrawText(HWND hWnd, HDC hDC, std::string& _text, int x0 , int y0 , bool _add )
{
    //HDC hDC = ::GetDC(hWnd);
    RECT rect;
    GetClientRect(hWnd, &rect);
    rect.left = x0;
    rect.top = y0;

    std::string _text_tmp;

    //行数をカウント
    ptrdiff_t lineCount = std::count(_text.begin(), _text.end(), '\n') + 1;
    int n = (rect.bottom - rect.top) / 8 / 2;

    //画面をはみ出るようだったら、文字列を削除
    if (lineCount > n)
    {
        size_t pos = 0;
        for (int i = 0; i < n; ++i) {
            pos = _text.find('\n', pos);
            if (pos == std::string::npos) {
                break;
            }
            ++pos;
        }
        // 最初のn行を削除
        _text_tmp = _text.substr(pos);

        AllPaintBlack(hWnd, hDC, DrawArea(1, 1, 0, 0));
    }
    else
        _text_tmp = _text;

    //結局代入するなら_text_tmp要らなかったね。
    _text = _text_tmp;

    HFONT hFont = CreateFont(
        -MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72),    // フォントの高さ（ピクセル単位）
        0,                                                  // 平均文字幅
        0,                                                  // 回転角度
        0,                                                  // ベースラインとx軸との角度
        FW_BOLD,                                             // フォントの重さ（太さ）
        FALSE,                                               // イタリック
        FALSE,                                               // 下線
        FALSE,                                               // 取り消し線
        DEFAULT_CHARSET,                                     // 文字セット
        OUT_DEFAULT_PRECIS,                                  // 出力精度
        CLIP_DEFAULT_PRECIS,                                 // クリッピング精度
        DEFAULT_QUALITY,                                     // 出力品質
        DEFAULT_PITCH | FF_SWISS,                            // ピッチとファミリー
        _T("Meiryo UI")                                      // フォント名
    );
    HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

    SetBkMode(hDC, TRANSPARENT); // テキストの背景を透明にします。
    SetTextColor(hDC, RGB(255, 255, 255)); // テキストの色を白にします。

    std::wstring _ostW = stringToWstring(_text_tmp);
    DrawText(hDC, _ostW.c_str(), -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);

    // 使用が終わった後、オブジェクトを選択解除し、削除します
    SelectObject(hDC, hOldFont);
    DeleteObject(hFont);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//動画保存クラス
/////////////////////////////////////////////////////////////////////////////////////////////
my_video_writer::my_video_writer(const char* _fname)
:my_video_writer()
{
//    my_video_writer::my_video_writer();
    filename = _fname;
}
my_video_writer::my_video_writer()
{
    fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    //fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    //fourcc = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');
    fps = 30; //動画からfpsを取得
    width = 720; //動画から幅を取得
    height = 480; //動画から高さを取得
    filename = "output.mp4";
}

int my_video_writer::open(const char* _fname)
{
    filename = _fname;
    return open();
}

int my_video_writer::open()
{
    if (output.isOpened() == false)
    {
        output.open(filename.c_str(), fourcc, fps, cv::Size(width, height));
    }
    return 0;
}

int my_video_writer::write(cv::Mat& frame)
{
    cv::Mat frame_resized;

    if (frame.empty())
        return 0;
    if (output.isOpened() == false)
        return 0;
    if (frame.cols == 0 || frame.rows == 0)
        return 0;
    if (width < 1 || height < 1)
        return 0;

    if (mtx.try_lock())
    {
        //LOGMSG2(width,"width");
        //LOGMSG2(height, "height");
        //LOGMSG2(frame.cols, "frame.cols");
        //LOGMSG2(frame.rows, "frame.rows");

        cv::resize(frame, frame_resized, cv::Size(), ((double)width / (double)frame.cols), ((double)height / (double)frame.rows));
        TRYCAT_CV(output.write(frame_resized));
        output.write(frame_resized);
        //output << frame_resized;
        mtx.unlock();
    }
    return 0;
}
int my_video_writer::release()
{
    while (mtx.try_lock() == false)
    {
        //DoEvents();
        Sleep(5);
    }

    //TRYCAT_CV(output.release());
    output.release();

    mtx.unlock();

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void AdjustAspectImageSize(const cv::Size& imageSize, const cv::Size& destSize, cv::Size& newSize)
{
    double destAspectRatio = float(destSize.width) / float(destSize.height);
    double imageAspectRatio = float(imageSize.width) / float(imageSize.height);

    if (imageAspectRatio > destAspectRatio)
    {
        // Margins on top/bottom
        newSize.width = destSize.width;
        newSize.height = int(imageSize.height *
            (double(destSize.width) / double(imageSize.width)));
    }
    else
    {
        // Margins on left/right
        newSize.height = destSize.height;
        newSize.width = int(imageSize.width *
            (double(destSize.height) / double(imageSize.height)));
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// n:横の分割数 m:縦の分割数 i:横列の順番 0が最初 j:縦列の順番 
//RECT RectSplit(RECT base, int n, int m, int i, int j)
RECT RectSplit(RECT base, DrawArea _drawarea)
{
    int W = base.right - base.left;
    int H = base.bottom - base.top;

    //int len_W = (int)((float)W) / ((float)_drawarea.spl_w);
    //int len_H = (int)((float)H) / ((float)_drawarea.spl_h);

    float len_W = ((float)W) / ((float)_drawarea.spl_w);
    float len_H = ((float)H) / ((float)_drawarea.spl_h);


    RECT rect_out;//変数の寿命に注意

    rect_out.left = (long)(_drawarea.num_w * len_W);
    rect_out.top = (long)(_drawarea.num_h * len_H);

    rect_out.right = rect_out.left + (long)len_W;
    rect_out.bottom = rect_out.top + (long)len_H;

    return rect_out;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//Windowsのコントロールに書き込む関数
//volatile bool DrawPicToHDC_flag = false;
std::mutex Mutex_DrawCycle_Draw;
int DrawPicToHDC(cv::Mat cvImg, HWND hWnd, HDC hDC, bool bMaintainAspectRatio, DrawArea _drawarea, bool _gray) //bMaintainAspectRatio=true
{

    if (cvImg.rows == 0 || cvImg.cols == 0) //画像の大きさが0だったら何もしない
    {
        return 0;
    }

    //オリジナルではHDCを生成していたが外した
    CRect rect;

    //Windowのサイズ
    //アイコン状態なら無視する
    bool ret = GetClientRect(hWnd, rect);
    if (ret != true)
        return 0;
    if (rect.Height() == 0 || rect.Width() == 0)
        return 0;

    cv::Size winSize(rect.right, rect.bottom);

    //表示する画像のサイズ
    cv::Size origImageSize(cvImg.cols, cvImg.rows);
    cv::Size imageSize;
    int  offsetX = 0;
    int  offsetY = 0;

    RECT base = rect;

    if (!bMaintainAspectRatio)//アスペクト比を無視
    {
        // Image should be the same size as the control's rectangle
        imageSize = winSize;
    }
    else //アスペクト比を維持
    {
        cv::Size newSize;
        AdjustAspectImageSize(origImageSize, winSize, imageSize);
    }

    //中心に寄せる
    if (_drawarea.spl_w == 1 && _drawarea.spl_h == 1)
    {
        offsetX = (winSize.width - imageSize.width) / 2;
        offsetY = (winSize.height - imageSize.height) / 2;
    }

    //4分割
    else
    {
        //描画エリアを分割する
        RECT rect00;
        rect00 = RectSplit(base, _drawarea);
        offsetX = rect00.left;
        offsetY = rect00.top;
        imageSize.width = imageSize.width / _drawarea.spl_w;//とりあえず
        imageSize.height = imageSize.height / _drawarea.spl_h;//とりあえず
    }
    // Resize the source to the size of the destination image if necessary

    cv::Mat cvImgTmp;
    cv::resize(cvImg, cvImgTmp, imageSize, 0, 0, cv::INTER_AREA);


    // To handle our Mat object of this width, the source rows must
    // be even multiples of a DWORD in length to be compatible with 
    // SetDIBits().  Calculate what the correct byte width of the 
    // row should be to be compatible with SetDIBits() below.
    int stride = ((((imageSize.width * 24) + 31) & ~31) >> 3);

    // Allocate a buffer for our DIB bits
    uchar* pcDibBits = (uchar*)malloc(imageSize.height * stride);

    if (pcDibBits != NULL)
    {
        // Copy the raw pixel data over to our dibBits buffer.
        // NOTE: Can setup cvImgTmp to add the padding to skip this.
        for (int row = 0; row < cvImgTmp.rows; ++row)
            //for (int row = 0; row < pt_cvImgTmp->rows; ++row)
        {
            // Get pointers to the beginning of the row on both buffers
            uchar* pcSrcPixel = cvImgTmp.ptr<uchar>(row);
            //uchar* pcSrcPixel = pt_cvImgTmp->ptr<uchar>(row);
            uchar* pcDstPixel = pcDibBits + (row * stride);

            // We can just use memcpy
            memcpy(pcDstPixel,
                pcSrcPixel,
                stride);
        }
        // Initialize the BITMAPINFO structure
        BITMAPINFO bitInfo;
        if (_gray)
            bitInfo.bmiHeader.biBitCount = 8;
        else
            bitInfo.bmiHeader.biBitCount = 24;
        bitInfo.bmiHeader.biWidth = cvImgTmp.cols;
        bitInfo.bmiHeader.biHeight = -cvImgTmp.rows;
        //bitInfo.bmiHeader.biWidth = pt_cvImgTmp->cols;
        //bitInfo.bmiHeader.biHeight = -pt_cvImgTmp->rows;
        bitInfo.bmiHeader.biPlanes = 1;
        bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitInfo.bmiHeader.biCompression = BI_RGB;
        bitInfo.bmiHeader.biClrImportant = 0;
        bitInfo.bmiHeader.biClrUsed = 0;
        bitInfo.bmiHeader.biSizeImage = 0;      //winSize.height * winSize.width * * 3;
        bitInfo.bmiHeader.biXPelsPerMeter = 0;
        bitInfo.bmiHeader.biYPelsPerMeter = 0;

        //排他処理 ChatGPTによると必要
        while (Mutex_DrawCycle_Draw.try_lock() == false)
        {
            DoEvents();
            Sleep(5);
        }
        {
            //PAINTSTRUCT ps;
            //HDC _hDC = BeginPaint(hWnd, &ps);
            HDC _hDC = GetDC(hWnd);
            if (_hDC != NULL)
            {
                // Add header and OPENCV image's data to the HDC
                StretchDIBits(_hDC,
                    offsetX,
                    offsetY,
                    cvImgTmp.cols,
                    cvImgTmp.rows,
                    //pt_cvImgTmp->cols,
                    //pt_cvImgTmp->rows,
                    0,
                    0,
                    cvImgTmp.cols,
                    cvImgTmp.rows,
                    //pt_cvImgTmp->cols,
                    //pt_cvImgTmp->rows,
                    pcDibBits,
                    &bitInfo,
                    DIB_RGB_COLORS,
                    SRCCOPY);

                free(pcDibBits);
            }
            else
            {
                LOGMSG2X("_hDC is Null", _hDC);
            }
            //EndPaint(hWnd, &ps);
            if(_hDC!=NULL)
                ReleaseDC(hWnd, _hDC);
        }
        Mutex_DrawCycle_Draw.unlock();

    }
    return 0;
}

HMENU hMenu_for_fullscreen;
RECT windowRect;
bool isFullscreen=false;
static std::vector<RECT> monitors;
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    monitors.push_back(*lprcMonitor);
    return TRUE;
}

int set_fullscreen(HWND hWnd, int monitor)
{
    //HMENU hMenu;
    // システム上のすべてのモニターを列挙
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

    // 2番目のモニターを選択（存在する場合）
    if (monitors.size() > monitor)
    {
        isFullscreen = true;
        // 元のウィンドウサイズと位置を保存
        GetWindowRect(hWnd, &windowRect);
        //hMenu = GetMenu(hWnd);
        //hMenu_for_fullscreen = GetMenu(hWnd);
        //SetMenu(hWnd, hMenu_for_fullscreen);

        // 2番目のモニターのスクリーン座標を取得
        RECT monitor_rect = monitors[monitor];

        // 現在のメニューバーを取得し、保存
        hMenu_for_fullscreen = GetMenu(hWnd);

        // メニューバーを消す
        SetMenu(hWnd, NULL);

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

void ResumeWindow(HWND hWnd)
{
    SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
    SetWindowPos(hWnd, HWND_TOP, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_FRAMECHANGED);

    //hMenu_for_fullscreen = GetMenu(hWnd);
    SetMenu(hWnd, hMenu_for_fullscreen);
    ShowWindow(hWnd, SW_SHOW);

    InvalidateRect(hWnd, NULL, TRUE);
}

void ToggleFullscreenWithMenu(HWND hWnd) 
{
    static WINDOWPLACEMENT wp = { sizeof(wp) };
    static DWORD savedStyle = 0;
    static DWORD savedExStyle = 0;
    //static RECT rcSaved = {};

    DWORD style = GetWindowLong(hWnd, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW) 
    {
        // 現在のウィンドウの位置とサイズを保存
        GetWindowPlacement(hWnd, &wp);

        // 現在のメニューバーを取得し、保存
        hMenu_for_fullscreen = GetMenu(hWnd);

        //GetWindowRect(hwnd, &rcSaved);
        GetWindowRect(hWnd, &windowRect);

        // モニター情報を取得
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);

        // ウィンドウスタイルを変更して全画面表示
        savedStyle = style;
        savedExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
        SetWindowLong(hWnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
        SetWindowLong(hWnd, GWL_EXSTYLE, savedExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
        SetWindowPos(hWnd, HWND_TOP,
            mi.rcMonitor.left, mi.rcMonitor.top,
            mi.rcMonitor.right - mi.rcMonitor.left,
            mi.rcMonitor.bottom - mi.rcMonitor.top,
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
    //else 
    //{
    //    // ウィンドウスタイルを元に戻す
    //    SetWindowLong(hWnd, GWL_STYLE, savedStyle);
    //    SetWindowLong(hWnd, GWL_EXSTYLE, savedExStyle);
    //    SetWindowPlacement(hWnd, &wp);
    //    SetWindowPos(hWnd, NULL, rcSaved.left, rcSaved.top, rcSaved.right - rcSaved.left, rcSaved.bottom - rcSaved.top,
    //        SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    //}
}

////////////////////////////////////////////////////////////////////////////////////////////////
//カメラのオープン
//std::atomic<bool> connected[100];
int CAPOPEN_TIMEOUT = 1000;
int CAPOPEN_TIMEOUT_INTERCAL = 10;

void tryConnect_url(const std::string& _url, cv::VideoCapture* _pt_capture, std::atomic<bool>& connected)
{
    _pt_capture->open(_url);
    if (_pt_capture->isOpened())
        connected = true;
}
void tryConnect_usb(int _usb, cv::VideoCapture* _pt_capture, std::atomic<bool>& connected)
{
    _pt_capture->open(_usb);
    if (_pt_capture->isOpened())
        connected = true;
}

int catchConnect(cv::VideoCapture* _pt_capture, std::atomic<bool>& _connected, std::thread& _th)
{
    int ret = 0;
    int max = CAPOPEN_TIMEOUT / CAPOPEN_TIMEOUT_INTERCAL;
    int i = 0;
    // タイムアウトを5秒とする
    for (i = 0; i < max; ++i)
    {
        if (_connected)
        {
            break; // 接続成功
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(CAPOPEN_TIMEOUT_INTERCAL));
    }
    //breakはここに出るはず
    if (!_connected)
    {
        LOGMSG2("接続タイムアウト",(i* CAPOPEN_TIMEOUT_INTERCAL));
        _pt_capture->release();
        // 必要に応じて強制終了処理
        // connectionThreadを安全に終了させる処理をここに記述
        ret = 0;
    }
    else
        ret = 1; //上手くいくと1を返す

    if (_th.joinable())
    {
        _th.join();
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//ファイルを読み込み、stringの配列に格納
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

                //item.erase(item.find_last_not_of(" \t\n\r\f\v") + 1);
                //item.erase(0, item.find_first_not_of(" \t\n\r\f\v"));
                //// 不要なヌル文字を除去
                //item.erase(std::remove(item.begin(), item.end(), '\0'), item.end());
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
