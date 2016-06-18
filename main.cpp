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
void generateGUI()
{
    int baseWidth, baseHeight;
    namedWindow( "Original", WINDOW_NORMAL );
    namedWindow( "Binary", WINDOW_NORMAL );
    namedWindow( "Control", WINDOW_NORMAL );
    //base width and height better position windows for debugging purpose
    baseWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    baseHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    resizeWindow("Original", baseWidth, baseHeight);
    resizeWindow("Binary", baseWidth, baseHeight);
    resizeWindow("Control", 1280, 50);
    createTrackbar("Frame", "Control", &videoPos, cap.get(CV_CAP_PROP_FRAME_COUNT));
    moveWindow("Original", 0, 0);
    moveWindow("Binary", baseWidth, 0);
    moveWindow("Control", 0, baseHeight+25);
}
void updateGUI()
{
    imshow("Original", frame);
    imshow("Binary", binaryFrame);
    //update time trackbar
    if(cap.get(CV_CAP_PROP_POS_FRAMES)- videoPos != 1)
    {
        cap.set(CV_CAP_PROP_POS_FRAMES, videoPos);
    }
    else
    {
        videoPos++;
        setTrackbarPos("Frame", "Control", videoPos);
    }
}
string convertInt(int number)
{
    stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}
void findRectSpec (vector<Point> rectContour)
{
    //get the moments and centers of contours
    Moments mu = moments( rectContour, false );
    Point2f cXY = Point2f(mu.m10/mu.m00, mu.m01/mu.m00);

    //find the rotated rectangle & get its 4 vertices
    RotatedRect minRect = minAreaRect( Mat(rectContour) );
    Point2f rect_points[4];
    minRect.points( rect_points );

    //find the width & height of the rotated rectangle
    float rectWidth = max(minRect.size.width,minRect.size.height);
    float rectHeight = min(minRect.size.width,minRect.size.height);

    //find midpoints of 4 sides of the rectangle
    Point2f mp1 = Point2f((rect_points[0].x + rect_points[1].x) / 2, (rect_points[0].y + rect_points[1].y) / 2);
    Point2f mp2 = Point2f((rect_points[2].x + rect_points[3].x) / 2, (rect_points[2].y + rect_points[3].y) / 2);
    Point2f mp3 = Point2f((rect_points[1].x + rect_points[2].x) / 2, (rect_points[1].y + rect_points[2].y) / 2);
    Point2f mp4 = Point2f((rect_points[3].x + rect_points[0].x) / 2, (rect_points[3].y + rect_points[0].y) / 2);

    //compare the midpoints to find the one that will be used to calculate the angle
    Point2f pXY;
    if (sqrt(pow(mp2.y-mp1.y,2.0)+pow(mp2.x-mp1.x,2.0)) > sqrt(pow(mp4.y-mp3.y,2.0)+pow(mp4.x-mp3.x,2.0)))
    {
        if (mp1.y < mp2.y)
            pXY = mp1;
        else
            pXY = mp2;
    }
    else
    {
        if (mp3.y<mp4.y)
            pXY = mp3;
        else
            pXY = mp4;
    }

    //calculate the angle & add 180 degrees to negative result of obtuse angle
    float angle = int(atan((cXY.y-pXY.y)/(pXY.x-cXY.x))*180/M_PI);
    if (angle < 0 )
        angle = angle + 180;

    //draw the rotated rectangle
    for( int j = 0; j < 4; j++ )
        line( frame, rect_points[j], rect_points[(j+1)%4], Scalar(0, 0, 255), 2, 8 );

    //draw the horizontal line & angle line
    line(frame, pXY, cXY, Scalar(0, 0, 255), 2, 8 );
    line(frame, cXY, Point2f(cXY.x + 100,cXY.y), Scalar(0, 0, 255), 2, 8 );

    //draw the center, note the center coordinates, width, height & angle of the rectangle
    circle(frame, cXY, 7, Scalar(0, 0, 255), -1);
    putText(frame, "Center: (" + convertInt(cXY.x) + " , " + convertInt(cXY.y) + ")", Point2f(cXY.x+10, cXY.y+20), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 0, 0), 2);
    putText(frame, "Width: " + convertInt(rectWidth) + " , Height: " + convertInt(rectHeight) , Point2f(cXY.x+10, cXY.y+40), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 0, 0), 2);
    putText(frame, "Angle(horizontal): " + convertInt(angle) + " degrees", Point2f(cXY.x+10, cXY.y+60), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 0, 0), 2);
    putText(frame, "Angle(to turn): " + convertInt(90-angle) + " degrees", Point2f(cXY.x+10, cXY.y+80), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 0, 0), 2);
}
//const int PAUSE = ' ';
//const int STOP = 27;
//void respondToKey(int waitTimeMs){
//    static boolean isPaused = false;
//    int keyPressed = waitKey(waitTimeMS);
//    //user wants it to pause
//    if(keyPressed == PAUSE) && !isPaused{
//        respondToKey(0);
//        isPaused = !isPaused;
//    //user wants it to resume
//    }else if(keyPressed == PAUSE) && !isPaused{
//        return;
//        isPaused = !isPaused;
//    }else if()
//}
bool isPaused = false;
int main(int argc, char *argv[])
{
    cap =  VideoCapture("../../sample videos/Pipe.avi");
    generateGUI();
    if(!cap.isOpened())
    {
        return -1;
    }
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
//    setTrackbarPos("Frame", "Control", 600);
    while(true)
    {
        cap >> frame;
        cvtColor(frame, hsvFrame, CV_BGR2HSV);
        inRange(hsvFrame,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),binaryFrame);
        erode(binaryFrame, binaryFrame, erodeKernel);
        dilate(binaryFrame, binaryFrame, dilateKernel);
        findContours( binaryFrame.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
//        vector<vector<Point> > polygonContours;
        for(int i = 0; i < contours.size(); i++)
        {
//            double contourPerimeter = arcLength(contours[i], true);
//            vector <Point> approx;
//            approxPolyDP(contours[i], approx, contourPerimeter*0.02, true);
//            polygonContours.insert(polygonContours.begin(), approx);
            findRectSpec(contours[i]);
        }
//        drawContours( frame, polygonContours, -1, Scalar(0,0,0), 3);
        updateGUI();
        int userPressedKey = waitKey(30);
        cout << userPressedKey << endl;
        do{
            if(userPressedKey == 1048603){//escape key
                return 0;
            }else if(userPressedKey == 1048608 && !isPaused){ //space key and pause
                isPaused = !isPaused;
                userPressedKey = waitKey(0);
            }else if(userPressedKey == 1048608){ //space key and resume
                isPaused = !isPaused;
                break;
            }
//            else if(userPressedKey == 1113939){ //right key
//                videoPos+=10;
//                break;
//            }else if(userPressedKey == 1113937){ //left key
//                videoPos--;
//                break;
//            }
        }while(isPaused);


    }
    return 0;
}



