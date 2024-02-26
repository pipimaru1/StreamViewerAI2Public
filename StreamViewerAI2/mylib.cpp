////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�֗��֐�������
//�������炱���ɏW��

#include "stdafx.h"
#include "framework.h"
#include "logfile.h"
#include "mylib.h"

#include <cstdlib> // getenv�֐��̂��߂ɕK�v


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


//���Ԃ𕶎���ŏo��
std::string GetCurrentDateTimeString(int _mode)
{
    // ���݂̃��[�J���������擾
    SYSTEMTIME st;
    GetLocalTime(&st);

    // ������X�g���[�����g�p���ăt�H�[�}�b�g����
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

// ���ϐ����܂ރp�X��W�J����
bool GetExpandEnvironmentPath(const std::string& path, std::string& dest)
{
    if (path.empty()) 
    {
        LOGMSG2("path is empty", path);
        return false;
    }
    // �W�J��̕�����̒������擾 (�k���������܂�)
    DWORD requiredSize = ExpandEnvironmentStringsA(path.c_str(), nullptr, 0);
    if (requiredSize == 0) 
    {
        LOGMSG2("���ϐ��W�J��̃p�X�������[��(���ϐ����ԈႢ)", path);
        return false;
    }

    // �K�v�ȃT�C�Y��dest��resize (�k�����������O)
    dest.resize(requiredSize - 2);

    // ���ۂɊ��ϐ���W�J
    DWORD copiedSize = ExpandEnvironmentStringsA(path.c_str(), &dest.front(), requiredSize);
    if (copiedSize == 0 || copiedSize > requiredSize) {
        // �G���[����
        LOGMSG2("���ϐ��W�J�ُ�", path);
        return false;
    }
    // ����
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
    LOGMSG2("�^��t�@�C��", _oss.str());
    return _oss.str();
}

//�摜�ɘ^�撆�X�^���v��t����
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
//�摜�Ƀ^�C���X�^���v��t����
int  time_stump(cv::Mat& _image, float stump_size)
{
    if (_image.empty())
        return 0;
    if (_image.cols == 0)
        return 0;
    if (_image.rows == 0)
        return 0;

    //������ǉ�
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
    wstring��string�֕ϊ�
*/
std::string wstring2string(std::wstring oWString)
{
    // wstring �� SJIS
    int iBufferSize = WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, (char*)NULL, 0, NULL, NULL);
    // �o�b�t�@�̎擾
    CHAR* cpMultiByte = new CHAR[iBufferSize];
    // wstring �� SJIS
    WideCharToMultiByte(CP_OEMCP, 0, oWString.c_str(), -1, cpMultiByte, iBufferSize, NULL, NULL);
    // string�̐���
    std::string oRet(cpMultiByte, cpMultiByte + iBufferSize - 1);
    // �o�b�t�@�̔j��
    delete[] cpMultiByte;
    // �ϊ����ʂ�Ԃ�
    return(oRet);
}


#pragma warning(disable : 4996)
std::wstring test_stringToWstring(const std::string& str)
{
    if (str.empty())
        return std::wstring();
    size_t charsNeeded = mbstowcs(nullptr, str.c_str(), 0); // �K�v�ȃ��C�h�����̐����擾

    if (charsNeeded == static_cast<size_t>(-1))
        return std::wstring(); // �ϊ��G���[

    std::vector<wchar_t> buffer(charsNeeded + 1);
    mbstowcs(&buffer[0], str.c_str(), buffer.size());
    return std::wstring(&buffer[0], charsNeeded);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//�o����AHDC���₽��Q�b�g����Ƃ��������Ȃ�̂Ő������Ȃ�
int AllPaintBlack(HWND hWnd, HDC hDC, DrawArea drawing_position)
{
    //��ʂ������h��Ԃ�
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
// �o�b�t�@���̃t���[����ǂݎ̂ĂāA�ŐV�̃t���[�����擾����֐�
bool getLatestFrame(cv::VideoCapture& cap, cv::Mat& frame) {
    cv::Mat temp;
    bool isNewFrameAvailable = false;

    // �o�b�t�@���̃t���[�������ׂēǂݎ̂Ă�
    while (cap.read(temp)) {
        isNewFrameAvailable = true;
    }

    // �Ō�ɓǂݍ��񂾃t���[�����ŐV�̃t���[��
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

    //�s�����J�E���g
    ptrdiff_t lineCount = std::count(_text.begin(), _text.end(), '\n') + 1;
    int n = (rect.bottom - rect.top) / 8 / 2;

    //��ʂ��͂ݏo��悤��������A��������폜
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
        // �ŏ���n�s���폜
        _text_tmp = _text.substr(pos);

        AllPaintBlack(hWnd, hDC, DrawArea(1, 1, 0, 0));
    }
    else
        _text_tmp = _text;

    //���Ǒ������Ȃ�_text_tmp�v��Ȃ������ˁB
    _text = _text_tmp;

    HFONT hFont = CreateFont(
        -MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72),    // �t�H���g�̍����i�s�N�Z���P�ʁj
        0,                                                  // ���ϕ�����
        0,                                                  // ��]�p�x
        0,                                                  // �x�[�X���C����x���Ƃ̊p�x
        FW_BOLD,                                             // �t�H���g�̏d���i�����j
        FALSE,                                               // �C�^���b�N
        FALSE,                                               // ����
        FALSE,                                               // ��������
        DEFAULT_CHARSET,                                     // �����Z�b�g
        OUT_DEFAULT_PRECIS,                                  // �o�͐��x
        CLIP_DEFAULT_PRECIS,                                 // �N���b�s���O���x
        DEFAULT_QUALITY,                                     // �o�͕i��
        DEFAULT_PITCH | FF_SWISS,                            // �s�b�`�ƃt�@�~���[
        _T("Meiryo UI")                                      // �t�H���g��
    );
    HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

    SetBkMode(hDC, TRANSPARENT); // �e�L�X�g�̔w�i�𓧖��ɂ��܂��B
    SetTextColor(hDC, RGB(255, 255, 255)); // �e�L�X�g�̐F�𔒂ɂ��܂��B

    std::wstring _ostW = stringToWstring(_text_tmp);
    DrawText(hDC, _ostW.c_str(), -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);

    // �g�p���I�������A�I�u�W�F�N�g��I���������A�폜���܂�
    SelectObject(hDC, hOldFont);
    DeleteObject(hFont);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//����ۑ��N���X
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
    fps = 30; //���悩��fps���擾
    width = 720; //���悩�畝���擾
    height = 480; //���悩�獂�����擾
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
// n:���̕����� m:�c�̕����� i:����̏��� 0���ŏ� j:�c��̏��� 
//RECT RectSplit(RECT base, int n, int m, int i, int j)
RECT RectSplit(RECT base, DrawArea _drawarea)
{
    int W = base.right - base.left;
    int H = base.bottom - base.top;

    //int len_W = (int)((float)W) / ((float)_drawarea.spl_w);
    //int len_H = (int)((float)H) / ((float)_drawarea.spl_h);

    float len_W = ((float)W) / ((float)_drawarea.spl_w);
    float len_H = ((float)H) / ((float)_drawarea.spl_h);


    RECT rect_out;//�ϐ��̎����ɒ���

    rect_out.left = (long)(_drawarea.num_w * len_W);
    rect_out.top = (long)(_drawarea.num_h * len_H);

    rect_out.right = rect_out.left + (long)len_W;
    rect_out.bottom = rect_out.top + (long)len_H;

    return rect_out;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//Windows�̃R���g���[���ɏ������ފ֐�
//volatile bool DrawPicToHDC_flag = false;
std::mutex Mutex_DrawCycle_Draw;
int DrawPicToHDC(cv::Mat cvImg, HWND hWnd, HDC hDC, bool bMaintainAspectRatio, DrawArea _drawarea, bool _gray) //bMaintainAspectRatio=true
{

    if (cvImg.rows == 0 || cvImg.cols == 0) //�摜�̑傫����0�������牽�����Ȃ�
    {
        return 0;
    }

    //�I���W�i���ł�HDC�𐶐����Ă������O����
    CRect rect;

    //Window�̃T�C�Y
    //�A�C�R����ԂȂ疳������
    bool ret = GetClientRect(hWnd, rect);
    if (ret != true)
        return 0;
    if (rect.Height() == 0 || rect.Width() == 0)
        return 0;

    cv::Size winSize(rect.right, rect.bottom);

    //�\������摜�̃T�C�Y
    cv::Size origImageSize(cvImg.cols, cvImg.rows);
    cv::Size imageSize;
    int  offsetX = 0;
    int  offsetY = 0;

    RECT base = rect;

    if (!bMaintainAspectRatio)//�A�X�y�N�g��𖳎�
    {
        // Image should be the same size as the control's rectangle
        imageSize = winSize;
    }
    else //�A�X�y�N�g����ێ�
    {
        cv::Size newSize;
        AdjustAspectImageSize(origImageSize, winSize, imageSize);
    }

    //���S�Ɋ񂹂�
    if (_drawarea.spl_w == 1 && _drawarea.spl_h == 1)
    {
        offsetX = (winSize.width - imageSize.width) / 2;
        offsetY = (winSize.height - imageSize.height) / 2;
    }

    //4����
    else
    {
        //�`��G���A�𕪊�����
        RECT rect00;
        rect00 = RectSplit(base, _drawarea);
        offsetX = rect00.left;
        offsetY = rect00.top;
        imageSize.width = imageSize.width / _drawarea.spl_w;//�Ƃ肠����
        imageSize.height = imageSize.height / _drawarea.spl_h;//�Ƃ肠����
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

        //�r������ ChatGPT�ɂ��ƕK�v
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
    // �V�X�e����̂��ׂẴ��j�^�[���
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

    // 2�Ԗڂ̃��j�^�[��I���i���݂���ꍇ�j
    if (monitors.size() > monitor)
    {
        isFullscreen = true;
        // ���̃E�B���h�E�T�C�Y�ƈʒu��ۑ�
        GetWindowRect(hWnd, &windowRect);
        //hMenu = GetMenu(hWnd);
        //hMenu_for_fullscreen = GetMenu(hWnd);
        //SetMenu(hWnd, hMenu_for_fullscreen);

        // 2�Ԗڂ̃��j�^�[�̃X�N���[�����W���擾
        RECT monitor_rect = monitors[monitor];

        // ���݂̃��j���[�o�[���擾���A�ۑ�
        hMenu_for_fullscreen = GetMenu(hWnd);

        // ���j���[�o�[������
        SetMenu(hWnd, NULL);

        // �E�B���h�E��2�Ԗڂ̃��j�^�[�Ɉړ����đS��ʕ\���ɂ���
        SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);

        SetWindowPos(hWnd, HWND_TOP, monitor_rect.left, monitor_rect.top, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);

        // �E�B���h�E��\��
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
        // ���݂̃E�B���h�E�̈ʒu�ƃT�C�Y��ۑ�
        GetWindowPlacement(hWnd, &wp);

        // ���݂̃��j���[�o�[���擾���A�ۑ�
        hMenu_for_fullscreen = GetMenu(hWnd);

        //GetWindowRect(hwnd, &rcSaved);
        GetWindowRect(hWnd, &windowRect);

        // ���j�^�[�����擾
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);

        // �E�B���h�E�X�^�C����ύX���đS��ʕ\��
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
    //    // �E�B���h�E�X�^�C�������ɖ߂�
    //    SetWindowLong(hWnd, GWL_STYLE, savedStyle);
    //    SetWindowLong(hWnd, GWL_EXSTYLE, savedExStyle);
    //    SetWindowPlacement(hWnd, &wp);
    //    SetWindowPos(hWnd, NULL, rcSaved.left, rcSaved.top, rcSaved.right - rcSaved.left, rcSaved.bottom - rcSaved.top,
    //        SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    //}
}

////////////////////////////////////////////////////////////////////////////////////////////////
//�J�����̃I�[�v��
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
    // �^�C���A�E�g��5�b�Ƃ���
    for (i = 0; i < max; ++i)
    {
        if (_connected)
        {
            break; // �ڑ�����
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(CAPOPEN_TIMEOUT_INTERCAL));
    }
    //break�͂����ɏo��͂�
    if (!_connected)
    {
        LOGMSG2("�ڑ��^�C���A�E�g",(i* CAPOPEN_TIMEOUT_INTERCAL));
        _pt_capture->release();
        // �K�v�ɉ����ċ����I������
        // connectionThread�����S�ɏI�������鏈���������ɋL�q
        ret = 0;
    }
    else
        ret = 1; //��肭������1��Ԃ�

    if (_th.joinable())
    {
        _th.join();
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//�t�@�C����ǂݍ��݁Astring�̔z��Ɋi�[
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
                //// �s�v�ȃk������������
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
