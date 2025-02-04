//
// Created by user on 2025-02-03.
//

#ifndef QNN_ANDROID_ODMODEL_HPP
#define QNN_ANDROID_ODMODEL_HPP

#include "ModelBase.hpp"
#include "../QnnManager.hpp"

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


class ODModel : public ModelBase {
public:
    ODModel(const char* modelPath,
            const char* backend,
            const char* precision);
    void initializeModel() override;
    void preprocess(cv::Mat& input) override;
    std::vector<float> infer(cv::Mat& input) override;
    std::vector<float> postprocess(const std::vector<float>& output) override;

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
    std::vector<Detection> doODInference(cv::Mat &mrgb);    // E2E inference

// PRIVATE VARIABLES
private:
    int width;
    int height;
    int numObjects;
    std::vector<std::string> classNames;

    const char *framework;
    QnnManager* qnnManager = nullptr;
};

#endif //QNN_ANDROID_ODMODEL_HPP
