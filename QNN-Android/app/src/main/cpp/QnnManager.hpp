//
// Created by user on 2025-01-10.
//
// Loading QNN Model by QNN API, not using TFLite

#ifndef QNNSKELETON_QNNMANAGER_HPP
#define QNNSKELETON_QNNMANAGER_HPP

#include "QnnInterface.h"
#include "QnnWrapperUtils.hpp"
#include "Utils/IOTensor.hpp"
#include "Utils/QnnSampleAppUtils.hpp"
#include "QNN/System/QnnSystemInterface.h"

#include <opencv2/core/types_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

// Graph Related Function Handle Types
typedef qnn_wrapper_api::ModelError_t (*ComposeGraphsFnHandleType_t)(
        Qnn_BackendHandle_t,
        QNN_INTERFACE_VER_TYPE,
        Qnn_ContextHandle_t,
        const qnn_wrapper_api::GraphConfigInfo_t **,
        const uint32_t,
        qnn_wrapper_api::GraphInfo_t ***,
        uint32_t *,
        bool
//        QnnLog_Callback_t,
//        QnnLog_Level_t
        );
typedef qnn_wrapper_api::ModelError_t (*FreeGraphInfoFnHandleType_t)(
        qnn_wrapper_api::GraphInfo_t ***, uint32_t);

typedef struct QnnFunctionPointers {
    ComposeGraphsFnHandleType_t composeGraphsFnHandle;
    FreeGraphInfoFnHandleType_t freeGraphInfoFnHandle;
    QNN_INTERFACE_VER_TYPE qnnInterface;
    QNN_SYSTEM_INTERFACE_VER_TYPE qnnSystemInterface;
} QnnFunctionPointers;

using namespace qnn::tools::sample_app;


enum class StatusCode {
    SUCCESS,
    FAILURE,
    FAILURE_INPUT_LIST_EXHAUSTED,
    FAILURE_SYSTEM_ERROR,
    FAILURE_SYSTEM_COMMUNICATION_ERROR,
    QNN_FEATURE_UNSUPPORTED
};



class QnnManager {
public:
    QnnManager(const char* model, const char* backend);
    ~QnnManager();
    std::string getBackendBuildId();

    StatusCode prepareQnnManager(std::string modelPath, std::string backEndPath);
    StatusCode prepareModel();
    StatusCode inferenceModel(float32_t* input_buffer);
    std::vector<std::pair<std::vector<size_t>, float32_t*>> retrieveOutputData(Qnn_Tensor_t* outputs, int numOutputTensors);
    qnn_wrapper_api::GraphInfo_t **getGraphsInfo() { return m_graphsInfo; }

    qnn_wrapper_api::GraphConfigInfo_t **m_graphConfigsInfo = nullptr;
    std::vector<std::pair<std::vector<size_t>, float32_t*>> m_inferData;

private:
    static qnn_wrapper_api::ModelError_t QnnModel_freeGraphsInfo(qnn_wrapper_api::GraphInfoPtr_t **graphsInfo, uint32_t numGraphsInfo) {
        return qnn_wrapper_api::freeGraphsInfo(graphsInfo, numGraphsInfo);
    }

    StatusCode initialize();
    StatusCode initializeBackend();
    StatusCode initializeProfiling();
    StatusCode isDevicePropertySupported();
    StatusCode createDevice();
    StatusCode verifyFailReturnStatus(Qnn_ErrorHandle_t errCode);

    StatusCode createContext();
    StatusCode composeGraphs();
    StatusCode finalizeGraphs();
    StatusCode createFromBinary();

    StatusCode fillDims(std::vector<size_t>& dims,uint32_t* inDimensions, uint32_t rank);
    StatusCode copyFromFloatToNative(float32_t* floatBuffer, Qnn_Tensor_t* tensor);


    StatusCode extractBackendProfilingInfo (Qnn_ProfileHandle_t profileHandle);
    StatusCode extractProfilingEvent(QnnProfile_EventId_t profileEventId);
    StatusCode extractProfilingSubEvents(QnnProfile_EventId_t profileEventId);

    StatusCode createPowerConfigId();
    StatusCode setPowerConfig();
    StatusCode setRpcLatencyAndPolling();

    void reportError(const std::string &err);

    bool m_isBackendInitialized;
    bool m_isContextCreated = false;
    bool m_loadFromCachedBinary = false;

    QnnFunctionPointers m_qnnFunctionPointers;
    void* m_backendHandle = nullptr;
    void* m_modelHandle = nullptr;
    Qnn_LogHandle_t m_logHandle = nullptr;
    Qnn_DeviceHandle_t m_deviceHandle = nullptr;
    Qnn_ProfileHandle_t m_profileBackendHandle = nullptr;

    QnnBackend_Config_t **m_backendConfig = nullptr;
    Qnn_ContextHandle_t m_context = nullptr;
    QnnContext_Config_t **m_contextConfig = nullptr;

    qnn_wrapper_api::GraphInfo_t **m_graphsInfo = nullptr;


    uint32_t m_graphsCount;
    uint32_t m_graphConfigsInfoCount;

    // Power performance
    uint32_t m_powerConfigId;
    uint32_t m_deviceId = 0;
    uint32_t m_coreId = 0;

    bool m_debug = true;

    // IO tensor
    qnn::tools::iotensor::IOTensor m_ioTensor;
    qnn::tools::iotensor::OutputDataType m_outputDataType;
    qnn::tools::iotensor::InputDataType m_inputDataType;

    ProfilingLevel m_profilingLevel = ProfilingLevel::BASIC;

};

#endif //QNNSKELETON_QNNMANAGER_HPP
