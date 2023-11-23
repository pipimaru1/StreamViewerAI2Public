#include "stdafx.h"

#include "yolov5_engine.h"
#include "logfile.h"

using namespace cv;
using namespace std;
using namespace cv::dnn;

//これ試す
//https://learnopencv.com/object-detection-using-yolov5-and-opencv-dnn-in-c-and-python/
//https://github.com/spmallick/learnopencv/tree/master/Object-Detection-using-YOLOv5-and-OpenCV-DNN-in-CPP-and-Python
// 
// Constants.

// Colors.
Scalar BLACK = Scalar(0, 0, 0);
Scalar BLUE = Scalar(255, 178, 50);
Scalar KOIBLUE = Scalar(255, 32, 32);
Scalar YELLOW = Scalar(0, 255, 255);
Scalar RED = Scalar(0, 0, 255);
Scalar GREY = Scalar(64, 64, 64);
Scalar DARKGREEN = Scalar(32, 200, 32);

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
#define TRYCAT_CV(tryal_codes) try\
{\
	tryal_codes;\
}\
catch (cv::Exception e){\
	const char* err_msg = e.what();\
	cerr << "RUNTIME ERROR IN CV:"<<__FILE__<<":"<<__LINE__<<"\n" << err_msg << std::endl;\
}

#define TRYCAT_CV_STD(tryal_codes) try\
{\
	tryal_codes;\
}\
catch (cv::Exception e){\
	const char* err_msg = e.what();\
	cerr << "RUNTIME ERROR IN CV:"<<__FILE__<<":"<<__LINE__<<"\n" << err_msg << std::endl;\
}\
catch (std::exception e) {\
    const char* err_msg = e.what(); \
    cerr << "RUNTIME ERROR IN CV:" << __FILE__ << ":" << __LINE__ << "\n" << err_msg << std::endl; \
}
//try catchを無効にするマクロ
#define _TRYCAT_CV_STD(tryal_codes)  tryal_codes;
#define _TRYCAT_CV(tryal_codes)  tryal_codes;
#define _TRYCAT_STD(tryal_codes)  tryal_codes;

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
YoloAIParametors::YoloAIParametors()
{
    _input_width = DEFAULT_AI_INPUT_WIDTH;
    _input_height = DEFAULT_AI_INPUT_HEIGHT;
    _score_threshold = DEFAULT_SCORE_THRESHOLD;
    _nms_threshold = DEFAULT_NMS_THRESHOLD;
    _confidence_thresgold = DEFAULT_CONF_THRESHOLD;

    _clssification_size=0;
    _onnx_file_name = std::string();
    _names_file_name = std::string();
}

YoloAIParametors::~YoloAIParametors()
{
}

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
bool YoloObjectDetection::busy()
{
    return _busy;
}

bool YoloObjectDetection::busy_set()
{
    return _busy=true;
}

bool YoloObjectDetection::busy_relese()
{
    return _busy = false;
}

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
#define LABEL_IN_BOX false
#define LABEL_OUT_BOX true
//void draw_label(Mat& input_image, string label, int left, int top, float _font_scale, int _thickness_font, int _fontface, cv::Scalar _rect_color)
void draw_label(Mat& input_image, string label, int left, int top, CvFontParam cfp, cv::Scalar _rect_color)
{
    // Display the label at the top of the bounding box.
    int baseLine;
    Size label_size = getTextSize(label, cfp._face, cfp._scale, cfp._thickness, &baseLine);
    top = max(top, label_size.height);
    if (0) //参考元にあったコード? 使っていない
    {
        if (LABEL_IN_BOX)
        {
            // Top left corner.
            Point tlc = Point(left, top);
            // Bottom right corner.
            Point brc = Point(left + label_size.width, top + label_size.height + baseLine);
            // Draw white rectangle.
            //rectangle(input_image, tlc, brc, BLACK, FILLED);
            // Put the label on the black rectangle.
        }
        if (LABEL_OUT_BOX)
        {
            Point tlc = Point(left, top - label_size.height - baseLine);
            Point brc = Point(left + label_size.width, top);
        }

        //↓??
        if (0)
        {
            putText(input_image, label, Point(left, top + label_size.height), cfp._face, cfp._scale,  BLACK, cfp._thickness + 1);
            putText(input_image, label, Point(left, top + label_size.height), cfp._face, cfp._scale, YELLOW, cfp._thickness);
        }
    }

    if (LABEL_IN_BOX)
    {
        rectangle(input_image, Point(left, top), Point(left + 105, top + 11), _rect_color, -1);
        putText(input_image, label, Point(left, top + label_size.height), cfp._face, cfp._scale, BLACK, cfp._thickness);
        //putText(input_image, label, Point(left, top + label_size.height), _fontface, _font_scale, YELLOW, _thickness_font);
    }
    if (LABEL_OUT_BOX)
    {
        rectangle(input_image, Point(left-1, top-11), Point(left + 105, top), _rect_color, -1);
        putText(input_image, label, Point(left, top -11 + label_size.height), cfp._face, cfp._scale, BLACK, cfp._thickness);
    }
}


