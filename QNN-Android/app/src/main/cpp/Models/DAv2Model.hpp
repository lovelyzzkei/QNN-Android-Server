//
// Created by user on 2025-02-06.
//

#ifndef APP_DAV2MODEL_HPP
#define APP_DAV2MODEL_HPP


#pragma once
#include <vector>
#include <string>
#include "QnnManager.hpp"
#include "IModel.hpp"


class DAv2Model : public IModel {
public:
    DAv2Model() = default;
    virtual ~DAv2Model() = default;
    void* postprocess() override;

};


#endif //APP_DAV2MODEL_HPP
