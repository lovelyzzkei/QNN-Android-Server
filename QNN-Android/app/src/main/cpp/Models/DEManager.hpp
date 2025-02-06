//
// Created by user on 2025-02-05.
//

#ifndef APP_DEMANAGER_HPP
#define APP_DEMANAGER_HPP

#include <vector>
#include "opencv2/opencv.hpp"
#include "QnnManager.hpp"


// Define the mean and standard deviation for normalization
//const std::vector<float> NORM_MEAN = {0.485f, 0.456f, 0.406f};
//const std::vector<float> NORM_STD = {0.229f, 0.224f, 0.225f};


class DEManager {
public:
    DEManager();
    DEManager(const char* device,
              const char* model, const char* backend,
              const char* precision, const char* framework);
    ~DEManager();

    void preprocessImage(cv::Mat &mrgb);
    std::vector<float> doDEInference(cv::Mat &mrgb); // Object Detection Inference
    std::vector<float> postprocess(std::vector<std::pair<std::vector<size_t>, float32_t*>> results);

    void setQnnManager(QnnManager* qnnManager) {
        this->qnnManager = qnnManager;
    };

// PRIVATE FUNCTIONS
private:


// PRIVATE VARIABLES
private:
    int width;
    int height;

    const char *framework;
    QnnManager* qnnManager = nullptr;
};


#endif //APP_DEMANAGER_HPP
