#include "stdafx.h"
#include "pose.h"

#include <opencv2\opencv.hpp>
#include <opencv2\dnn.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\highgui.hpp>

using namespace std;
using namespace cv;
using namespace cv::dnn;

//キーポイントペアの定義
//#define COCO
//#define MPI

float pose_joint_circle = 2.0f;
float pose_link_line = 4.0f;
#define LINECOLOR (cv::Scalar(192, 192, 0))

HBITMAP MatToBitmap(const cv::Mat& mat);

////////////////////////////////
//operator
////////////////////////////////
std::ostream& operator << (std::ostream& os, const KEYPoint& kp)
{
    os << "Id:" << kp.id << ", Point:" << kp.point << ", Prob:" << kp.probability << std::endl;
    return os;
}

std::ostream& operator << (std::ostream& os, const ValidPair& vp)
{
    os << "A:" << vp.aId << ", B:" << vp.bId << ", score:" << vp.score << std::endl;
    return os;
}
////////////////////////////////
template < class T > std::ostream& operator << (std::ostream& os, const std::vector<T>& v)
{
    os << "[";
    bool first = true;
    for (typename std::vector<T>::const_iterator ii = v.begin(); ii != v.end(); ++ii, first = false)
    {
        if (!first) os << ",";
        os << " " << *ii;
    }
    os << "]";
    return os;
}
////////////////////////////////
template < class T > std::ostream& operator << (std::ostream& os, const std::set<T>& v)
{
    os << "[";
    bool first = true;
    for (typename std::set<T>::const_iterator ii = v.begin(); ii != v.end(); ++ii, first = false)
    {
        if (!first) os << ",";
        os << " " << *ii;
    }
    os << "]";
    return os;
}
////////////////////////////////

void populateColorPalette(std::vector<cv::Scalar>& colors, int nColors) 
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis1(64, 200);
    std::uniform_int_distribution<> dis2(100, 255);
    std::uniform_int_distribution<> dis3(100, 255);

    for (int i = 0; i < nColors; ++i) {
        colors.push_back(cv::Scalar(dis1(gen), dis2(gen), dis3(gen)));
    }
}


PoseNet::PoseNet()
{
}

int PoseNet::init(std::string& _protoFile, std::string& _weightsFile, int _gpu)
{
    //set_mode(_mode);

    protoFile = _protoFile;
    weightsFile = _weightsFile;

    if (_gpu)
    {
        inputNet = readNet(protoFile, weightsFile);
        inputNet.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        inputNet.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    }
    else
    {
        //inputNet = readNet(
        //    "C:/Programming/repos/WinPose/WinPose/pose/coco/pose_deploy_linevec.prototxt",
        //    "C:/Programming/repos/WinPose/WinPose/pose/coco/pose_iter_440000.caffemodel");
        inputNet = readNet(protoFile, weightsFile);
        inputNet.setPreferableBackend(DNN_TARGET_CPU);
    }
    return 0;
} 

int PoseNet::init(const char* _protoFile, const char* _weightsFile, int _gpu)
{
    std::string s1 = _protoFile;
    std::string s2 = _weightsFile;
    init(s1, s2, _gpu);
    return 0;
}
HBITMAP PoseNet::run_pose_multi(const char* _imageFile)
{
    std::string _tmps = _imageFile;
    return run_pose_multi((_tmps));
}


