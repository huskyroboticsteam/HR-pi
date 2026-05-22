#include <opencv2/opencv.hpp>
// #include "/home/robot/HR-pi/C_Code/functions.h"
// #include <vector>
//g++ stream_spectra.cpp -o stream_spec `pkg-config --cflags --libs opencv4`
//v4l2-ctl -d /dev/video0 --list-ctrls
// v4l2-ctl -d /dev/video0 -c exposure_time_absolute=1000
#define SCREEN_WIDTH 2880
#define SCREEN_HEIGHT 1800

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <deque>
#include <numeric>
#include <cmath>
using namespace std;


double find_peaks(int start, int end, vector<int> column_data){
    double scale = column_data.size() / (660-440);
    start = column_data.size() - ((start - 440) * scale);
    end = column_data.size() - ((end - 440) * scale);
    int peak_value = 0;
    for(int i = end; i <= start; i++){
        if(column_data[i] > peak_value){
            peak_value = column_data[i];
        }
    }

    return peak_value;
}

double find_abs_peaks(int start, int end, vector<int> column_data, vector<double> dark, vector<double> light){
    double scale = column_data.size() / (660-440);
    start = column_data.size() - ((start - 440) * scale);
    end = column_data.size() - ((end - 440) * scale);
    double dark_val = 0.0;
    double light_val = 0.0;
    int peak_value = 0;
    for(int i = end; i <= start; i++){
        if(column_data[i] > peak_value){
            peak_value = column_data[i];
        }
        dark_val += dark[i];
        light_val += light[i];
    }

    dark_val /= (start-end);
    light_val /= (start-end);

    //cout<<peak_value<<": peak"<<'\n';
    double absorbance = -log10((peak_value - dark_val)/(light_val - dark_val));
    //cout<<absorbance<<": abs"<<'\n';

    return absorbance;
}

vector<double> load_calibration(const string& filename) {
    vector<double> data;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Failed to open calibration file: " << filename << "\n";
        return data;
    }

    string line;
    if (getline(file, line)) {
        stringstream ss(line);
        string value;

        while (getline(ss, value, ',')) {
            try {
                data.push_back(stoi(value));
            } catch (...) {
                cerr << "Invalid number in calibration file\n";
            }
        }
    }

    return data;
}

