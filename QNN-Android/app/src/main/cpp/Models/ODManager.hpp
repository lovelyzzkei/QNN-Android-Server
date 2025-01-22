//
// Created by user on 2025-01-09.
//

#ifndef ARIA_ODMANAGER_HPP
#define ARIA_ODMANAGER_HPP

#include <vector>
#include "opencv2/opencv.hpp"
#include "../TFLiteManager.hpp"

#include "android/log.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/interpreter.h"
#include "../QnnManager.hpp"

#define LOG_TAG "CYJUNG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

struct Detection {
    float x1;
    float y1;
    float x2;
    float y2;
    std::string cls;
    float score;
};

// Define the mean and standard deviation for normalization
const std::vector<float> NORM_MEAN = {0.485f, 0.456f, 0.406f};
const std::vector<float> NORM_STD = {0.229f, 0.224f, 0.225f};


class ODManager{
public:
    ODManager();
    ODManager(const char* device,
              const char* model, const char* backend,
              const char* precision, const char* framework);
    ~ODManager();

    void preprocessImage(cv::Mat &mrgb);
    std::vector<Detection> doODInference(cv::Mat &mrgb); // Object Detection Inference
    std::vector<Detection> postprocessResults();
    std::vector<Detection> postprocessResultsQNN(
            std::vector<std::pair<std::vector<size_t>, float32_t*>> dets);


    void setQnnManager(QnnManager* qnnManager) {
        this->qnnManager = qnnManager;
    };

// PRIVATE FUNCTIONS
private:
    void loadClassNames();
    void prepareODModel(const char* device);
    std::vector<Detection> filterHighConfBoxes(
            const std::vector<std::vector<float>>& boxes,  // (1, N_OBJS, 4)
            const std::vector<float>& scores,             // (1, N_OBJS)
            const std::vector<float>& class_idx,            // (1, N_OBJS)
            float score_threshold, float iou_threshold);
    std::vector<Detection> nonMaximumSuppression(const std::vector<Detection>& detections, float iou_threshold);
    template <typename T> std::vector<T> tensorToVector(const TfLiteTensor* tensor);

// PRIVATE VARIABLES
private:
    int width;
    int height;
    int numObjects;
    std::vector<std::string> classNames;
    std::unique_ptr<tflite::Interpreter> mODModel;

    const char *framework;
    QnnManager* qnnManager = nullptr;
    TFLiteManager* tfLiteManager = nullptr;
};

#endif //ARIA_ODMANAGER_HPP
