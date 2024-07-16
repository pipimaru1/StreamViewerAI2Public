#include "stdafx.h"

#include "yolov5_engine.h"
#include "logfile.h"
#include "mylib.h"
#include "sqlutil.h"
//#include <onnxruntime_cxx_api.h>

using namespace cv;
using namespace std;
using namespace cv::dnn;

//#define DEFAULT_SCORE_THRESHOLD  0.2f
//#define DEFAULT_NMS_THRESHOLD  0.60f
//#define DEFAULT_CONF_THRESHOLD  0.2f

float DEFAULT_SCORE_THRESHOLD = 0.4;
float DEFAULT_NMS_THRESHOLD = 0.5;
float DEFAULT_CONF_THRESHOLD = 0.4;

//���ꎎ��
//https://learnopencv.com/object-detection-using-yolov5-and-opencv-dnn-in-c-and-python/
//https://github.com/spmallick/learnopencv/tree/master/Object-Detection-using-YOLOv5-and-OpenCV-DNN-in-CPP-and-Python
// 
// Constants.

// Colors.
Scalar BLACK     = Scalar(0, 0, 0);
Scalar BLUE      = Scalar(255, 128, 64);
Scalar CYAN      = Scalar(255, 178, 50);
Scalar KOIBLUE   = Scalar(255, 32, 32);
Scalar YELLOW    = Scalar(0, 255, 255);
Scalar RED       = Scalar(0, 0, 255);
Scalar GREY      = Scalar(64, 64, 64);
Scalar DARKGREEN = Scalar(32, 200, 32);
Scalar DARKDARKGREEN = Scalar(24, 100, 24);
Scalar ORANGE    = Scalar(0, 160, 255);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
YoloAIParametors::YoloAIParametors()
{
    GPU_Number = 0;
    yolo_version = YOLOV5;
    input_width = DEFAULT_AI_INPUT_WIDTH;
    input_height = DEFAULT_AI_INPUT_HEIGHT;
    score_threshold = DEFAULT_SCORE_THRESHOLD;
    nms_threshold = DEFAULT_NMS_THRESHOLD;
    confidence_thresgold = DEFAULT_CONF_THRESHOLD;

    clssification_size=0;
    onnx_file_name = std::string();
    names_file_name = std::string();
}

YoloAIParametors::~YoloAIParametors()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LABEL_IN_BOX false
#define LABEL_OUT_BOX true
void draw_label(Mat& input_image, string label, int left, int top, CvFontParam cfp, cv::Scalar _rect_color)
{
    // Display the label at the top of the bounding box.
    int baseLine;
    Size label_size = getTextSize(label, cfp.face, cfp.scale, cfp.thickness, &baseLine);
    top = max(top, label_size.height);
    if (0) //�Q�l���ɂ������R�[�h? �g���Ă��Ȃ�
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

        //��??
        if (0)
        {
            putText(input_image, label, Point(left, top + label_size.height), cfp.face, cfp.scale,  BLACK, cfp.thickness + 1);
            putText(input_image, label, Point(left, top + label_size.height), cfp.face, cfp.scale, YELLOW, cfp.thickness);
        }
    }

    if (LABEL_IN_BOX)
    {
        rectangle(input_image, Point(left, top), Point(left + 105, top + 11), _rect_color, -1);
        putText(input_image, label, Point(left, top + label_size.height), cfp.face, cfp.scale, BLACK, cfp.thickness);
        //putText(input_image, label, Point(left, top + label_size.height), _fontface, _font_scale, YELLOW, _thickness_font);
    }
    if (LABEL_OUT_BOX)
    {
        rectangle(input_image, Point(left-1, top-11), Point(left + 105, top), _rect_color, -1);
        putText(input_image, label, Point(left, top -11 + label_size.height), cfp.face, cfp.scale, BLACK, cfp.thickness);
    }
}


