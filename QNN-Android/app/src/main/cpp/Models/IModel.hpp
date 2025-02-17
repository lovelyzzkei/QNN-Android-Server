//
// Created by user on 2025-02-06.
//

#ifndef APP_IMODEL_HPP
#define APP_IMODEL_HPP


#pragma once
#include <string>
#include <opencv2/opencv.hpp>
#include "Log/Logger.hpp"
#include "QnnManager.hpp"
#include "QnnTypeMacros.hpp"

/**
 * A generic interface for any model that follows:
 *  - initialize(...)
 *  - preprocess(...)
 *  - inference()
 *  - postprocess()
 */
class IModel {
public:
    virtual ~IModel() = default;

    // Provide a default init that loads QNN context,
    // or sets up your backends, etc.
    bool initialize(const std::string& device,
                    const std::string& modelName,
                    const std::string& backend,
                    const std::string& precision,
                    const std::string& framework,
                    const int vtcmSizeInMB,
                    const int offset)
    {
        std::string model = std::string(modelName) + "_" + std::string(precision) + ".so";
        LOGD("Initializing with modelFile: %s", model.c_str());
        qnnManager = new QnnManager(model.c_str(), backend.c_str(), vtcmSizeInMB, offset);

        // ASSUME input tensor is NHWC!!
        auto dim = getQnnTensorDimensions((*qnnManager->getContextMgr()->getGraphsInfo())[0].inputTensors);
        width = dim[2];
        height = dim[1];
        LOGD("width: %d, height: %d", width, height);

        return true;
    }

    virtual void setPowerMode(int powerMode) {
        qnnManager->setPowerMode(powerMode);
    }

    // Provide a default preprocess if most models do the same steps
    // (YUV->RGB, resize, etc.). Derived classes can override if needed.
    virtual bool preprocess(const uint8_t* pYUVFrameData, int iWidth, int iHeight) {
        // Convert to OpenCV Mat, YUV -> RGB (Originally YUV_420_888)
        cv::Mat myuv(iHeight + iHeight / 2, iWidth, CV_8UC1, (unsigned char*)pYUVFrameData);
        cv::Mat mrgb(iHeight, iWidth, CV_8UC3);
        cv::cvtColor(myuv, mrgb, cv::COLOR_YUV2RGB_NV21, 3);

        // Resize and convert BGR to RGB
        cv::resize(mrgb, mrgb, cv::Size(width, height), cv::INTER_CUBIC);
        cv::cvtColor(mrgb, mrgb, CV_BGR2RGB);
        mrgb.convertTo(mrgb, CV_32FC3, 1.0/255.0);
        LOGD("mrgb size: %d x %d", mrgb.cols, mrgb.rows);

        // Normalize the image -> mean and std are fit to NYU dataset. Need to change to use camera
        cv::Scalar mean(NORM_MEAN[0], NORM_MEAN[1], NORM_MEAN[2]);
        cv::Scalar std(NORM_STD[0], NORM_STD[1], NORM_STD[2]);

        cv::subtract(mrgb, mean, mrgb);
        cv::divide(mrgb, std, mrgb);

        this->preprocessedInput = mrgb;
        return true;
    }

    // Provide a default inference that calls QNN
    virtual bool inference() {
        qnnManager->runInference(
                reinterpret_cast<float32_t *>(this->preprocessedInput.data));
        return true;
    }


    /**
     * Postprocess the outputs. Typically returns pointers
     * or data structures with final results (bounding boxes, depth map, etc.).
     *
     * Because the results can vary by model, we use a void* here
     * or a generic pointer. You could also define a union or a polymorphic
     * return type if desired.
     *
     * @return pointer to postprocessed results (must be cast or interpreted).
     */
    virtual void* postprocess() = 0;

    int getWidth() const { return width; }

protected:
    int width;
    int height;

    // Variables related to inference
    cv::Mat preprocessedInput;
    QnnManager* qnnManager = nullptr;

    std::vector<float> NORM_MEAN = {0.485f, 0.456f, 0.406f};
    std::vector<float> NORM_STD = {0.229f, 0.224f, 0.225f};

};


#endif //APP_IMODEL_HPP
