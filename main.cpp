#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
using namespace cv;
using namespace std;

//predefined constant for identifying the orange color of pathmarker
int H_MIN = 0;
int H_MAX = 50;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 200;
int V_MAX = 256;
//values used for GUI manipulation
int videoPos = 0;
//kernel used in eroding and dilating (used for erasing noise)
Mat erodeKernel = getStructuringElement( MORPH_RECT,
                                       Size( 11, 11));
Mat dilateKernel = erodeKernel;
//global objects used in main
VideoCapture cap;
Mat frame; //original one frame from video
Mat hsvFrame; //segmented frame in hsv colorspace
Mat binaryFrame; //black and white frame with white being the color of interest
void generateGUI(){
    namedWindow( "Original", CV_WINDOW_AUTOSIZE );
    namedWindow( "Binary", CV_WINDOW_AUTOSIZE );
    namedWindow( "Control", WINDOW_NORMAL );
    resizeWindow("Control", 1280, 50);
    createTrackbar("Frame", "Control", &videoPos, cap.get(CV_CAP_PROP_FRAME_COUNT));
    moveWindow("Original", 0, 0);
    moveWindow("Binary", 640, 0);
    moveWindow("Control", 0, 505);
}
void updateGUI(){
    imshow("Original", frame);
    imshow("Binary", binaryFrame);
    //update time trackbar
    if(cap.get(CV_CAP_PROP_POS_FRAMES)- videoPos != 1){
        cap.set(CV_CAP_PROP_POS_FRAMES, videoPos);
    }
    else{
        videoPos++;
        setTrackbarPos("Frame", "Control", videoPos);
    }
}
int main(int argc, char *argv[]){
    cap =  VideoCapture("../../sample videos/Pipe.avi");
    generateGUI();
    if(!cap.isOpened()){
        return -1;
    }
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    setTrackbarPos("Frame", "Control", 600);
    while(true){
        cap >> frame;
        cvtColor(frame, hsvFrame, CV_BGR2HSV);
        inRange(hsvFrame,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),binaryFrame);
        erode(binaryFrame, binaryFrame, erodeKernel);
        dilate(binaryFrame, binaryFrame, dilateKernel);
        findContours( binaryFrame.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
        vector<vector<Point> > polygonContours;
        for(int i = 0; i < contours.size(); i++)
        {
            double contourPerimeter = arcLength(contours[i], true);
            vector <Point> approx;
            approxPolyDP(contours[i], approx, contourPerimeter*0.02, true);
            polygonContours.insert(polygonContours.begin(), approx);
        }
        drawContours( frame, polygonContours, -1, Scalar(0,0,0), 3);
        updateGUI();
        if(waitKey(30) >= 0) break;
    }
    return 0;
}