//�F�����Ƃ̃f�[�^�𐮗����邽�߂̍\����
struct DetctionData
{
    float cx;
    float cy;
    // Box dimension.
    float w;
    float h;
    // Bounding box coordinates.
    int left;
    int top;
    int width;
    int height;

};

//�F�����Ƃ̃f�[�^�𐮗����邽�߂̍\����
struct DetectedObject
{
    cv::Rect bbox; // �o�E���f�B���O�{�b�N�X

    float cx;
    float cy;
    // Box dimension.
    float w;
    float h;

    float confidence; // �M���x�X�R�A
    int classID; // �N���XID
    int objectID;

    DetectedObject();
    DetectedObject(
        float _cx,
        float _cy,
        float _w,
        float _h,
        cv::Rect bbox, // �o�E���f�B���O�{�b�N�X
        float confidence, // �M���x�X�R�A
        int classID, // �N���XID
        int objectID
    );
};

DetectedObject::DetectedObject()
{
    cx=0.0;
    cy=0.0;
    w=0.0;
    h=0.0;
    confidence = 0.0;
    classID = 0;
    objectID = 0;
    bbox = cv::Rect(0, 0, 0, 0);
}
DetectedObject::DetectedObject
(
    float _cx,
    float _cy,
    float _w,
    float _h,
    cv::Rect _bbox,      // �o�E���f�B���O�{�b�N�X
    float _confidence,   // �M���x�X�R�A
    int _classID,        // �N���XID
    int _objectID
)
{
    cx = _cx;
    cy = _cy;
    w = _w;
    h = _h;
    confidence = _confidence;
    classID = _classID;
    objectID = _objectID;
    bbox = _bbox;
}

//�e�X�g��
int extract_object_data(
    vector<Mat>& _outputs, 
    const vector<string>& class_name, 
    vector<DetectedObject>& _objects, 
    int yolo_version,
    float _confidence_thresgold,
    float _x_factor, 
    float _y_factor
)
{
    int rows = 0;
    int dimensions = 0;
    float* _data = nullptr;

    if (yolo_version == YOLOV8)
    { 

#ifdef _CUDA
        //YOLOV8�Ńo�E���f�B���O�{�b�N�X�������Əo�Ȃ��̂͂��̂�����̍s�񑀍삪�����炵��
        //https://github.com/ultralytics/ultralytics/issues/1852
        rows = _outputs[0].size[2];
        dimensions = _outputs[0].size[1];
        _outputs[0] = _outputs[0].reshape(1, dimensions);
        cv::transpose(_outputs[0], _outputs[0]);
        
        //cv::Mat rawData = _outputs[0];
        //rawData = rawData.t();
        //int strideNum = 8400;//outputNodeDims[1];//8400
        //int signalResultNum = 84;  //outputNodeDims[2];//84

        float* data = (float*)_outputs[0].data; //�f�[�^�̈�̃|�C���^����???
        for (int i = 0; i < rows; ++i)
        //for (int i = 0; i < strideNum; ++i)
        {
            DetectedObject _obj;    //�ꎞ�f�[�^�u����

            float* classes_scores = data + 4;
            cv::Mat scores(1, (int)class_name.size(), CV_32FC1, classes_scores);
            cv::Point class_id;
            double maxClassScore;
            minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);

            if (maxClassScore > _confidence_thresgold)
            {
                //confidences.push_back((float));
                //class_ids.push_back(class_id.x);
                _obj.classID = class_id.x;
                _obj.confidence = maxClassScore;
                _obj.cx = data[0];
                _obj.cy = data[1];
                _obj.w = data[2];
                _obj.h = data[3];

                //�C���[�W�̑傫���ɍ��킹�ăo�E���f�B���O�{�b�N�X�̒l���Z�b�g����
                _obj.bbox.x = int((_obj.cx - 0.5 * _obj.w) * _x_factor);
                _obj.bbox.y = int((_obj.cy - 0.5 * _obj.h) * _y_factor);
                _obj.bbox.width = int(_obj.w * _x_factor);
                _obj.bbox.height = int(_obj.h * _y_factor);

                _objects.push_back(_obj);
            }
            data += dimensions;
            //data += signalResultNum;
        }
#else
        //https://github.com/ultralytics/ultralytics/issues/1852
        rows = _outputs[0].size[2];
        dimensions = _outputs[0].size[1];
        _outputs[0] = _outputs[0].reshape(1, dimensions);
        cv::transpose(_outputs[0], _outputs[0]);

        float* data = (float*)_outputs[0].data; //�f�[�^�̈�̃|�C���^����???
        for (int i = 0; i < rows; ++i)
        {
            DetectedObject _obj;    //�ꎞ�f�[�^�u����

            float* classes_scores = data + 4;
            cv::Mat scores(1, (int)class_name.size(), CV_32FC1, classes_scores);
            cv::Point class_id;
            double maxClassScore;
            minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);

            if (maxClassScore > _confidence_thresgold)
            {
                //confidences.push_back((float));
                //class_ids.push_back(class_id.x);
                _obj.classID = class_id.x;
                _obj.confidence = maxClassScore;
                _obj.cx = data[0];
                _obj.cy = data[1];
                _obj.w = data[2];
                _obj.h = data[3];

                //�C���[�W�̑傫���ɍ��킹�ăo�E���f�B���O�{�b�N�X�̒l���Z�b�g����
                _obj.bbox.x = int((_obj.cx - 0.5 * _obj.w) * _x_factor);
                _obj.bbox.y = int((_obj.cy - 0.5 * _obj.h) * _y_factor);
                _obj.bbox.width = int(_obj.w * _x_factor);
                _obj.bbox.height = int(_obj.h * _y_factor);

                _objects.push_back(_obj);
            }
            data += dimensions;
        }
