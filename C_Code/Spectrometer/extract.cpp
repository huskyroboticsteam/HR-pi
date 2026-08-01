// #include "/home/robot/HR-pi/C_Code/functions.h"
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
//g++ extract.cpp -o ~/Pictures/extract `pkg-config --cflags --libs opencv4`
// using namespace std;
int main(int argc, char** argv) {
    // cv::VideoCapture cap("/dev/video0", cv::CAP_V4L2);  // camera 
    
    // cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y','U','Y','V'));
    // cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    // cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    // cap.set(cv::CAP_PROP_FPS, 5);   // lower FPS if neededindex 0
    // cap.set(cv::CAP_PROP_CONVERT_RGB, 0);
    
    // if (!cap.isOpened()) {
    //     std::cout << "Camera not opened\n";
    //     return -1;
    // }

    cv::Mat frame = imread(argv[1], cv::IMREAD_GRAYSCALE);
    
    // cap >> frame;
    
    if (frame.empty()) return 1;
    // cv::cvtColor(frame, frame, cv::COLOR_YUV2GRAY_YUYV);

    std::vector<int> column_data(frame.cols);
    uint8_t* data = frame.ptr<uint8_t>(0);
    for (int y = 0; y < frame.rows; y++) {
        uint8_t* row = frame.ptr<uint8_t>(y);
        for (int x = 0; x < frame.cols; x++) {
            // for (int ii=0; ii<frame.channels(); ii++) {
                column_data[x] += row[x]; // Y component
            // }
            
        }
    }
    for (int ii=0; ii<frame.cols; ii++) {
        column_data[ii]/=frame.rows;
    }
    // cv::Mat output_im(frame.rows, frame.cols, CV_32FC1, );
    // std::vector<std::vector<int>> output_im(frame.rows, std::vector<int>(frame.cols));
    int scale = 4;
    int plot_h = 1080*scale;
    cv::Mat output_im(cv::Size(frame.cols, frame.rows),CV_8UC1);
    cv::Mat plot(plot_h, frame.cols*scale, CV_8UC1, cv::Scalar(255));
    for (int y = 0; y < frame.rows; y++) {
        unsigned char* row = output_im.ptr<unsigned char>(y);
        for (int x = 0; x < frame.cols; x++) {
            row[x] = column_data[x];
        }
    }
    
    for(int x=1;x<frame.cols;x++) {

        int y1 = plot_h-(column_data[x-1]*plot_h)/255;
        int y2 = plot_h-(column_data[x]*plot_h)/255;

        cv::line(plot,
                 cv::Point((x-1)*scale,y1),
                 cv::Point((x)*scale,y2),
                 cv::Scalar(0,0,0),4);
    }
    // cv::Mat img(100, 100, CV_32FC1, data);
    cv::imwrite("out.tiff", output_im);
    cv::imwrite("plot.png", plot);
    // cv::Mat output(frame.rows, frame.cols, CV_32FC1, output_im);
    printf("%i\n",column_data[1]);

    return 0;
}
