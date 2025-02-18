//
// Created by user on 2025-02-18.
//

#include "SuperResolutionModel.hpp"

void* SuperResolutionModel::postprocess() {
    auto results = qnnManager->m_inferData;

    auto dims = results[0].first;
    auto fPtr = results[0].second;

    // 1) Calculate the total number of elements from dims
    size_t numElements = 1;
    for (auto d : dims)
    {
        numElements *= d;
    }

    // 2) Construct a std::vector<float> by copying from the float32_t* buffer
//    std::vector<float> ret(fPtr, fPtr + numElements);
    std::vector<float>* ret = new std::vector<float>(fPtr, fPtr + numElements);

    std::vector<uint8_t>* dst = new std::vector<uint8_t>(numElements);
    for (int i = 0; i < numElements; i++) {
        float val = std::clamp(ret->at(i), 0.0f, 1.0f);
        dst->at(i) = (uint8_t)(val*255.0f);
    }


    return static_cast<void*>(dst);
}
