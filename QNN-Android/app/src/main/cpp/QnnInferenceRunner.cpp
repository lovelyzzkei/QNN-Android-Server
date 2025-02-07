//
// Created by user on 2025-02-07.
//

#include "Log/Logger.hpp"
#include "Utils/DataUtil.hpp"
#include "QnnInferenceRunner.hpp"

using namespace qnn::tools;

void QnnInferenceRunner::setup(qnn_wrapper_api::GraphInfo_t** graphsInfo,
                                size_t graphsCount,
                                Qnn_ProfileHandle_t profileHandle)
{
    m_graphsInfo = graphsInfo;
    m_graphsCount = graphsCount;
    m_profileHandle = profileHandle;
}


StatusCode QnnInferenceRunner::execute(float32_t* inputBuffer, QnnLoader& loader, QnnBackendManager& backendMgr) {
    auto returnStatus = StatusCode::SUCCESS;


    for (size_t graphIdx = 0; graphIdx < m_graphsCount; graphIdx++) {
        Qnn_Tensor_t* inputs = nullptr;
        Qnn_Tensor_t* outputs = nullptr;

        // Setting input,output tensors
        QNN_INFO("Setting up io tensors");
        if (iotensor::StatusCode::SUCCESS !=
            m_ioTensor.setupInputAndOutputTensors(&inputs, &outputs, (*m_graphsInfo)[graphIdx])) {
            returnStatus = StatusCode::FAILURE;
            break;
        }
        if (nullptr == inputs) {
            QNN_ERROR("inputs is nullptr");
            return StatusCode::FAILURE;
        }

        auto graphInfo = (*m_graphsInfo)[graphIdx];
        auto inputCount = graphInfo.numInputTensors;

        // Set input tensor data
        // TODO: Modify to get multiple inputs
        QNN_INFO("Setting %d input tensor data", inputCount);
        for (size_t inputIdx = 0; inputIdx < inputCount; inputIdx++) {

            // DEBUG
//            for (int i = 0; i < 10; i++) {
//                QNN_DEBUG("before buffer[%d] = %f", i, input_buffer[i]);
//            }
            memcpy(QNN_TENSOR_GET_CLIENT_BUF(inputs).data, inputBuffer,
                   QNN_TENSOR_GET_CLIENT_BUF(inputs).dataSize);
            QNN_INFO("Input tensor data set");

//                // Quantize input tensor
//                // returnStatus = copyFromFloatToNative(input_buffer, inputs); // Copy uint8_t* image to input
//            }
        }

        // Execute graph
        auto start = std::chrono::high_resolution_clock::now();
        Qnn_ErrorHandle_t executeStatus = QNN_GRAPH_NO_ERROR;
        executeStatus = loader.getFunctionPointers().qnnInterface.graphExecute(
                graphInfo.graph,
                inputs,
                graphInfo.numInputTensors,
                outputs,
                graphInfo.numOutputTensors,
                backendMgr.getProfileHandle(),
                nullptr);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start);
        QNN_INFO("graphExecute time: %.3f ms", duration.count());

        if (QNN_GRAPH_NO_ERROR != executeStatus) {
            QNN_ERROR("Execute Status: %d", executeStatus);
            return StatusCode::FAILURE;
        }
        backendMgr.extractBackendProfilingInfo(loader);

        auto outputsVector = retrieveOutputData(outputs, graphInfo.numOutputTensors);
        m_inferData = outputsVector;
    }
    QNN_INFO("Inference done");
    return returnStatus;
}


// Retreive output from Qnn_Tensor_t outputs
// Return format: vector<pair<vector<size_t> Dimensions, float32_t* DataBuffer>>
std::vector<std::pair<std::vector<size_t>, float32_t*>> QnnInferenceRunner::retrieveOutputData(Qnn_Tensor_t* outputs, int numOutputTensors) {
    std::vector<std::pair<std::vector<size_t>, float32_t*>> outputsVector;

    QNN_INFO("Retrieving output tensor data");
    for (size_t outputIdx = 0; outputIdx < numOutputTensors; outputIdx++) {
        Qnn_Tensor_t* output = &outputs[outputIdx];
        std::vector<size_t> outputDims;

        if (StatusCode::SUCCESS != fillDims(outputDims, QNN_TENSOR_GET_DIMENSIONS(output), QNN_TENSOR_GET_RANK(output))) {
            QNN_ERROR("Failed to fill output dims");
            continue; // Skip this tensor on failure
        }

//        for (size_t dimi = 0; dimi < outputDims.size(); dimi++) {
//            QNN_INFO("output_dims[%zu][%zu] = %zu", outputIdx, dimi, outputDims[dimi]);
//        }

        size_t elementCount = datautil::calculateElementCount(outputDims);
        float32_t* floatBuffer = static_cast<float32_t*>(malloc(elementCount * sizeof(float32_t)));

        if (nullptr == floatBuffer) {
            QNN_ERROR("Memory allocation failed for floatBuffer");
            continue; // Skip this tensor on failure
        }

        float* in = reinterpret_cast<float*>(QNN_TENSOR_GET_CLIENT_BUF(output).data);
        if (nullptr == in) {
            QNN_ERROR("Received a nullptr in input buffer");
            free(floatBuffer);
            continue; // Skip this tensor on failure
        }

        memcpy(floatBuffer, in, elementCount * sizeof(float32_t));
        outputsVector.emplace_back(outputDims, floatBuffer);
    }
    return outputsVector;
}

StatusCode QnnInferenceRunner::fillDims(std::vector<size_t>& dims,uint32_t* inDimensions, uint32_t rank) {
    if (nullptr == inDimensions) {
        QNN_ERROR("input dimensions is nullptr");
        __android_log_print(ANDROID_LOG_ERROR, "QNN ", "******************************************************************** input dimensions is nullptr\n");
        return StatusCode::FAILURE;
    }
    for (size_t r = 0; r < rank; r++) {
        dims.push_back(inDimensions[r]);
    }
    return StatusCode::SUCCESS;
}