//AIによる検知 アドレス値を受け取って格納する方法に変更
int pre_process(vector<Mat>& outputs,Mat& input_image, Net& net, float input_width= DEFAULT_AI_INPUT_WIDTH, float input_height= DEFAULT_AI_INPUT_HEIGHT)
{
    Mat blob;
    int ret = 0;
    
    //vector<Mat> outputs;
    
    //例外処理を入れてみる
    if (false)
    {
        //この関数の呼び出しもとでtryを入れている
        _TRYCAT_CV_STD(blobFromImage(input_image, blob, 1. / 255., Size(input_width, input_height), Scalar(), true, false));
        _TRYCAT_CV_STD(net.setInput(blob));
        // Forward propagate.
        _TRYCAT_CV_STD(net.forward(outputs, net.getUnconnectedOutLayersNames()));

    }
   //こっちNULLチェック どっちがいいのやら
    else 
    {
        if (input_image.data == NULL)
        {
            ret = 1;
            LOGMSG("input_image.data is null");
        }
        else if (input_image.empty())
        {
            ret = 2;
            LOGMSG("input_image.data is empty");
        }
        else
        {
            blobFromImage(input_image, blob, 1. / 255., Size(input_width, input_height), Scalar(), true, false);
            net.setInput(blob);
            // Forward propagate.
            //_LOGMSG2("outputs1", outputs);
            net.forward(outputs, net.getUnconnectedOutLayersNames());
            //_LOGMSG2("outputs2", outputs);
            ret = 0;
        }
    }    
    return ret;
}

//指示したオブジェクトだけを表示する場合はfalseにする
#define VIEW_ALL true 