void getKeyPoints(cv::Mat& probMap, double threshold, std::vector<KEYPoint>& keyPoints) 
{
    cv::Mat smoothProbMap;
    cv::GaussianBlur(probMap, smoothProbMap, cv::Size(3, 3), 0, 0);

    cv::Mat maskedProbMap;
    cv::threshold(smoothProbMap, maskedProbMap, threshold, 255, cv::THRESH_BINARY);

    maskedProbMap.convertTo(maskedProbMap, CV_8U, 1);

    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(maskedProbMap, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    for (int i = 0; i < contours.size(); ++i) {
        cv::Mat blobMask = cv::Mat::zeros(smoothProbMap.rows, smoothProbMap.cols, smoothProbMap.type());

        cv::fillConvexPoly(blobMask, contours[i], cv::Scalar(1));

        double maxVal;
        cv::Point maxLoc;

        cv::minMaxLoc(smoothProbMap.mul(blobMask), 0, &maxVal, 0, &maxLoc);

        keyPoints.push_back(KEYPoint(maxLoc, probMap.at<float>(maxLoc.y, maxLoc.x)));
    }
}


void splitNetOutputBlobToParts(cv::Mat& netOutputBlob, const cv::Size& targetSize, std::vector<cv::Mat>& netOutputParts) 
{
    int nParts = netOutputBlob.size[1];
    int h = netOutputBlob.size[2];
    int w = netOutputBlob.size[3];

    for (int i = 0; i < nParts; ++i) {
        cv::Mat part(h, w, CV_32F, netOutputBlob.ptr(0, i));

        cv::Mat resizedPart;

        cv::resize(part, resizedPart, targetSize);

        netOutputParts.push_back(resizedPart);
    }
}

void PoseNet::splitNetOutputBlobToParts(cv::Mat& input, std::vector<cv::Mat>& netOutputParts)
{
    ::splitNetOutputBlobToParts(netOutputBlob, cv::Size(input.cols, input.rows), netOutputParts);
}

void populateInterpPoints(const cv::Point& a, const cv::Point& b, int numPoints, std::vector<cv::Point>& interpCoords)
{
    float xStep = ((float)(b.x - a.x)) / (float)(numPoints - 1);
    float yStep = ((float)(b.y - a.y)) / (float)(numPoints - 1);

    interpCoords.push_back(a);

    for (int i = 1; i < numPoints - 1; ++i) {
        interpCoords.push_back(cv::Point(a.x + (int)xStep * i, a.y + (int)yStep * i));
    }

    interpCoords.push_back(b);
}


void getValidPairs(
    const std::vector<std::pair<int, int>> mapIdx,
    const std::vector<std::pair<int, int>> posePairs,
    const std::vector<cv::Mat>& netOutputParts,
    const std::vector<std::vector<KEYPoint>>& detectedKeypoints,
    std::vector<std::vector<ValidPair>>& validPairs,
    std::set<int>& invalidPairs
){
    int nInterpSamples = 10;
    float pafScoreTh = 0.1f;
    float confTh = 0.7f;

    for (int k = 0; k < mapIdx.size(); ++k) {

        //A->B constitute a limb
        cv::Mat pafA = netOutputParts[mapIdx[k].first];
        cv::Mat pafB = netOutputParts[mapIdx[k].second];

        //Find the keypoints for the first and second limb
        const std::vector<KEYPoint>& candA = detectedKeypoints[posePairs[k].first];
        const std::vector<KEYPoint>& candB = detectedKeypoints[posePairs[k].second];

        int nA = (int)candA.size();
        int nB = (int)candB.size();

        /*
          # If keypoints for the joint-pair is detected
          # check every joint in candA with every joint in candB
          # Calculate the distance vector between the two joints
          # Find the PAF values at a set of interpolated points between the joints
          # Use the above formula to compute a score to mark the connection valid
         */

        if (nA != 0 && nB != 0) {
            std::vector<ValidPair> localValidPairs;

            for (int i = 0; i < nA; ++i) {
                int maxJ = -1;
                float maxScore = -1;
                bool found = false;

                for (int j = 0; j < nB; ++j) {
                    std::pair<float, float> distance((float)(candB[j].point.x - candA[i].point.x), (float)(candB[j].point.y - candA[i].point.y));

                    float norm = std::sqrt(distance.first * distance.first + distance.second * distance.second);

                    if (!norm) {
                        continue;
                    }

                    distance.first /= norm;
                    distance.second /= norm;

                    //Find p(u)
                    std::vector<cv::Point> interpCoords;
                    populateInterpPoints(candA[i].point, candB[j].point, nInterpSamples, interpCoords);
                    //Find L(p(u))
                    std::vector<std::pair<float, float>> pafInterp;
                    for (int l = 0; l < interpCoords.size(); ++l) {
                        pafInterp.push_back(
                            std::pair<float, float>(
                                pafA.at<float>(interpCoords[l].y, interpCoords[l].x),
                                pafB.at<float>(interpCoords[l].y, interpCoords[l].x)
                            ));
                    }

                    std::vector<float> pafScores;
                    float sumOfPafScores = 0;
                    int numOverTh = 0;
                    for (int l = 0; l < pafInterp.size(); ++l) {
                        float score = pafInterp[l].first * distance.first + pafInterp[l].second * distance.second;
                        sumOfPafScores += score;
                        if (score > pafScoreTh) {
                            ++numOverTh;
                        }

                        pafScores.push_back(score);
                    }

                    float avgPafScore = sumOfPafScores / ((float)pafInterp.size());

                    if (((float)numOverTh) / ((float)nInterpSamples) > confTh) {
                        if (avgPafScore > maxScore) {
                            maxJ = j;
                            maxScore = avgPafScore;
                            found = true;
                        }
                    }

                }/* j */

                if (found) {
                    localValidPairs.push_back(ValidPair(candA[i].id, candB[maxJ].id, maxScore));
                }

            }/* i */

            validPairs.push_back(localValidPairs);

        }
        else {
            invalidPairs.insert(k);
            validPairs.push_back(std::vector<ValidPair>());
        }
    }/* k */
}

void PoseNet::getValidPairs(const std::vector<cv::Mat>& netOutputParts,
    const std::vector<std::vector<KEYPoint>>& detectedKeypoints,
    std::vector<std::vector<ValidPair>>& validPairs,
    std::set<int>& invalidPairs)
{
    ::getValidPairs(
        mapIdx,
        posePairs,
        netOutputParts,
        detectedKeypoints,
        validPairs,
        invalidPairs);
}

void getPersonwiseKeypoints(
    const std::vector<std::pair<int, int>> mapIdx,
    const std::vector<std::pair<int, int>> posePairs,
    const std::vector<std::vector<ValidPair>>& validPairs,
    const std::set<int>& invalidPairs,
    std::vector<std::vector<int>>& personwiseKeypoints) 
{
    for (int k = 0; k < mapIdx.size(); ++k)
    {
        if (invalidPairs.find(k) != invalidPairs.end())
        {
            continue;
        }

        const std::vector<ValidPair>& localValidPairs(validPairs[k]);

        int indexA(posePairs[k].first);
        int indexB(posePairs[k].second);

        for (int i = 0; i < localValidPairs.size(); ++i)
        {
            bool found = false;
            int personIdx = -1;

            for (int j = 0; !found && j < personwiseKeypoints.size(); ++j)
            {
                if (indexA < personwiseKeypoints[j].size() &&
                    personwiseKeypoints[j][indexA] == localValidPairs[i].aId)
                {
                    personIdx = j;
                    found = true;
                }
            }/* j */

            if (found)
            {
                personwiseKeypoints[personIdx].at(indexB) = localValidPairs[i].bId;
            }
            else if (k < 17) {
                std::vector<int> lpkp(std::vector<int>(18, -1));

                lpkp.at(indexA) = localValidPairs[i].aId;
                lpkp.at(indexB) = localValidPairs[i].bId;

                personwiseKeypoints.push_back(lpkp);
            }
        }/* i */
    }/* k */
}

void PoseNet::getPersonwiseKeypoints(
    const std::vector<std::vector<ValidPair>>& validPairs,
    const std::set<int>& invalidPairs,
    std::vector<std::vector<int>>& personwiseKeypoints)
{
    ::getPersonwiseKeypoints(
        mapIdx,
        posePairs,
        validPairs,
        invalidPairs,
        personwiseKeypoints
    );
}


cv::Mat convertToGrayscale(const cv::Mat& colorImage) {
    // 結果を格納するMatオブジェクトを初期化（同じサイズで1チャンネルのグレースケール画像）
    cv::Mat grayImage(colorImage.rows, colorImage.cols, CV_8UC1);

    for (int y = 0; y < colorImage.rows; ++y) {
        for (int x = 0; x < colorImage.cols; ++x) {
            // 各ピクセルのBGR値を取得
            cv::Vec3b pixel = colorImage.at<cv::Vec3b>(y, x);

            // RGB値の平均を計算
            uchar average = (pixel[0] + pixel[1] + pixel[2]) / 3;

            // 新しい画像の同じ位置に平均値を設定
            grayImage.at<uchar>(y, x) = average;
        }
    }
    return grayImage;
}

// カラー画像の彩度を落とす関数
cv::Mat reduceSaturation(const cv::Mat& inputImage, float saturationScale) {
    CV_Assert(inputImage.type() == CV_8UC3); // 入力画像がBGR形式であることを確認

    // HSV色空間に変換
    cv::Mat hsvImage;
    cv::cvtColor(inputImage, hsvImage, cv::COLOR_BGR2HSV);

    // HSV画像をチャンネルごとに分割
    std::vector<cv::Mat> channels(3);
    cv::split(hsvImage, channels);

    // 彩度チャンネル（channels[1]）の値を調整
    channels[1] = channels[1] * saturationScale;

    // チャンネルをマージしてHSV画像を再構成
    cv::merge(channels, hsvImage);

    // BGR色空間に戻す
    cv::Mat outputImage;
    cv::cvtColor(hsvImage, outputImage, cv::COLOR_HSV2BGR);

    return outputImage;
}

cv::Mat PoseNet::run_pose_multi(cv::Mat& input, int _output_image_mode)
{
    //input = _input;
    cv::Mat frameCopy = input.clone();
    frameWidth = input.cols;
    frameHeight = input.rows;

    //int inWidth = 368;
    //int inHeight = 368;
    //float thresh = 0.1f;

    //cv::Mat 
    //inputBlob = cv::dnn::blobFromImage(frameCopy, 1.0 / 255.0, cv::Size((int)((368 * frameCopy.cols) / frameCopy.rows), 368), cv::Scalar(0, 0, 0), false, false);
    inputBlob = cv::dnn::blobFromImage(input, 1.0 / 255.0, cv::Size((int)((368 * input.cols) / input.rows), 368), cv::Scalar(0, 0, 0), false, false);
    //  AIに入れる前の画像の前処理
    //  画像から4次元のブロブを作成します。オプションでサイズ変更とクロップimage中心からのリサイズ、クロップ、値の減算mean値の減算，値のスケーリングscalefactor青と赤のチャンネルを入れ替えます。
    //    引数
    //    image         入力画像（1，3，4チャンネル）．
    //    scalefactor	の乗数image値の乗数．
    //    size	        出力画像の空間サイズ
    //    mean	        チャンネルから減算される平均値を持つスカラー．の場合，値は(mean - R, mean - G, mean - B) の順になるように意図されています．imageがBGR順でありswapRBが真であれば
    //    swapRB	    3チャンネル画像の最初と最後のチャンネルを入れ替える必要があることを示すフラグ．
    //    crop	        リサイズ後に画像をクロップするかどうかを示すフラグ．
    //    ddepth	    出力されるblobの深さ．CV_32F または CV_8U を選択します．
    //                  もしcropが真の場合，入力画像はリサイズされ，リサイズ後の片側が対応する次元と等しくなり，もう片側が同等以上になります．
    //                  sizeの対応する寸法と等しくなり，もう片方の寸法は同等かそれ以上になります．
    //                  そして，中央からの切り出しが行われます．
    //                  もしcropが偽の場合，アスペクト比を維持したままクロップを行わない直接のリサイズが行われます．
    //    戻り値
    //    4次元MatNCHWの次元順で作成します。

    inputNet.setInput(inputBlob);
    // データのセット

    //cv::Mat 
    //TRYCAT_CV_STD(netOutputBlob = inputNet.forward())
    netOutputBlob = inputNet.forward();
    // 順伝播
    //https://stackoverflow.com/questions/31650201/opencv-error-assertion-failed-empty-in-java-and-opencv-3-0-and-what-does-i
    //https://stackoverflow.com/questions/69161066/assertion-failed-empty-in-cvcascadeclassifierdetectmultiscale


    std::vector<cv::Mat> netOutputParts;

    //splitNetOutputBlobToParts(netOutputBlob, cv::Size(input.cols, input.rows), netOutputParts);
    splitNetOutputBlobToParts(input, netOutputParts);
    //std::chrono::time_point<std::chrono::system_clock> finishTP = std::chrono::system_clock::now();
    //std::cout << "Time Taken in forward pass = " << std::chrono::duration_cast<std::chrono::milliseconds>(finishTP - startTP).count() << " ms" << std::endl;

    int keyPointId = 0;

    //これは計算のたびに初期化必要????
    std::vector<std::vector<KEYPoint>> detectedKeypoints;
    std::vector<KEYPoint> keyPointsList;

    for (int i = 0; i < nPoints; ++i)
    {
        std::vector<KEYPoint> keyPoints;
        getKeyPoints(netOutputParts[i], 0.1, keyPoints);
        //std::cout << "Keypoints - " << keypointsMapping[i] << " : " << keyPoints << std::endl;

        for (int i = 0; i < keyPoints.size(); ++i, ++keyPointId)
        {
            keyPoints[i].id = keyPointId;
        }

        detectedKeypoints.push_back(keyPoints);
        keyPointsList.insert(keyPointsList.end(), keyPoints.begin(), keyPoints.end());
    }

    std::vector<cv::Scalar> colors;
    populateColorPalette(colors, nPoints);

    cv::Mat outputFrame;
    if (_output_image_mode == OUTPUT_IMAGE_MODE_GRAY)
        //cv::cvtColor(input.clone(), outputFrame, cv::COLOR_BGR2GRAY);
        //outputFrame=convertToGrayscale(input.clone()); //ビット深度に違いが出るため、Win32apiでは正常に表示できない
        outputFrame = reduceSaturation(input.clone(), 0.02F);
    else
        outputFrame = input.clone();

    for (int i = 0; i < nPoints; ++i)
    {
        for (int j = 0; j < detectedKeypoints[i].size(); ++j)
        {
            cv::circle(outputFrame, detectedKeypoints[i][j].point, (int)pose_joint_circle, colors[i], -1, cv::LINE_AA);
        }
    }

    std::vector<std::vector<ValidPair>> validPairs;
    std::set<int> invalidPairs;
    getValidPairs(netOutputParts, detectedKeypoints, validPairs, invalidPairs);

    std::vector<std::vector<int>> personwiseKeypoints;
    getPersonwiseKeypoints(validPairs, invalidPairs, personwiseKeypoints);

    for (int i = 0; i < nPoints - 1; ++i)
    {
        for (int n = 0; n < personwiseKeypoints.size(); ++n)
        {
            const std::pair<int, int>& posePair = posePairs[i];
            int indexA = personwiseKeypoints[n][posePair.first];
            int indexB = personwiseKeypoints[n][posePair.second];

            if (indexA == -1 || indexB == -1)
            {
                continue;
            }

            const KEYPoint& kpA = keyPointsList[indexA];
            const KEYPoint& kpB = keyPointsList[indexB];

            //cv::line(outputFrame, kpA.point, kpB.point, colors[i], (int)pose_link_line, cv::LINE_AA);
            cv::line(outputFrame, kpA.point, kpB.point, LINECOLOR, (int)pose_link_line, cv::LINE_AA);
            
        }
    }
    return outputFrame;
}

HBITMAP PoseNet::run_pose_multi(std::string& _imageFile)
{
    cv::Mat input = imread(_imageFile);
    //cv::imshow("Detected Pose", outputFrame);
    cv::Mat outputFrame = run_pose_multi(input);
    return MatToBitmap(outputFrame);
}


// Function to convert cv::Mat to BITMAP
HBITMAP MatToBitmap(const cv::Mat& mat) 
{
    //int bpp = (int)8 * mat.elemSize();
    size_t bpp = 8 * mat.elemSize();
    assert((bpp == 8 || bpp == 24 || bpp == 32));

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = mat.cols;
    bmi.bmiHeader.biHeight = -mat.rows; // Negative indicates top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = (WORD)bpp;
    bmi.bmiHeader.biCompression = BI_RGB;

    HBITMAP hBitmap = CreateDIBitmap(GetDC(NULL), &bmi.bmiHeader, CBM_INIT, mat.data, &bmi, DIB_RGB_COLORS);

    return hBitmap;
}
