//
// Created by user on 2025-02-06.
//

#ifndef APP_MODELFACTORY_HPP
#define APP_MODELFACTORY_HPP

#include <memory>
#include "TaskType.hpp"
#include "IModel.hpp"
#include "YoloModel.hpp"
#include "DAv2Model.hpp"
#include "ImageClassificationModel.hpp"
#include "SuperResolutionModel.hpp"

std::unique_ptr<IModel> createModel(TaskType task) {
    switch (task) {
        case TaskType::OBJECT_DETECTION:
            return std::make_unique<YoloModel>();
        case TaskType::DEPTH_ESTIMATION:
            return std::make_unique<DAv2Model>();
        case TaskType::IMAGE_CLASSIFICATION:
            return std::make_unique<ImageClassificationModel>();
        case TaskType::SUPER_RESOLUTION:
             return std::make_unique<SuperResolutionModel>();
        default:
            return nullptr;
    }
}


#endif //APP_MODELFACTORY_HPP