//AIの結果をストリームに出力する
//描画機能は省いた 
//いずれpost_processと統合したほうがいい
//AIの結果を文字列として_ostに出力する
// 結果はカンマで区切るB
//_headerは項目の先頭につけるヘッダ　カメラ番号、フレーム番、時間など
//#define CLASSIFICATION_SIZE 11
Mat post_process_str(
    YoloAIParametors yp,
    YoloFontsParam yfp,
    bool draw_image,            //ふつうはture, falseにすると描画処理をしない。
    const Mat& input_image,     //このMatに上書きして返す
    vector<Mat>& outputs,       //aiの分類の情報
    const vector<string>& class_name,
    int& number_of_persons, std::vector<std::string>& class_list_view, 
    std::string _header,            //日付等
    std::string& _ost               //書き込んだ文字列を返す時の格納場所
)
{
    //何も検出していなければそのまま返す
    if (outputs.size() == 0)
        return input_image;

    cv::Mat output_image = input_image;

    // Initialize vectors to hold respective outputs while unwrapping     detections.
    vector<int> class_ids;
    vector<float> confidences;
    vector<Rect> boxes;
    // Resizing factor. 要素のリサイズ
    float x_factor = input_image.cols / yp._input_width;
    float y_factor = input_image.rows / yp._input_height;

    int rows = outputs[0].size[1];
    int dimensions = outputs[0].size[2];
    bool yolov8 = false;
    // yolov5 has an output of shape (batchSize, 25200, 85) (Num classes + box[x,y,w,h] + confidence[c])
    // yolov8 has an output of shape (batchSize, 84,  8400) (Num classes + box[x,y,w,h])
    if (dimensions > rows) // Check if the shape[2] is more than shape[1] (yolov8)
    {
        yolov8 = true;
        rows = outputs[0].size[2];
        dimensions = outputs[0].size[1];

        outputs[0] = outputs[0].reshape(1, dimensions);
        cv::transpose(outputs[0], outputs[0]);
    }
    float* data = (float*)outputs[0].data;
    // 25200 for default size 640.
    // Iterate through 25200 detections.
    for (volatile int i = 0; i < rows; ++i)
    {
        if (yolov8)
        {
            float* classes_scores = data + 4;

            cv::Mat scores(1, (int)class_name.size(), CV_32FC1, classes_scores);
            cv::Point class_id;
            double maxClassScore;

            minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);

            if (maxClassScore > yp._score_threshold)
            {
                confidences.push_back(maxClassScore);
                class_ids.push_back(class_id.x);

                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];

                int left = int((x - 0.5 * w) * x_factor);
                int top = int((y - 0.5 * h) * y_factor);

                int width = int(w * x_factor);
                int height = int(h * y_factor);

                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
        //yolov5
        else
        {
            float confidence = data[4];
            // Discard bad detections and continue.
            if (confidence >= yp._confidence_thresgold)
            {
                float* classes_scores = data + 5;
                // Create a 1x85 Mat and store class scores of 80 classes.
                Mat scores(1, (int)class_name.size(), CV_32FC1, classes_scores);
                // Perform minMaxLoc and acquire the index of best class  score.
                Point class_id;
                double max_class_score;
                minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
                // Continue if the class score is above the threshold.
                if (max_class_score > yp._score_threshold)
                {
                    // Store class ID and confidence in the pre-defined respective vectors.
                    confidences.push_back(confidence);
                    class_ids.push_back(class_id.x);
                    // Center.
                    float cx = data[0];
                    float cy = data[1];
                    // Box dimension.
                    float w = data[2];
                    float h = data[3];
                    // Bounding box coordinates.
                    int left = int((cx - 0.5 * w) * x_factor);
                    int top = int((cy - 0.5 * h) * y_factor);
                    int width = int(w * x_factor);
                    int height = int(h * y_factor);
                    // Store good detections in the boxes vector.
                    boxes.push_back(Rect(left, top, width, height));
                }
            }
        }
        data += dimensions;
    }

    ostringstream _os;
    Scalar RECTCOLOR;
    //囲う四角を描画
    vector<int> indices;
    //cudnnの関数
    cv::dnn::NMSBoxes(boxes, confidences, yp._score_threshold, yp._nms_threshold, indices);
    number_of_persons = 0;
    for (int i = 0; i < indices.size(); i++)
    {
        int idx = indices[i];
        Rect box = boxes[idx];
        int left    = box.x      ;
        int top     = box.y      ;
        int width   = box.width  ;
        int height  = box.height ;

        bool draw_rect = VIEW_ALL;

        //オブジェクト名の格納
        string object_name(class_name[class_ids[idx]]);

        int count_class_list_view = 0;
        //表示したいオブジェクト種類数だけ回す
        while (count_class_list_view < class_list_view.size())
        {
            //表示したいオブジェクトだったら表示フラグを立てる
            if (object_name.compare(class_list_view[count_class_list_view]) == 0)
                draw_rect = true;
            count_class_list_view++;
        }

        //人だったら数える
        if (object_name.compare("person") == 0 ||
            object_name.compare("driver") == 0)
        {
            number_of_persons++;
            RECTCOLOR = BLUE;
        }
        else if (object_name.compare("forklift") == 0)
        {
            RECTCOLOR = KOIBLUE;
        }
        else if (object_name.compare("excavator") == 0 ||
            object_name.compare("bulldozer") == 0 ||
            object_name.compare("wheelloder") == 0 ||
            object_name.compare("grader") == 0)
        {
            RECTCOLOR = YELLOW;
        }
        else
            RECTCOLOR = DARKGREEN;

        if (draw_image && draw_rect) //データ書き込みのみの場合は描画はしない
        {
            // Draw bounding box.
            rectangle(output_image, Point(left, top), Point(left + width, top + height), RECTCOLOR, yfp._thickness_box);
            // Get the label for the class name and its confidence.
            string label = format("%.2f", confidences[idx]);
            label = class_name[class_ids[idx]] + ":" + label;
            // Draw class labels.
            draw_label(output_image, label, left, top,yfp._label, RECTCOLOR);
        }

        
        //ストリームに解析した結果を保存
        _os << _header << ","
            << i << ","
            << indices.size() << ","
            << idx << ","
            << class_ids[idx] << ","
            << confidences[idx] << ","
            << class_name[class_ids[idx]] << ","
            << yp._score_threshold << ","
            << yp._nms_threshold << ","
            << yp._confidence_thresgold << ","
            << left << ","
            << top << ","
            << (left + width) << ","
            << (top + height) << ","
            << width << ","
            << height << ","
            << yp._onnx_file_name << ","
            << yp._names_file_name << ","
            << input_image.cols << ","
            << input_image.rows << endl;

    }
    _ost = _os.str().c_str();
    return input_image;
}
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
YoloObjectDetection::YoloObjectDetection()
{
    _YP._input_width = DEFAULT_AI_INPUT_WIDTH;
    _YP._input_height = DEFAULT_AI_INPUT_HEIGHT;

    _YP._score_threshold = (float)DEFAULT_SCORE_THRESHOLD;
    _YP._nms_threshold = (float)DEFAULT_NMS_THRESHOLD;
    _YP._confidence_thresgold = (float)DEFAULT_CONF_THRESHOLD;

    _YFP._label._scale      = (float)FONT_SCALE_LABEL;
    _YFP._label._face       = FONT_FACE_LABEL;
    _YFP._label._thickness  = THICKNESS_FONT_LABEL;

    _YFP._person._scale       = (float)FONT_SCALE_PERSON;
    _YFP._person._face        = FONT_FACE_PERSON;
    _YFP._person._thickness   = THICKNESS_FONT_PERSON;

    _YFP._time._scale        = (float)FONT_SCALE_TIME;
    _YFP._time._face         = FONT_FACE_TIME;
    _YFP._time._thickness    = THICKNESS_FONT_TIME;

    _YFP._thickness_box = THICKNESS_BOX;
}

