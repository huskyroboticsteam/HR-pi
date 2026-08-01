#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cmath>

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "Usage: ./fluorescent /dev/video20\n";
        return -1;
    }

    cv::VideoCapture cap(argv[1], cv::CAP_V4L2);

    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y','U','Y','V'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(cv::CAP_PROP_FPS, 5);
    cap.set(cv::CAP_PROP_CONVERT_RGB, 0);

    if (!cap.isOpened()) {
        std::cerr << "Camera not opened\n";
        return -1;
    }

    cv::Mat frame;

    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    // making CSV
    std::ofstream file("fluorescent_avg.csv");
    if (!file.is_open()) {
        std::cerr << "Failed to open CSV file\n";
        return -1;
    }

    file << "intensity\n";

    // === SPIKE DETECTION VARIABLES ===
    double baseline = 0;
    bool baseline_initialized = false;

    double alpha = 0.03;      // smoothing factor
    double threshold = 10.0;  // spike sensitivity
    int cooldown = 0;         // prevents repeated triggers

    while (true) {

        cap >> frame;
        if (frame.empty()) break;

        long long sumY = 0;
        long long count = 0;

        // reads in YUV format
        for (int y = 0; y < height; y++) {
            uchar* row = frame.ptr<uchar>(y);

            for (int x = 0; x < width * 2; x += 4) {
                // YUYV format: Y0 U Y1 V
                uchar Y0 = row[x];
                uchar Y1 = row[x + 2];

                sumY += Y0 + Y1;
                count += 2;
            }
        }

        double avgIntensity = (double)sumY / count;

        // write to csv file
        file << avgIntensity << "\n";

        // detects if there are sudden spikes to the light values being read
        if (!baseline_initialized) {
            baseline = avgIntensity;
            baseline_initialized = true;
        }

        double diff = avgIntensity - baseline;

        if (diff > threshold && cooldown == 0) {
            std::cout << "SPIKE DETECTED! Intensity: "
                      << avgIntensity
                      << " | Baseline: " << baseline
                      << " | Diff: " << diff << std::endl;

            cooldown = 10; // ignore next few frames
        }

        // updates the baseline
        baseline = alpha * avgIntensity + (1 - alpha) * baseline;

        if (cooldown > 0) cooldown--;

        std::cout << "Avg: " << avgIntensity << std::endl;

        if (cv::waitKey(1) == 'q')
            break;
    }

    file.close();
    return 0;
}