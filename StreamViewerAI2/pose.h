#pragma once
//int run_pose();
//HBITMAP run_pose();
//HBITMAP run_pose(int& width, int& height);

//#define POSE_COCO 1
//#define POSE_MPI 2


////////////////////////////////
struct KEYPoint
{
    KEYPoint(cv::Point point, float probability)
    {
        this->id = -1;
        this->point = point;
        this->probability = probability;
    }

    int id;
    cv::Point point;
    float probability;
};

////////////////////////////////
struct ValidPair {
    ValidPair(int aId, int bId, float score) 
    {
        this->aId = aId;
        this->bId = bId;
        this->score = score;
    }

    int aId;
    int bId;
    float score;
};

#define OUTPUT_IMAGE_MODE_NORMAL 0
#define OUTPUT_IMAGE_MODE_GRAY 1
class PoseNet
{
private:
    //ì‡ïîÇ…éùÇ¡ÇƒÇ¢ÇΩÇŸÇ§Ç™à¿íËÇ∑ÇÈ
    const std::string keypointsMapping[18] =
    {
        "Nose", "Neck",
        "R-Sho", "R-Elb", "R-Wr",
        "L-Sho", "L-Elb", "L-Wr",
        "R-Hip", "R-Knee", "R-Ank",
        "L-Hip", "L-Knee", "L-Ank",
        "R-Eye", "L-Eye", "R-Ear", "L-Ear"
    };

    const std::vector<std::pair<int, int>> mapIdx =
    {
        {31,32}, {39,40}, {33,34}, {35,36}, {41,42}, {43,44},
        {19,20}, {21,22}, {23,24}, {25,26}, {27,28}, {29,30},
        {47,48}, {49,50}, {53,54}, {51,52}, {55,56}, {37,38},
        {45,46}
    };

    const std::vector<std::pair<int, int>> posePairs =
    {
        {1,2}, {1,5}, {2,3}, {3,4}, {5,6}, {6,7},
        {1,8}, {8,9}, {9,10}, {1,11}, {11,12}, {12,13},
        {1,0}, {0,14}, {14,16}, {0,15}, {15,17}, {2,17},
        {5,16}
    };

    int inWidth = 368;
    int inHeight = 368;
    float thresh = 0.1f;

    //cv::dnn::Net net;
    cv::dnn::Net inputNet;
    //cv::Mat input;
    //cv::Mat frameCopy;

    cv::Mat inputBlob;
    cv::Mat netOutputBlob;
    std::string protoFile;
    std::string weightsFile;
    //std::vector<cv::Mat> netOutputParts;

    //int* POSE_PAIRS[2];

    //int POSE_PAIR(int _n, int _m);

    //int mode;
    int nPoints = 18;

public:

    PoseNet();
    //PoseNet(int _mode);

    int init(std::string& _protoFile, std::string& _weightsFile, int _gpu);
    int init(const char* _protoFile, const char* _weightsFile, int _gpu);
    //int set_mode(int _mode);

    HBITMAP run_pose_multi(std::string& _imageFile);
    HBITMAP run_pose_multi(const char* _imageFile);

    cv::Mat run_pose_multi(cv::Mat& input, int _output_image_mode = OUTPUT_IMAGE_MODE_NORMAL);

    int frameWidth=0;
    int frameHeight=0;

    void getValidPairs(
        const std::vector<cv::Mat>& netOutputParts,
        const std::vector<std::vector<KEYPoint>>& detectedKeypoints,
        std::vector<std::vector<ValidPair>>& validPairs,
        std::set<int>& invalidPairs);

    void getPersonwiseKeypoints(
        const std::vector<std::vector<ValidPair>>& validPairs,
        const std::set<int>& invalidPairs,
        std::vector<std::vector<int>>& personwiseKeypoints);

    void splitNetOutputBlobToParts(cv::Mat& input, std::vector<cv::Mat>& netOutputParts);


    inline std::string& set_protoFile(std::string& _protoFile)
    {
        protoFile = _protoFile;
        return protoFile;
    };
    inline std::string& set_weightsFile(std::string& _weightsFile)
    {
        weightsFile = _weightsFile;
        return weightsFile;
    };
    inline std::string& get_protoFile() { return protoFile; };
    inline std::string& get_weightsFile() { return weightsFile; };


};

#ifdef _CUDA
#define USE_GPU true
#else
#define USE_GPU false
#endif

//HBITMAP run_pose_single(std::string& imageFile, int& width, int& height);

//inline int PoseNet::POSE_PAIR(int _n, int _m )
//{
//    return posePairs[_n][_m];
//}
//extern PoseNet* _pt_posenet;
