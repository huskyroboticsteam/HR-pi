#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <thread>
#include <chrono>
using namespace std;

//To Compile:
//g++ find_peaks.cpp -o calib `pkg-config --cflags --libs opencv4`

//To get calibration values:
//./calib /dev/video0

// Save calibration to CSV
void save_calibration(const std::vector<int>& data, const std::string& filename) {
    std::ofstream file(filename);
    for (size_t i = 0; i < data.size(); i++) {
        file << data[i];
        if (i != data.size() - 1) file << ",";
    }
    file << "\n";
}

// Compute column averages
std::vector<int> compute_column_average(const cv::Mat& frame) {
    int width = frame.cols;
    int height = frame.rows;

    std::vector<int> column_data(width, 0);

    for (int y = 0; y < height; y++) {
        const uint8_t* row = frame.ptr<uint8_t>(y);
        for (int x = 0; x < width; x++) {
            column_data[x] += row[x];
        }
    }

    for (int x = 0; x < width; x++) {
        column_data[x] /= height;
    }

    return column_data;
}

// Capture dark calibration
std::vector<int> calibrate(cv::VideoCapture& cap, int frames) {
    cv::Mat frame;
    std::vector<int> avg;

    for (int i = 0; i < frames; i++) {
        cap >> frame;
        if (frame.empty()) continue;

        cv::cvtColor(frame, frame, cv::COLOR_YUV2GRAY_YUYV);

        std::vector<int> curr = compute_column_average(frame);

        if (avg.empty()) {
            avg = curr;
        } else {
            for (size_t j = 0; j < curr.size(); j++) {
                avg[j] += curr[j];
            }
        }
    }

    for (size_t j = 0; j < avg.size(); j++) {
        avg[j] /= frames;
    }

    return avg;
}



int main(int argc, char** argv) {
    // Allow camera path like your main file (e.g. /dev/video0)
    const char* device = (argc > 1) ? argv[1] : "0";

    // 
    std::string arg1 = argv[1];

    cv::VideoCapture cap; 

    // Handle numeric vs string device
    if (std::isdigit(device[0])) {
        cap.open(std::stoi(device), cv::CAP_V4L2);
    } else {
        cap.open(device, cv::CAP_V4L2);
    }

    if (!cap.isOpened()) {
        std::cerr << "Camera not opened\n";
        return -1;
    }

    // MATCH YOUR MAIN FILE SETTINGS
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y','U','Y','V'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(cv::CAP_PROP_FPS, 5);
    cap.set(cv::CAP_PROP_CONVERT_RGB, 0);

    //Dark cali

    std::cout << "Cover the lens for dark calibration...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5)); // give time to cover lens

    // dark actually means data
    std::vector<int> dark = calibrate(cap, 10);

    // saves calibration data to different file name depending on camera input
    save_calibration(dark, "calibration.csv");

    std::cout << "Saved calibration.csv\n";

    // find peaks
    std::vector<double> values = readCSV("calibration.csv");
    std::vector<int> peaks = findPeaks(values);

    return 0;
}


std::vector<double> readCSV(string filename) {
    std::vector<double> values;
    std::ifstream file(filename);

    if (!file.is_open()) {
        return values;
    }

    std::string line;

    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std:string value;

        while(std::getline(ss, value, ',')) {
            // convert to double
            values.push_back(std::stod(value));
        }
    }

    return values;
}

std::vector<int> findPeaks(vector<double> values) {
    std::vector<int> peaks;
    for (int i = 1; i < values.size() - 1; i++) {
        if (values[i] > values[i - 1] && values[i] > values[i + 1]) {
            peaks.push_back(i);
        }
    }
    return peaks;
}

