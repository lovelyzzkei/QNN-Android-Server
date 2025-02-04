//
// Created by user on 2025-02-03.
//

#ifndef QNN_ANDROID_MODELBASE_HPP
#define QNN_ANDROID_MODELBASE_HPP

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

class ModelBase {
protected:
    const char* modelPath;
    const char* backend;
    int width, height;

public:
    ModelBase(const char* modelPath, const char* backend);
    virtual ~ModelBase() = default;

    // Common Model Methods
    virtual void initializeModel() = 0;
    virtual void preprocess(cv::Mat& input) = 0;
    virtual std::vector<float> infer(cv::Mat& input) = 0;
    virtual std::vector<float> postprocess(const std::vector<float>& output) = 0;
};



#endif //QNN_ANDROID_MODELBASE_HPP
