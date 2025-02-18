//
// Created by user on 2025-02-17.
//

#ifndef APP_IMAGECLASSIFICATIONMODEL_HPP
#define APP_IMAGECLASSIFICATIONMODEL_HPP

#pragma once
#include <vector>
#include <string>
#include "QnnManager.hpp"
#include "IModel.hpp"


class ImageClassificationModel : public IModel {
public:
    ImageClassificationModel();
    virtual ~ImageClassificationModel() = default;
    void* postprocess() override;
    void loadClassNames();
private:
    std::vector<std::string> classNames;
};



#endif //APP_IMAGECLASSIFICATIONMODEL_HPP