#endif
    }
    else if (yolo_version == YOLOV5)
    {
        rows = _outputs[0].size[1]; //���o�����I�u�W�F�N�g�̐�?
        dimensions = _outputs[0].size[2]; //��������? �e�I�u�W�F�N�g�̗v�f��
        float* _data = (float*)_outputs[0].data; //�f�[�^�̈�̃|�C���^����???

        for (int i = 0; i < rows; ++i)
        {
            DetectedObject _obj;    //�ꎞ�f�[�^�u����
            _obj.confidence = _data[4];
            if (_obj.confidence >= _confidence_thresgold)
            {
                float* _classes_scores = _data + 5;
                Mat scores(1, (int)class_name.size(), CV_32FC1, _classes_scores);
                cv::Point class_id;
                double maxClassScore;
                minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);
                _obj.classID = class_id.x;

                _obj.confidence = _data[4];
                _obj.cx = _data[0];
                _obj.cy = _data[1];
                _obj.w = _data[2];
                _obj.h = _data[3];

                //�C���[�W�̑傫���ɍ��킹�ăo�E���f�B���O�{�b�N�X�̒l���Z�b�g����
                _obj.bbox.x = int((_obj.cx - 0.5 * _obj.w) * _x_factor);
                _obj.bbox.y = int((_obj.cy - 0.5 * _obj.h) * _y_factor);
                _obj.bbox.width = int(_obj.w * _x_factor);
                _obj.bbox.height = int(_obj.h * _y_factor);

                //_obj.bbox.x = max(_obj.bbox.x, 0);
                //_obj.bbox.y = max(_obj.bbox.y, 0);

                _objects.push_back(_obj);
            }
            _data += dimensions;
        }
    }

    return 0;
}

