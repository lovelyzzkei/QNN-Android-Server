//
// Created by user on 2025-02-07.
//

#ifndef APP_QNNINFERENCERUNNER_HPP
#define APP_QNNINFERENCERUNNER_HPP


#pragma once
#include <vector>
#include <utility>
#include <cstdlib>
#include <arm_neon.h>

#include "QnnLoader.hpp"
#include "QnnBackendManager.hpp"
#include "QnnContextManager.hpp"
#include "QnnTypeMacros.hpp"
#include "Utils/IOTensor.hpp"
#include "Utils/QnnSampleAppUtils.hpp"

class QnnInferenceRunner {
public:
    void setup(qnn_wrapper_api::GraphInfo_t** graphsInfo,
            size_t graphsCount,
            Qnn_ProfileHandle_t profileHandle);

    StatusCode execute(float32_t* inputBuffer, QnnLoader& loader,
                       QnnBackendManager& backendMgr, QnnContextManager& contextMgr, int vtcmSizeInMB, int offset);

    // Provide a getter for the final outputs
    const std::vector<std::pair<std::vector<size_t>, float32_t*>>& getOutputs() const {
        return m_inferData;
    }
    // IO tensor
    qnn::tools::iotensor::IOTensor m_ioTensor;

private:
    std::vector<std::pair<std::vector<size_t>, float32_t*>> retrieveOutputData(Qnn_Tensor_t* outputs, int numOutputTensors);
    StatusCode fillDims(std::vector<size_t>& dims,uint32_t* inDimensions, uint32_t rank);

    // references to the graph(s) we will execute
    qnn_wrapper_api::GraphInfo_t** m_graphsInfo = nullptr;
    size_t m_graphsCount = 0;
    Qnn_ProfileHandle_t m_profileHandle = nullptr;

    // final inference data
    std::vector<std::pair<std::vector<size_t>, float32_t*>> m_inferData;
};


#endif //APP_QNNINFERENCERUNNER_HPP