int char_to_int(char * arg){
    double res = 0;
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

void save_calibration(const std::vector<double>& data, const std::string& filename) {
    std::ofstream file(filename);
    for (size_t i = 0; i < data.size(); i++) {
        file << data[i];
        if (i != data.size() - 1) file << ",";
    }
    file << "\n";
}

// scale pixels (# pixels in a nm)
double scale = (double)SCREEN_WIDTH / (660-440);
// iterate through frames, finding the peak intensity at each frame
// push this peak intensity and it's index (wavelength) to separate stacks
// find peaks, then the corresponding index. scale index to determine wavelength of peak


// for each frame, iterate through intensity values (y values) to find peak of the frame.
// push the intensity (y) and its wavelength (x) to a map (or each to two separate stacks?)
// of the intensities, return the highest one within a given range of wavelengths.
// scale the wavelength and return that too


// function return peak within a given range of wavelengths:

void on_quit(vector<int> averages, vector<double> dark, vector<double> light) {

    double red_peak = find_abs_peaks(625, 660, averages, dark, light);
    double orange_peak = find_abs_peaks(590, 625, averages, dark, light);
    double yellow_peak = find_abs_peaks(565, 590, averages, dark, light);
    double green_peak = find_abs_peaks(500, 565, averages, dark, light);
    double cyan_peak = find_abs_peaks(485, 500, averages, dark, light);
    double blue_peak = find_abs_peaks(440, 485, averages, dark, light);
    vector<double> peaks = {red_peak, orange_peak, yellow_peak, green_peak, cyan_peak, blue_peak};

    save_calibration(peaks, "peaks_colors.csv");
}

int main(int argc,  char** argv) {
    int vals[argc-1];
    intparse(argc-1, argv+1, vals);
    cv::VideoCapture cap(argv[1], cv::CAP_V4L2);  // camera 
    cv::namedWindow("Frame", cv::WINDOW_NORMAL);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y','U','Y','V'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(cv::CAP_PROP_FPS, 5);   // lower FPS if needed
    cap.set(cv::CAP_PROP_CONVERT_RGB, 0);
    
    if (!cap.isOpened()) {
        cout << "Camera not opened\n";
        return -1;
    }
    cv::Mat frame;
    const uint32_t width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    const uint32_t height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    vector<int> column_data(width, 0);
    uint8_t* row;
    int scale = 1;
    int plot_h = 540;
    cv::Mat output_im(cv::Size(width, height),CV_8UC1);
    cv::Mat plot(plot_h, width, CV_8UC1, cv::Scalar(255));
    cv::namedWindow("Frame", cv::WINDOW_NORMAL);
    const double horiz_scale = (SCREEN_WIDTH/width);
    const double vert_scale = (SCREEN_HEIGHT/height);
    const double window_scailing = horiz_scale>vert_scale ? vert_scale : horiz_scale;
    const uint32_t scaled_width = (uint32_t)(window_scailing*width);
    const uint32_t scaled_height = (uint32_t)(window_scailing*height);
    printf("%i, %i, %i, %i\n", scaled_width, scaled_height, width, height);
    cv::Mat resized_image(cv::Size(scaled_width, scaled_height),CV_8UC1);
    vector<double> dark_profile = load_calibration("dark_calibration_final.csv");
    vector<double> light_profile = load_calibration("light_calibration_final.csv");

    int margin_left = 60;
    int counter = 0;
    int offset = 30;
    vector<int> averages(width);
    cap >> frame;
    
    // if(frame.empty()) break;
    cv::cvtColor(frame, frame, cv::COLOR_YUV2GRAY_YUYV);
    for (int y = 0; y < height; y++) {
        row = frame.ptr<uint8_t>(y);
        for (int x = 0; x < width; x++) {
            column_data[x] += row[x]; // Y component    
            
        }
    }
    
    for (int ii=0; ii<width; ii++) {
        //Avg sum of col values
        column_data[ii]/=height;
    }

    // ticks
    for (int i = 0; i <= 5; i++) {
        // int percent = i * 20;
        int percent = i * 20;
        double y = (plot_h - offset) - (percent * (plot_h - 2*offset) / 100); //Set top/bottom bounds for y-axis tick marks //1.3 JUST WORKS 
        //double y = (plot_h - offset)()

        cv::line(plot,
            cv::Point(margin_left - 5, y),
            cv::Point(margin_left, y),
            cv::Scalar(0), 1);

        cv::putText(plot,
            to_string(percent),
            cv::Point(5, y + 5),
            cv::FONT_HERSHEY_SIMPLEX,
            0.4,
            cv::Scalar(0), 1);
    }
    
    int window = 10;
    deque<double> y1_buffer, y2_buffer;
    for(int x=1;x<width;x++) {

        //regular plotting
        double y1 = (plot_h)-((column_data[x-1]-dark_profile[x-1])*(plot_h - 2* offset))/(light_profile[x-1]-dark_profile[x-1]); 
        double y2 = (plot_h)-((column_data[x]-dark_profile[x])*(plot_h - 2*offset))/(light_profile[x]-dark_profile[x]);
        
        //add buffers
        //shared plotting
        y1_buffer.push_back(y1);
        y2_buffer.push_back(y2);
        if (y1_buffer.size() > window) y1_buffer.pop_front();
        if (y2_buffer.size() > window) y2_buffer.pop_front();
        //compute averages (shared)
        double y1_average = accumulate(y1_buffer.begin(), y1_buffer.end(), 0.0) / y1_buffer.size();
        double y2_average = accumulate(y2_buffer.begin(), y2_buffer.end(), 0.0) / y2_buffer.size();

        //shared plotting
        y1_average -= offset;
        y2_average -= offset;
        counter++;

        averages = column_data;
        cv::line(plot,
                cv::Point((x-1),(int)(y1_average)),
                cv::Point((x),(int)(y2_average)),    
                cv::Scalar(0,0,0),1);  


            }
        cv::resize(plot, resized_image, cv::Size(scaled_width, scaled_height));
        cv::imshow("Frame", resized_image);
        
        fill(column_data.begin(), column_data.end(), 0);
        plot.setTo(255);
    
    on_quit(averages, dark_profile, light_profile);

}



