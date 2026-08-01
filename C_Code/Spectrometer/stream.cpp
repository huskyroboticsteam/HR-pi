#include <opencv2/opencv.hpp>
// #include "/home/robot/HR-pi/C_Code/functions.h"
// #include <vector>
//g++ stream.cpp -o stream `pkg-config --cflags --libs opencv4`
#define SCREEN_WIDTH 2880
#define SCREEN_HEIGHT 1800
int char_to_int(char * arg){
    float res = 0;
    uint8_t negative = *arg=='-';
    uint8_t base = 10;
    if (*(arg+negative)=='0'){
        if (*(arg+negative+1)=='b'){
            base=2;
        } else if (*(arg+negative+1)=='x'){
            base=16;
        }
    }
    int curr_num=0;
    for (int i=negative+2*(base!=10); *(arg+i)!='\0'; i++){
            curr_num=(int)(*(arg+i))-(int)'0';
            if (curr_num>9){
                curr_num-=39;
            }
            res*=base;
            res+=curr_num;
    }
    return (negative ? -1*res : res);
}
void intparse (int argc, char** args, int * output){
    for (int i=0; i<argc; i++){
        *(output+i)=char_to_int(*(args+i));
    }
}
int main(int argc, char** argv) {
    int vals[argc-1];
    intparse(argc-1, argv+1, vals);
    cv::VideoCapture cap(argv[1], cv::CAP_V4L2);  // camera 
    
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y','U','Y','V'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(cv::CAP_PROP_FPS, vals[1]);   // lower FPS if needed
    cap.set(cv::CAP_PROP_CONVERT_RGB, 0);
    const uint32_t width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    const uint32_t height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::namedWindow("Frame", cv::WINDOW_NORMAL);
    const float horiz_scale = (SCREEN_WIDTH/width);
    const float vert_scale = (SCREEN_HEIGHT/height);
    const float window_scailing = horiz_scale>vert_scale ? vert_scale : horiz_scale;
    const uint32_t scaled_width = (uint32_t)(window_scailing*width);
    const uint32_t scaled_height = (uint32_t)(window_scailing*height);
    // printf("%i, %i, %i, %i\n", scaled_width, scaled_height, width, height);
    cv::Mat resized_image(cv::Size(scaled_width, scaled_height),CV_8UC1);
    if (!cap.isOpened()) {
        std::cout << "Camera not opened\n";
        return -1;
    }

    while(true) {
        cv::Mat frame;
        cap >> frame;
        if(frame.empty()) break;
        cv::cvtColor(frame, frame, cv::COLOR_YUV2GRAY_YUYV);
        cv::resize(frame, resized_image, cv::Size(scaled_width, scaled_height));
        cv::imshow("Frame", resized_image);
        if(cv::waitKey(1) == 'q')
            break;
    }
}