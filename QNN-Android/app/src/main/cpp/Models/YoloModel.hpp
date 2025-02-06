//
// Created by user on 2025-02-06.
//

#ifndef APP_YOLOMODEL_HPP
#define APP_YOLOMODEL_HPP

#pragma once
#include <vector>
#include <string>
#include "CommonModelBase.hpp"
#include "QnnManager.hpp"

struct Detection {
    float x1;
    float y1;
    float x2;
    float y2;
    std::string cls;
    float score;
};

// A container for all detections from one inference
struct DetectionResult {
    std::vector<Detection> detections;
};


class YoloModel : public CommonModelBase {
public:
    YoloModel();
    virtual ~YoloModel() {}

    void* postprocess() override;

private:
    void loadClassNames();
    std::vector<Detection> filterHighConfBoxes(
            const std::vector<std::vector<float>>& boxes,  // (1, N_OBJS, 4)
            const std::vector<float>& scores,             // (1, N_OBJS)
            const std::vector<float>& class_idx,            // (1, N_OBJS)
            float score_threshold, float iou_threshold);
    std::vector<Detection> nonMaximumSuppression(const std::vector<Detection>& detections, float iou_threshold);

private:
    std::vector<std::string> classNames;
};


#endif //APP_YOLOMODEL_HPP