//AI�ɂ�錟�m �A�h���X�l���󂯎���Ċi�[������@�ɕύX
int pre_process(vector<Mat>& outputs,Mat& input_image, Net& net, float input_width= DEFAULT_AI_INPUT_WIDTH, float input_height= DEFAULT_AI_INPUT_HEIGHT)
{
    Mat blob;
    int ret = 0;
    
    //vector<Mat> outputs;
    
    //��O���������Ă݂�
    if (false)
    {
        //���̊֐��̌Ăяo�����Ƃ�try�����Ă���
        _TRYCAT(blobFromImage(input_image, blob, 1.0/255.0, Size((int)input_width, (int)input_height), Scalar(), true, false));
        _TRYCAT(net.setInput(blob));
        // Forward propagate.
        _TRYCAT(net.forward(outputs, net.getUnconnectedOutLayersNames()));

    }
   //������NULL�`�F�b�N �ǂ����������̂��
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
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            //�����G���[�񍐂���
            // https://github.com/opencv/opencv/issues/23977
            // OpenCV CUDA �r���h���g�p���� ONNX ���f���Ō��o�����s����ƁAYOLOv8 ���E�{�b�N�X�̐��@�� 0 �ɂȂ� #23977
            // https://github.com/ultralytics/ultralytics/issues/3682
            // OpenCV 4.7.0���Ƃ��܂������ƕ񍐂���
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            blobFromImage(input_image, blob, 1.0/255.0, Size((int)input_width, (int)input_height), Scalar(), true, false);
            net.setInput(blob);
            // Forward propagate.
            //LOGMSG2("outputs1", outputs);
            //TRYCAT(net.forward(outputs, net.getUnconnectedOutLayersNames()));
            net.forward(outputs, net.getUnconnectedOutLayersNames());
            //LOGMSG2("outputs2", outputs);
            ret = 0;
        }
    }    
    return ret;
}

//�w�������I�u�W�F�N�g������\������ꍇ��false�ɂ���
#define VIEW_ALL true 