//TR A2CW(const char* str)
//{
//    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
//    return converter.from_bytes(str).c_str();
//}
//std::wstring A2CW(const std::string& ascii)
LPCWSTR _A2CW(const std::string& ascii)
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

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//AIの初期化
//識別名リストとonnxファイルを読み込む
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//int YoloObjectDetection::init_yolov5(
//    string filepath_of_names, 
//    string filepath_of_onnx,
//    //int clssification_size, 
//    float _iw, float _ih, float _sc_th,float _nms_th, float _conf_th, 
//    bool __count_of_person, 
//    bool __count_of_time)
//GPUデバイスを選択するときは、cuda_runtime.hをインクルードして、cudaSetDevice(int device)を指定する。
//#include <cuda_runtime.h>
//cudaSetDevice(0);
int YoloObjectDetection::init_yolov5(YoloAIParametors yp, bool __count_of_person, bool __count_of_time)
{
    //USES_CONVERSION;
    _YP = yp;
    //_YP._input_width = _iw;
    //_YP._input_height = _ih;
    //_YP._score_threshold = _sc_th;
    //_YP._nms_threshold = _nms_th;
    //_YP._confidence_thresgold = _conf_th;
    //_YP._names_file_name = filepath_of_names;
    //_YP._onnx_file_name = filepath_of_onnx;

    _count_of_person = __count_of_person;
    _count_of_time = __count_of_time;

    //_clssification_size = clssification_size;

    ifstream ifs(_YP._names_file_name);
    string line;

    //wostringstream _msg;
    ostringstream _msg;
    _msg << "names: " << _YP._names_file_name << std::endl
        << "onnx: " << _YP._onnx_file_name << std::endl
        << "input width: " << _YP._input_width << std::endl
        << "input height: " << _YP._input_height << std::endl;
    LPCWSTR _lpcmsg = _A2CW(_msg.str().c_str());
    //LPCWSTR _lpcmsg = _msg.str().c_str();

    _list_of_class.clear();
    _YP._clssification_size = 0;
    while (getline(ifs, line))
    {
        //ファイルからclassificationの名を読み込む
        _list_of_class.push_back(line);
        ++_YP._clssification_size;
    }
     // Load model. yolov8だとreadNetまたはreadNetFromONNXでエラーが出る
    try
    {
        //cudaSetDevice(0);
        _LOGMSG("dnn::readNetFromONNX:" <<_YP._onnx_file_name);
        net=cv::dnn::readNetFromONNX(_YP._onnx_file_name);

#ifdef _CUDA
        net.setPreferableBackend(DNN_BACKEND_CUDA);
        net.setPreferableTarget(DNN_TARGET_CUDA_FP16);
        //net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
        //net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        //net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
#else
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
#endif
    }
    catch (const cv::Exception& e)
    {
        const char* err_msg = e.what();
        _LOGMSG(err_msg);
        int id = MessageBox(0, _lpcmsg, L"AI ONNX read file error", MB_OK);
        //int id = MessageBoxW(0, A2CW(err_msg), L"AI ONNX read file error", MB_OK);
        return 0;
    }
    ifs.close();
    return 1;
}

