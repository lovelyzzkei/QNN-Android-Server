//
// Created by user on 2025-02-03.
//

#include "ModelBase.hpp"

ModelBase::ModelBase(const char* modelPath, const char* backend)
        : modelPath(modelPath), backend(backend), width(0), height(0) {}