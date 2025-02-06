//
// Created by user on 2025-02-06.
//

#include "DAv2Model.hpp"

void* DAv2Model::postprocess() {
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
    return static_cast<void*>(ret);
}