vector<Mat>& YoloObjectDetection::_pre_process(Mat& input_image)
{
    //vector<Mat> _detections;     // Process the image.

    if (input_image.empty() || input_image.data == NULL || input_image.cols == 0; input_image.rows == 0)
        return detections;

    pre_process(detections, input_image, net, _YP._input_width, _YP._input_height);

    //TRYCAT_CV_STD(detections = pre_process(input_image, net, _input_width, _input_height));

    return detections;
}

cv::Mat YoloObjectDetection::_post_process(
    bool draw_image,    //ふつうはture。falseにすると描画処理をせず、解析データをテキストに書くだけになる。
    const cv::Mat& input_image, 
    std::string _header, std::string& _ost
)
{
    Mat img;
    if (input_image.data == NULL)
        return img;
    
    number_of_persons = 0;

    img = post_process_str(
        _YP,
        _YFP,
        draw_image,
        input_image.clone(), //元イメージ
        detections,
        _list_of_class,         //ファイルから読み込んだ分類名のリスト
        number_of_persons, class_list_view,
        _header,            //日付等
        _ost               //書き込んだ文字列を返す時の格納場所
    );

    string label = format("Count of Persons = %i ", number_of_persons);
    //たまにnullポインターが発生する。原因は不明。
    if (img.data!=NULL)
    {
        putText(img, label, TEXT_POINT_PERSON, _YFP._person._face, _YFP._person._scale, BLACK, _YFP._person._thickness + 1);
        putText(img, label, TEXT_POINT_PERSON, _YFP._person._face, _YFP._person._scale, BLUE, _YFP._person._thickness);
         
        ostringstream _os;
        _os << "AI:" << _YP._onnx_file_name;
    }
    else
    {
        cout << "null pointer:"<<__FILE__<<":"<<__LINE__<<endl;
    }

    //処理時間の表示
    if (_count_of_time)
    {
        vector<double> layersTimes;
        double freq = getTickFrequency() / 1000;
        double t = net.getPerfProfile(layersTimes) / freq;
        string label = format("Inference time = %.2f ms", t);
        putText(img, label, Point(20, 40), _YFP._time._face, _YFP._time._scale, RED, _YFP._time._thickness);
    }
    return img;
}

