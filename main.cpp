#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
using namespace cv;
using namespace std;

int H_MIN = 0;
int H_MAX = 50;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 200;
int V_MAX = 256;
Mat erodeKernel = getStructuringElement( MORPH_RECT,
                                       Size( 11, 11));
Mat dilateKernel = erodeKernel;
int main(int argc, char *argv[])
{

    VideoCapture cap("/home/mong/Desktop/Pipe.avi");
    if(!cap.isOpened()){
        return -1;
    }
    namedWindow( "Original", CV_WINDOW_AUTOSIZE );
    namedWindow( "Binary", CV_WINDOW_AUTOSIZE );
    moveWindow("Original", 0, 0);
    moveWindow("Binary", 640, 0);
    Mat frame;
    Mat hsvFrame;
    Mat binaryFrame;
    vector<vector<Point> > contours;
    vector<vector<Point> > polygonContours;
    vector<Vec4i> hierarchy;
    while(true){
        cap >> frame;
        cvtColor(frame, hsvFrame, CV_BGR2HSV);
        inRange(hsvFrame,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),binaryFrame);
        erode(binaryFrame, binaryFrame, erodeKernel);
        dilate(binaryFrame, binaryFrame, dilateKernel);
        findContours( binaryFrame.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
        for(int i = 0; i < contours.size(); i++)
        {
            double contourPerimeter = arcLength(contours[i], true);
            vector <Point> approx;
            approxPolyDP(contours[i], approx, contourPerimeter*0.02, true);
            polygonContours.insert(polygonContours.begin(), approx);
            drawContours( frame, polygonContours, i, Scalar(0,0,0), 3);
        }
        imshow("Original", frame);
        imshow("Binary", binaryFrame);
        if(waitKey(30) >= 0) break;
    }

    waitKey(0);
//    while(true){
//
//    }
    return 0;
}



