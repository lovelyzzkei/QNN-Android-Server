//
// Created by user on 2025-02-05.
//

#include "DEManager.hpp"
#include "Log/Logger.hpp"
#include "QnnTypeMacros.hpp"
#include "QnnManager.hpp"

DEManager::DEManager() {

}

DEManager::DEManager(const char* device,
                     const char* model_, const char* backend_,
                     const char* precision, const char* framework) {

    std::string model = std::string(model_) + "_" + std::string(precision) + ".so";
    qnnManager = new QnnManager(model.c_str(), backend_);

    auto dim = getQnnTensorDimensions((*qnnManager->getGraphsInfo())[0].inputTensors);
    QNN_INFO("Dim[0, 1, 2, 3]: %d, %d, %d, %d", dim[0], dim[1], dim[2], dim[3]);
    width = dim[2];
    height = dim[1];
    this->framework = framework;
}


DEManager::~DEManager() {
}


void DEManager::preprocessImage(cv::Mat &mrgb) {
    const std::vector<float> NORM_MEAN = {0.485f, 0.456f, 0.406f};
    const std::vector<float> NORM_STD = {0.229f, 0.224f, 0.225f};

    cv::resize(mrgb, mrgb, cv::Size(width, height), cv::INTER_CUBIC);
    QNN_INFO("width: %d, height: %d", width, height);
    cv::cvtColor(mrgb, mrgb, CV_BGR2RGB);
    mrgb.convertTo(mrgb, CV_32FC3, 1.0/255.0);

    // Normalize the image -> mean and std are fit to NYU dataset. Need to change to use camera
    cv::Scalar mean(NORM_MEAN[0], NORM_MEAN[1], NORM_MEAN[2]);
    cv::Scalar std(NORM_STD[0], NORM_STD[1], NORM_STD[2]);

    cv::subtract(mrgb, mean, mrgb);
    cv::divide(mrgb, std, mrgb);
}


std::vector<float> DEManager::doDEInference(cv::Mat &mrgb) {
    std::vector<float> depthMap;
    // Preprocess
    logExecutionTime("Preprocess", [&]() {
        preprocessImage(mrgb);
    });

    // QNN version of executing model
    logExecutionTime("Inference", [&]() {
        qnnManager->inferenceModel(reinterpret_cast<float32_t *>(mrgb.data));
    });

    // Postprocessing
    logExecutionTime("Postprocess", [&]() {
        depthMap = postprocess(qnnManager->m_inferData);
    });
    return depthMap;
}


std::vector<float> DEManager::postprocess(std::vector<std::pair<std::vector<size_t>, float32_t*>> results) {
    auto dims = results[0].first;
    auto fPtr = results[0].second;

    // 1) Calculate the total number of elements from dims
    size_t numElements = 1;
    for (auto d : dims)
    {
        numElements *= d;
    }

    // 2) Construct a std::vector<float> by copying from the float32_t* buffer
    std::vector<float> ret(fPtr, fPtr + numElements);
    return ret;
}


