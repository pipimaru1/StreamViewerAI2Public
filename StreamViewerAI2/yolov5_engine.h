#pragma once

#include <torch/torch.h>
#include <iostream>
#include <opencv2\opencv.hpp>
//#include "E:\Programing\opencv480g\build\install\include\opencv2\opencv.hpp"
#include <fstream>
#include <filesystem>
#include <vector>
#include <onnxruntime_cxx_api.h>

#define DEFAULT_ONNX_FILE_PATH "awz.onnx"
#define DEFAULT_NAMES_FILE_PATH "awz.names"
#define DEFAULT_ONNX_FILE_PATH_YOLOV5 "yolov5s.onnx"
#define DEFAULT_NAMES_FILE_PATH_YOLOV5 "yolov5s.names"
//#define DEFAULT_CLASSIFICATION_SIZE 12


#ifdef _GPUX
#define DEFAULT_AI_INPUT_WIDTH 1280.0f
#define DEFAULT_AI_INPUT_HEIGHT 1280.0f
#else
#define DEFAULT_AI_INPUT_WIDTH 640.0f
#define DEFAULT_AI_INPUT_HEIGHT 640.0f
#endif



#define DEFAULT_SCORE_THRESHOLD  0.2f
#define DEFAULT_NMS_THRESHOLD  0.60f
#define DEFAULT_CONF_THRESHOLD  0.2f

// Text parameters.
#define FONT_SCALE_LABEL  0.4f
#define FONT_FACE_LABEL   FONT_HERSHEY_SIMPLEX
#define THICKNESS_FONT_LABEL   1

//for sumnail big font
#define FONT_SCALE_PERSON  0.75f
#define FONT_FACE_PERSON   FONT_HERSHEY_SIMPLEX
#define THICKNESS_FONT_PERSON   2
#define TEXT_POINT_PERSON  Point(10, 20)

#define FONT_SCALE_TIME  0.3f
#define FONT_FACE_TIME   FONT_HERSHEY_SIMPLEX
#define THICKNESS_FONT_TIME   1


#define THICKNESS_BOX   2
#define THICKNESS_FONT   1


struct YoloAIParametors
{
public:
    float _input_width;
    float _input_height;

    float _score_threshold;
    float _nms_threshold;
    float _confidence_thresgold;

    int _clssification_size;
    std::string _onnx_file_name;
    std::string _names_file_name;

    YoloAIParametors();

/*    
        YoloAIParametors(
        float _input_width = DEFAULT_AI_INPUT_WIDTH,
        float _input_height = DEFAULT_AI_INPUT_HEIGHT,
        float _score_threshold = DEFAULT_SCORE_THRESHOLD,
        float _nms_threshold = DEFAULT_NMS_THRESHOLD,
        float _confidence_thresgold = DEFAULT_CONF_THRESHOLD);
 */
    ~YoloAIParametors();
};
struct CvFontParam
{
    float _scale;
    int _face;
    int _thickness;
};

struct YoloFontsParam
{
    CvFontParam _label;
    CvFontParam _person;
    CvFontParam _time;

    int _thickness_box;
};


class YoloObjectDetection
{
public:
    bool busy();
    bool busy_set();
    bool busy_relese();

public:
    YoloAIParametors _YP;

private:
    YoloFontsParam _YFP;

    bool _count_of_person = false;
    bool _count_of_time = false;
    volatile  bool _busy = false;

public:
    int number_of_persons = 0;
    std::vector<cv::Mat> detections;     // Process the image.
    std::vector<std::string> _list_of_class;
    std::vector<std::string> class_list_view = { "person", "forklift", "tractor", "driver", "truck", "excavator", "wheelloder", "grader", "bulldozer", "pallet", "cargo" , "car" };
    cv::dnn::Net net;

    std::vector<cv::Mat>& _pre_process(cv::Mat& input_image);
    //cv::Mat _post_process_simple(
    //    bool draw_image,
    //    cv::Mat& input_image
    //);
    cv::Mat _post_process(
        bool draw_image,
        const cv::Mat& input_image, 
        std::string _header, std::string& _ost
    );

    YoloObjectDetection();

    int init_yolov5(
        YoloAIParametors yp,
        bool __count_of_person, bool __count_of_time);

    //int init_yolov5(
    //    std::string filepath_of_names, 
    //    std::string filepath_of_onnx, 
    //    //int clssification_size, 
    //    float _iw, //ここは学習モデルで固定
    //    float _ih, //ここは学習モデルで固定 
    //    float _sc_th, float _nms_th, float _conf_th, 
    //    bool __count_of_person, bool __count_of_time);
  
    //int init_font(float __font_scale_label, int __thickness_font_label, int __fontface_label, 
    //              float __font_scale_person, int __thickness_font_person, int __fontface_person);
};

cv::Mat post_process_str(
    YoloAIParametors yp,
    YoloFontsParam yfp,
    bool draw_image,                //ふつうはture, falseにすると描画処理をしない。テキストのみ返す。解析用。
    const cv::Mat& input_image, std::vector<cv::Mat>& outputs, const std::vector<std::string>& class_name,
    int& number_of_persons, std::vector<std::string>& class_list_view,
    std::string _header,            //日付等
    std::string& _ost               //AIの解析結果を書き込んだ文字列の格納場所
);

LPCWSTR _A2CW(const std::string& ascii);