long long EventID = 0;
//AI�̌��ʂ��X�g���[���ɏo�͂���
//�`��@�\�͏Ȃ��� 
//������post_process�Ɠ��������ق�������
//AI�̌��ʂ𕶎���Ƃ���_ai_csv_ostring�ɏo�͂���
// ���ʂ̓J���}�ŋ�؂�B
//_header�͍��ڂ̐擪�ɂ���w�b�_�@�J�����ԍ��A�t���[���ԁA���ԂȂ�
//#define CLASSIFICATION_SIZE 11
Mat post_process_str(
    YoloAIParametors yp,
    YoloFontsParam yfp,
    bool draw_image,                        //�ӂ���ture, false�ɂ���ƕ`�揈�������Ȃ��B
    const Mat& input_image,                 //����Mat�ɏ㏑�����ĕԂ�
    vector<Mat>& outputs,                   //ai�̕��ނ̏��
    const vector<string>& class_name,
    int& number_of_persons, std::vector<std::string>& class_list_view, 
    //std::string _header,                  //���t��
    AiRecordHeader& _ai_rec_header,         //���t��
    std::string& _ai_csv_ostring,           //�������񂾕������Ԃ����̊i�[�ꏊ
    std::vector<AiSqlOutput>& _sql_ai_data,
    int _version                            //yolo�̃o�[�W����
)
{
    //�������o���Ă��Ȃ���΂��̂܂ܕԂ�
    if (outputs.size() == 0)
        return input_image;

    cv::Mat output_image = input_image;

#ifdef _DEBUG
    if (1)
    {
        //�֐��̒��Ńg�����X�t�H�[������̂ňꎞ�f�[�^�ɃR�s�[
        vector<Mat> _tmp_outputs = outputs;
        vector<DetectedObject> _objects;
        extract_object_data(_tmp_outputs, class_name, _objects, yp.yolo_version, yp.score_threshold, input_image.cols / yp.input_width, input_image.rows / yp.input_height);
    }
#endif

    //std::vector<DetectedObject> _detections;
    //_detections=convertDetections(outputs, yp._confidence_thresgold );


    // Initialize vectors to hold respective outputs while unwrapping     detections.
    vector<int> class_ids;
    vector<float> confidences;
    vector<Rect> boxes;
    // Resizing factor. �v�f�̃��T�C�Y
    float x_factor = input_image.cols / yp.input_width;
    float y_factor = input_image.rows / yp.input_height;

    int rows = outputs[0].size[1]; //���o�����I�u�W�F�N�g�̐�?
    int dimensions = outputs[0].size[2]; //��������? �e�I�u�W�F�N�g�̗v�f��
    bool yolov8 = false;

    // yolov5 has an output of shape (batchSize, 25200, 85) (Num classes + box[x,y,w,h] + confidence[c])
    // yolov8 has an output of shape (batchSize, 84,  8400) (Num classes + box[x,y,w,h])
    if (0)
    {
        if (dimensions > rows) // Check if the shape[2] is more than shape[1] (yolov8) gpu�̏ꍇ�͐��藧���Ȃ�
        {
            yolov8 = true;
            rows = outputs[0].size[2];
            dimensions = outputs[0].size[1];

            outputs[0] = outputs[0].reshape(1, dimensions);
            cv::transpose(outputs[0], outputs[0]);
        }
        float* data = (float*)outputs[0].data; //�f�[�^�̈�̃|�C���^����???
    }
    // 25200 for default size 640.
    // Iterate through 25200 detections.
    if (_version == YOLOV8)
    {
        //https://github.com/ultralytics/ultralytics/issues/1852
        rows = outputs[0].size[2];
        dimensions = outputs[0].size[1];
        outputs[0] = outputs[0].reshape(1, dimensions);
        cv::transpose(outputs[0], outputs[0]);

        float* data = (float*)outputs[0].data; //�f�[�^�̈�̃|�C���^����???
        for (int i = 0; i < rows; ++i)
        {
            float* classes_scores = data + 4;

            cv::Mat scores(1, (int)class_name.size(), CV_32FC1, classes_scores);
            cv::Point class_id;
            double maxClassScore;

            minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);

            if (maxClassScore > yp.score_threshold)
            {
                confidences.push_back((float)maxClassScore);
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
            data += dimensions;
        }
    }
    else if(_version == YOLOV5)//yolov5
    {
        float* data = (float*)outputs[0].data; //�f�[�^�̈�̃|�C���^����???
        for (int i = 0; i < rows; ++i)
        {
            //yolov5
            float confidence = data[4];
            // Discard bad detections and continue.
            if (confidence >= yp.confidence_thresgold)
            {
                float* classes_scores = data + 5;
                // Create a 1x85 Mat and store class scores of 80 classes.
                Mat scores(1, (int)class_name.size(), CV_32FC1, classes_scores);
                // Perform minMaxLoc and acquire the index of best class  score.
                Point class_id;
                double max_class_score;
                minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
                // Continue if the class score is above the threshold.
                if (max_class_score > yp.score_threshold)
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
            data += dimensions;
        }
    }
    ostringstream _os;
    Scalar RECTCOLOR;
    //�͂��l�p��`��
    vector<int> indices;
    //cudnn�̊֐�
    //���o�������̂̏d�Ȃ����瓯�ꐫ�𔻒f�����o����֐� indices�ɒ��o����id�̔z�񂪓������
    cv::dnn::NMSBoxes(boxes, confidences, yp.score_threshold, yp.nms_threshold, indices);
    number_of_persons = 0;

    //SQL
    //std::vector<AiSqlOutput> sql_ai_data;

    for (int i = 0; i < indices.size(); i++)
    {
        int idx = indices[i];
        Rect box = boxes[idx];
        int left    = box.x      ;
        int top     = box.y      ;
        int width   = box.width  ;
        int height  = box.height ;

        bool draw_rect = VIEW_ALL;

        //�I�u�W�F�N�g���̊i�[
        string object_name(class_name[class_ids[idx]]);

        int count_class_list_view = 0;
        //�\���������I�u�W�F�N�g��ސ�������
        while (count_class_list_view < class_list_view.size())
        {
            //�\���������I�u�W�F�N�g��������\���t���O�𗧂Ă�
            if (object_name.compare(class_list_view[count_class_list_view]) == 0)
                draw_rect = true;
            count_class_list_view++;
        }

        //�ǉ����������@�I�u�W�F�N�g�ɂ���Đ�������F��ς�����
        //�l�������琔����
        if (object_name.compare("person") == 0 ||
            object_name.compare("driver") == 0)
        {
            number_of_persons++;
            RECTCOLOR = CYAN;
        }
        else if (object_name.compare("forklift") == 0)
        {
            RECTCOLOR = KOIBLUE;
        }
        else if (object_name.compare("truck") == 0)
        {
            RECTCOLOR = ORANGE;
        }
        else if (object_name.compare("excavator") == 0 ||
            object_name.compare("bulldozer") == 0 ||
            object_name.compare("wheelloder") == 0 ||
            object_name.compare("grader") == 0)
        {
            RECTCOLOR = YELLOW;
        }
        else if (object_name.compare("pallet") == 0 ||
                 object_name.compare("cargo") == 0 )
            RECTCOLOR = DARKDARKGREEN;
        else
            RECTCOLOR = DARKGREEN;

        if (draw_image && draw_rect) //�f�[�^�������݂݂̂̏ꍇ�͕`��͂��Ȃ�
        {
            // Draw bounding box.
            rectangle(output_image, Point(left, top), Point(left + width, top + height), RECTCOLOR, yfp.thickness_box);
            // Get the label for the class name and its confidence.
            string label = format("%.2f", confidences[idx]);
            label = class_name[class_ids[idx]] + ":" + label;
            // Draw class labels.
            draw_label(output_image, label, left, top, yfp.label, RECTCOLOR);
        }
        
        AiSqlOutput _ai_sql_output;
        _ai_sql_output.EventID = EventID;  // INT,
        _ai_sql_output.Category = L"AI";
        _ai_sql_output.Location = _ai_rec_header.Location;
        _ai_sql_output.StreamURL = _ai_rec_header.StreamURL;
        _ai_sql_output.Timestamp = string2wstring(TimeStumpStr());   // DATETIME,
        _ai_sql_output.Index_per_frame = 1;                           // INT,
        _ai_sql_output.index_max_frame = indices.size();              //  INT,
        _ai_sql_output.idx = idx; //  INT,
        _ai_sql_output.ClassID = class_ids[idx]; //  INT,
        _ai_sql_output.Confidence = confidences[idx]; //  FLOAT,
        _ai_sql_output.ClassName = string2wstring(class_name[class_ids[idx]]); //  NVARCHAR(50),
        _ai_sql_output.ScoreThreshold = yp.score_threshold; //  FLOAT,
        _ai_sql_output.NmsThreshold = yp.nms_threshold; //  FLOAT,
        _ai_sql_output.ConfidenceThreshold = yp.confidence_thresgold; //  FLOAT,
        _ai_sql_output.x0 = left;                         //  INT,
        _ai_sql_output.y0 = top;                          //  INT,
        _ai_sql_output.x1 = (left + width);               //  INT,
        _ai_sql_output.y1 = (top + height);               //  INT,
        _ai_sql_output.Width = width;                     //  INT,
        _ai_sql_output.Height = height;                   //  INT,
        _ai_sql_output.OnnxFileName = string2wstring(getFileName( yp.onnx_file_name)); //  NVARCHAR(255),
        _ai_sql_output.NamesFileName = string2wstring(getFileName(yp.names_file_name)); //  NVARCHAR(255),
        _ai_sql_output.ImageWidth = input_image.cols; //  INT,
        _ai_sql_output.ImageHeight = input_image.rows; //  INT

        EventID++;  // INT,

        _sql_ai_data.push_back(_ai_sql_output);

        //�����R�[�h�Ŏc��
        std::ostringstream _header;
        _header << "AI,\"" << wstring2string( _ai_rec_header.Location) << "\"," << wstring2string(_ai_rec_header.StreamURL)<< "," << wstring2string(_ai_rec_header.Timestamp);
        //�ǉ����������@�X�g���[���ɉ�͂������ʂ�ۑ�
        _os << _header.str() << ","
            << i << ","
            << indices.size() << ","
            << idx << ","
            << class_ids[idx] << ","
            << confidences[idx] << ","
            << class_name[class_ids[idx]] << ","
            << yp.score_threshold << ","
            << yp.nms_threshold << ","
            << yp.confidence_thresgold << ","
            << left << ","
            << top << ","
            << (left + width) << ","
            << (top + height) << ","
            << width << ","
            << height << ","
            << getFileName(yp.onnx_file_name) << ","
            << getFileName(yp.names_file_name) << ","
            << input_image.cols << ","
            << input_image.rows << endl;
    }
    //�T�[�o�[�ƃf�[�^�i�[��̐ݒ�
    if (0)
    {
        SqlServer _sql_server;
        _sql_server.server_name = L"140.81.145.5\\AWIOTQ01";
        _sql_server.db_name = L"aicamera";
        _sql_server.uid = L"sa";
        _sql_server.pwd = L"AWSqlServer01";

        Sql_Write(_sql_server, _sql_ai_data, _sql_ai_data.size());
    }
    
    _ai_csv_ostring = _os.str().c_str();
    return input_image;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
YoloObjectDetection::YoloObjectDetection()
{
    YP.yolo_version = YOLOV5;
    YP.input_width = DEFAULT_AI_INPUT_WIDTH;
    YP.input_height = DEFAULT_AI_INPUT_HEIGHT;
    YP.score_threshold = (float)DEFAULT_SCORE_THRESHOLD;
    YP.nms_threshold = (float)DEFAULT_NMS_THRESHOLD;
    YP.confidence_thresgold = (float)DEFAULT_CONF_THRESHOLD;

    YFP.label.scale      = (float)FONT_SCALE_LABEL;
    YFP.label.face       = FONT_FACE_LABEL;
    YFP.label.thickness  = THICKNESS_FONT_LABEL;

    YFP.person.scale       = (float)FONT_SCALE_PERSON;
    YFP.person.face        = FONT_FACE_PERSON;
    YFP.person.thickness   = THICKNESS_FONT_PERSON;

    YFP.time.scale        = (float)FONT_SCALE_TIME;
    YFP.time.face         = FONT_FACE_TIME;
    YFP.time.thickness    = THICKNESS_FONT_TIME;

    YFP.thickness_box = THICKNESS_BOX;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//AI�̏�����
//���ʖ����X�g��onnx�t�@�C����ǂݍ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//int YoloObjectDetection::init_yolov5(
//    string filepath_of_names, 
//    string filepath_of_onnx,
//    //int clssification_size, 
//    float _iw, float _ih, float _sc_th,float _nms_th, float _conf_th, 
//    bool _count_of_person, 
//    bool __count_of_time)
//GPU�f�o�C�X��I������Ƃ��́Acuda_runtime.h���C���N���[�h���āAcudaSetDevice(int device)���w�肷��B
//#include <cuda_runtime.h>
//cudaSetDevice(0);
int YoloObjectDetection::init_object_detection(YoloAIParametors yp, bool _count_of_person, bool __count_of_time)
{
    //USES_CONVERSION;
    YP = yp;

    count_of_person = _count_of_person;
    count_of_time = __count_of_time;

    ifstream ifs(YP.names_file_name);
    string line;

    //wostringstream _msg;
    ostringstream _msg;
    _msg << "names: " << YP.names_file_name << std::endl
        << "onnx: " << YP.onnx_file_name << std::endl
        << "input width: " << YP.input_width << std::endl
        << "input height: " << YP.input_height << std::endl;

    std::wstring _wsmsg = _A2CW(_msg.str().c_str());

    list_of_class.clear();
    YP.clssification_size = 0;
    while (getline(ifs, line))
    {
        //�t�@�C������classification�̖���ǂݍ���
        list_of_class.push_back(line);
        ++YP.clssification_size;
    }
     // Load model. yolov8����readNet�܂���readNetFromONNX�ŃG���[���o��
    LOGMSG("dnn::readNetFromONNX:" << YP.onnx_file_name);
    try
    {

#ifdef _CUDA
    #ifndef _OPENCV470
        //cudaSetDevice(YP.GPU_Number);
    #endif
        net = cv::dnn::readNetFromONNX(YP.onnx_file_name);
        net.setPreferableBackend(DNN_BACKEND_CUDA);
        net.setPreferableTarget(DNN_TARGET_CUDA_FP16);
        //net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
        //net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        //net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
#else
        net = cv::dnn::readNetFromONNX(YP.onnx_file_name);
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
#endif
    }
    catch (const cv::Exception& e)
    {
        const char* err_msg = e.what();
        LOGMSG(err_msg);
        int id = MessageBox(0, _wsmsg.c_str(), L"AI ONNX read file error", MB_OK);
        //int id = MessageBoxW(0, A2CW(err_msg), L"AI ONNX read file error", MB_OK);
        return 0;
    }
    ifs.close();
    return 1;
}

vector<Mat>& YoloObjectDetection::_pre_process(Mat& input_image)
{
    if (input_image.empty() || input_image.data == NULL || input_image.cols == 0; input_image.rows == 0)
        return detections;

    pre_process(detections, input_image, net, YP.input_width, YP.input_height);

    //TRYCAT_CV_STD(detections = pre_process(input_image, net, _input_width, _input_height));

    return detections;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//AI�̉�͌��ʂ�\��������A�t�@�C���ɏ������񂾂�A�f�[�^�x�[�X�ɓ��ꂽ�肷��
//�����͋@�\���O�ɕ����ăV���v���ɂ���
cv::Mat YoloObjectDetection::_post_process(
    bool draw_image,    //�ӂ���ture�Bfalse�ɂ���ƕ`�揈���������A��̓f�[�^���e�L�X�g�ɏ��������ɂȂ�B
    const cv::Mat& input_image, 
    //std::string _header, 
    AiRecordHeader& _ai_rec_header,
    std::string& _ai_csv_ostring,
    std::vector<AiSqlOutput>& _sql_ai_data
)
{
    Mat img;
    if (input_image.data == NULL)
        return img;
    
    number_of_persons = 0;

    img = post_process_str(
        YP,
        YFP,
        draw_image,
        input_image.clone(), //���C���[�W
        detections,
        list_of_class,         //�t�@�C������ǂݍ��񂾕��ޖ��̃��X�g
        number_of_persons, class_list_view,
        //_header,            //���t��
        _ai_rec_header,            //���t��
        _ai_csv_ostring,              //�������񂾕������Ԃ����̊i�[�ꏊ
        _sql_ai_data,
        YP.yolo_version
        );

    string label = format("Count of Persons = %i ", number_of_persons);
    //���܂�null�|�C���^�[����������B�����͕s���B
    if (img.data!=NULL)
    {
        putText(img, label, TEXT_POINT_PERSON, YFP.person.face, YFP.person.scale, BLACK, YFP.person.thickness + 1);
        putText(img, label, TEXT_POINT_PERSON, YFP.person.face, YFP.person.scale, CYAN, YFP.person.thickness);
         
        ostringstream _os;
        _os << "AI:" << YP.onnx_file_name;
    }
    else
    {
        cout << "null pointer:"<<__FILE__<<":"<<__LINE__<<endl;
    }

    //�������Ԃ̕\��
    if (count_of_time)
    {
        vector<double> layersTimes;
        double freq = getTickFrequency() / 1000;
        double t = net.getPerfProfile(layersTimes) / freq;
        string label = format("Inference time = %.2f ms", t);
        putText(img, label, Point(20, 40), YFP.time.face, YFP.time.scale, RED, YFP.time.thickness);
    }
    return img;
}

