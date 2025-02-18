//
// Created by user on 2025-02-18.
//

#ifndef APP_SUPERRESOLUTIONMODEL_HPP
#define APP_SUPERRESOLUTIONMODEL_HPP

#pragma once
#include <vector>
#include <string>
#include "QnnManager.hpp"
#include "IModel.hpp"


class SuperResolutionModel : public IModel {
public:
    SuperResolutionModel() = default;
    virtual ~SuperResolutionModel() = default;
    void* postprocess() override;

};



#endif //APP_SUPERRESOLUTIONMODEL_HPP
