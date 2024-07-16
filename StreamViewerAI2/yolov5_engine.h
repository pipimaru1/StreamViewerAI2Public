#pragma once

#include <iostream>
#include <opencv2\opencv.hpp>
//#include "E:\Programing\opencv480g\build\install\include\opencv2\opencv.hpp"
#include <fstream>
#include <filesystem>
#include <vector>
//#include <onnxruntime_cxx_api.h>

#include "sqlutil.h"

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

//#define DEFAULT_SCORE_THRESHOLD  0.2f
//#define DEFAULT_NMS_THRESHOLD  0.60f
//#define DEFAULT_CONF_THRESHOLD  0.2f

extern float DEFAULT_SCORE_THRESHOLD;
extern float DEFAULT_NMS_THRESHOLD;
extern float DEFAULT_CONF_THRESHOLD;

extern long long EventID;

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

#define YOLOV5 0
#define YOLOV8 1

struct YoloAIParametors
{
public:
    int GPU_Number;
    int yolo_version;

    float input_width;
    float input_height;

    float score_threshold;
    float nms_threshold;
    float confidence_thresgold;

    int clssification_size;
    std::string onnx_file_name;
    std::string names_file_name;

    YoloAIParametors();
    
    //YoloAIParametors(
    //float _input_width = DEFAULT_AI_INPUT_WIDTH,
    //float _input_height = DEFAULT_AI_INPUT_HEIGHT,
    //float _score_threshold = DEFAULT_SCORE_THRESHOLD,
    //float _nms_threshold = DEFAULT_NMS_THRESHOLD,
    //float _confidence_thresgold = DEFAULT_CONF_THRESHOLD);

    ~YoloAIParametors();
};
struct CvFontParam
{
    float scale;
    int face;
    int thickness;
};

struct YoloFontsParam
{
    CvFontParam label;
    CvFontParam person;
    CvFontParam time;

    int thickness_box;
};

//�F�����̃f�[�^�𐮗����邽�߂̍\����
//struct DetectedObject {
//    float x;      // �o�E���f�B���O�{�b�N�X�̒��S��X���W
//    float y;      // �o�E���f�B���O�{�b�N�X�̒��S��Y���W
//    float width;  // �o�E���f�B���O�{�b�N�X�̕�
//    float height; // �o�E���f�B���O�{�b�N�X�̍���
//    float confidence; // �M���x
//    int classId;  // �N���XID
//};
//YOLOv5��net.forward()���\�b�h���瓾����cv::Mat�̃f�[�^��DetectedObject�\���̂̃x�N�^�ɕϊ�����
//std::vector<DetectedObject> convertDetections(const std::vector<cv::Mat>& outputs, float confThreshold);

class YoloObjectDetection
{
    bool count_of_person = false;
    bool count_of_time = false;
    //volatile  bool _busy = false;

public:
    YoloAIParametors YP;
private:
    YoloFontsParam YFP;

public:
    YoloObjectDetection();

    int number_of_persons = 0;
    std::vector<cv::Mat> detections;     // Process the image.
    std::vector<std::string> list_of_class;
    std::vector<std::string> class_list_view = { "person", "forklift", "tractor", "driver", "truck", "excavator", "wheelloder", "grader", "bulldozer", "pallet", "cargo" , "car" };
    cv::dnn::Net net;

    std::vector<cv::Mat>& _pre_process(cv::Mat& input_image);

    cv::Mat _post_process(
        bool draw_image,
        const cv::Mat& input_image, 
        //std::string _header, 
        AiRecordHeader& _ai_rec_header,
        std::string& _ai_csv_ostring,
        std::vector<AiSqlOutput>& _sql_ai_data
    );

    //int load_YoloObjectDetection(int _load, std::wstring _file_onnx, std::wstring _file_names);


    int init_object_detection(
        YoloAIParametors yp,
        bool __count_of_person, bool __count_of_time);

public:
    std::atomic<bool> busy = false;//�g���Ă��Ȃ�����
};

cv::Mat post_process_str(
    YoloAIParametors yp,
    YoloFontsParam yfp,
    bool draw_image,                //�ӂ���ture, false�ɂ���ƕ`�揈�������Ȃ��B�e�L�X�g�̂ݕԂ��B��͗p�B
    const cv::Mat& input_image, std::vector<cv::Mat>& outputs, const std::vector<std::string>& class_name,
    int& number_of_persons, std::vector<std::string>& class_list_view,
    std::string _header,            //���t��
    std::string& _ai_csv_ostring,               //AI�̉�͌��ʂ��������񂾕�����̊i�[�ꏊ
    std::vector<AiSqlOutput>& _sql_ai_data,
    int _version
);

/////////////////////////////////
//YOLO V8
