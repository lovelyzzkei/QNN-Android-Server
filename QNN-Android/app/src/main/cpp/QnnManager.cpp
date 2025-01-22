//
// Created by user on 2025-01-10.
// Mainly reference from UbiquitousLearning/mllm

#include <string>
#include <vector>
#include <utility>
#include <cstdlib>

#include "QNN/HTP/QnnHtpDevice.h"
#include "Log/Logger.hpp"
#include "PAL/DynamicLoading.hpp"

#include "Utils/DataUtil.hpp"
#include "Utils/DynamicLoadUtil.hpp"
#include "Utils/QnnSampleAppUtils.hpp"

#include "QnnManager.hpp"
#include "QnnTypeMacros.hpp"
#include "HTP/QnnHtpGraph.h"

#include <opencv2/core/types_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

using namespace qnn;
using namespace qnn::tools;
using namespace qnn::tools::sample_app;


QnnManager::QnnManager(const char* model, const char* backend) {
    // TODO : Get both from user
//    std::string modelPath = "libQnn_yolov6_fp16.so";
    std::string modelPath = std::string(model);
    std::string backEndPath;

    if (strcmp(backend, "NPU") == 0) {
        backEndPath = "libQnnHtp.so";
    } else if (strcmp(backend, "CPU") == 0) {
        backEndPath = "libQnnCpu.so";
    }
//    std::string backEndPath = "libQnnHtpV69Stub.so";

//    std::string opPackagePaths = "libQnnCpuOpPackageExample.so:QnnOpPackage_interfaceProvider;";

    QNN_INFO("Model: %s", modelPath.c_str());
    QNN_INFO("Backend: %s", backEndPath.c_str());

    // Prepare QNN resources
    this->prepareQnnManager(modelPath, backEndPath);
    this->prepareModel();

    QNN_INFO("All preparation for model done");
}


QnnManager::~QnnManager(){

}


// Initialize QNN with the user's model and desired backend
StatusCode QnnManager::prepareQnnManager(std::string modelPath, std::string backEndPath) {
    // Load backend and model .so and validate all the required function symbols are resolved
    // QNN Interface mechanism can be used to set up a table of function pointers to QNN APIs
    // in the backend instead of manually resolving symbols to each and every API, which makes
    // resolving symbols easy.
    auto statusCode = dynamicloadutil::getQnnFunctionPointers(
            backEndPath,
            modelPath,
            &m_qnnFunctionPointers,
            &m_backendHandle,
            true,        // True
            &m_modelHandle);

    m_outputDataType   = iotensor::OutputDataType::FLOAT_AND_NATIVE;
    m_inputDataType    = iotensor::InputDataType::FLOAT;


    // cause we build graph in runtime, the freeGraphInfoFnHandle should be assigned here
    if (dynamicloadutil::StatusCode::SUCCESS != statusCode) {
        if (dynamicloadutil::StatusCode::FAIL_LOAD_BACKEND == statusCode) {
            exitWithMessage(
                    "Error initializing QNN Function Pointers: could not load backend: " + backEndPath,
                    EXIT_FAILURE);
        } else if (dynamicloadutil::StatusCode::FAIL_LOAD_MODEL == statusCode) {
            exitWithMessage(
                    "Error initializing QNN Function Pointers: could not load model: ",
                    EXIT_FAILURE);
        } else {
            exitWithMessage("Error initializing QNN Function Pointers", EXIT_FAILURE);
        }
    }
    QNN_INFO("QNN Function Pointers initialized");

//    m_qnnFunctionPointers.freeGraphInfoFnHandle = QnnManager::QnnModel_freeGraphsInfo;

    // init qnn resources
    QNN_INFO("Backend build version: %s", getBackendBuildId().c_str());

    // Initialize logging in the backend -> Currently not initialize
    if (StatusCode::SUCCESS != this->initialize()) {
        QNN_ERROR("Initialization failure");
    }
    QNN_INFO("Initialization success");

    // Initialize QnnBackend
    // Once logging has been successfully initialized, backend can be initialized.
    if (StatusCode::SUCCESS != this->initializeBackend()) {
        QNN_ERROR("Backend Initialization failure");
    }
    QNN_INFO("Backend Initialized");

    auto devicePropertySupportStatus = this->isDevicePropertySupported();
    if (StatusCode::FAILURE != devicePropertySupportStatus) {
        auto createDeviceStatus = this->createDevice();
        if (StatusCode::SUCCESS != createDeviceStatus) {
            QNN_ERROR("Device Creation failure");
        }
    }
    QNN_INFO("Device Created");

    // If profiling is desired, after the backend is initialized, a profile handle can be set up.
    // This profile handle can be used at a later point in any API that supports profiling.
    if (StatusCode::SUCCESS != this->initializeProfiling()) {
        QNN_ERROR("Profiling Initialization failure");
    }
    QNN_INFO("Profiling Initialized");

    // Op packages are way to supply libraries containing ops to backends.
    // They can be registered as shown below
//        if (StatusCode::SUCCESS != this->registerOpPackages()) {
//            QNN_ERROR("Register Op Packages failure");
//        }
    return StatusCode::SUCCESS;
}



StatusCode QnnManager::prepareModel() {
    if (!m_loadFromCachedBinary) {
        if (StatusCode::SUCCESS != this->createContext()) {
            QNN_ERROR("Context Creation failure");
            return StatusCode::FAILURE;
        }
        QNN_INFO("Context Created");

        /*
         * Prepare graphs
            qnn-sample-app relies on the output from one of the converters to create a QNN network
            in the backend. composeGraphsFnHandle is mapped to QnnModel_composeGraphs API in the
            model shared library, which takes qnn_wrapper_api::GraphInfo_t*** as one of the parameters.
            The function composeGraphsFnHandle will make necessary calls to the backend to create
            a network(s). It also writes all necessary information, like information about input
            and output tensors related to the graph, required to execute a graph into the structure
            graphsInfo as shown in the following code block:
         */
        if (StatusCode::SUCCESS != this->composeGraphs()) {
            QNN_ERROR("Graph Prepare failure");
            return StatusCode::FAILURE;
        }
        QNN_INFO("Graph Prepared");

        if (StatusCode::SUCCESS != this->finalizeGraphs()) {
            QNN_ERROR("Graph Finalize failure");
            return StatusCode::FAILURE;
        }
        QNN_INFO("Graph Finalized");

    } else {
        if (StatusCode::SUCCESS != this->createFromBinary()) {
            QNN_ERROR("Create From Binary failure");
            return StatusCode::FAILURE;
        }
    }
    return StatusCode::SUCCESS;
}


// executeGraphs() that is currently used by qnn-sample-app's main.cpp.
// This function runs all the graphs present in model.so by reading
// inputs from input_list based files and writes output to .raw files.
StatusCode QnnManager::inferenceModel(float32_t* input_buffer) {
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
            memcpy(QNN_TENSOR_GET_CLIENT_BUF(inputs).data, input_buffer,
                   QNN_TENSOR_GET_CLIENT_BUF(inputs).dataSize);
            QNN_INFO("Input tensor data set");

            // I think this part is special case for image input
//            if (m_inputDataType == iotensor::InputDataType::FLOAT &&
//                QNN_TENSOR_GET_DATA_TYPE(inputs) == QNN_DATATYPE_FLOAT_32) {
//                for (int i = 0; i < 10; i++) {
//                    QNN_INFO("before buffer[%d] = %f", i, input_buffer[i]);
//                }
//                QNN_INFO("##############################################");
//                memcpy(QNN_TENSOR_GET_CLIENT_BUF(inputs).data, input_buffer,
//                       QNN_TENSOR_GET_CLIENT_BUF(inputs).dataSize);
//                QNN_INFO("Input tensor data set");
//
//                // 얘는 input을 양자화해서 inputBuffer에 넣어주는 작업
//                // returnStatus = copyFromFloatToNative(input_buffer, inputs); // Copy uint8_t* image to input
//            }
        }

        // Execute graph
        auto start = std::chrono::high_resolution_clock::now();
        Qnn_ErrorHandle_t executeStatus = QNN_GRAPH_NO_ERROR;
        executeStatus = m_qnnFunctionPointers.qnnInterface.graphExecute(
                graphInfo.graph,
                inputs,
                graphInfo.numInputTensors,
                outputs,
                graphInfo.numOutputTensors,
                m_profileBackendHandle,
                nullptr);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start);
        QNN_INFO("graphExecute time: %.3f ms", duration.count());

        if (QNN_GRAPH_NO_ERROR != executeStatus) {
            QNN_ERROR("Execute Status: %d", executeStatus);
            return StatusCode::FAILURE;
        }


        // Retrieve output tensor data
        auto outputsVector = retrieveOutputData(outputs, graphInfo.numOutputTensors);
        m_inferData = outputsVector;

        returnStatus = StatusCode::SUCCESS;
    }

    QNN_INFO("Inference done");
    return returnStatus;
}


// Retreive output from Qnn_Tensor_t outputs
// Return format: vector<pair<vector<size_t> Dimensions, float32_t* DataBuffer>>
std::vector<std::pair<std::vector<size_t>, float32_t*>> QnnManager::retrieveOutputData(Qnn_Tensor_t* outputs, int numOutputTensors) {
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



StatusCode QnnManager::initialize() {
//    if (!log::initializeLogging()) {
//        QNN_ERROR("ERROR: Unable to initialize logging!");
//        return StatusCode::FAILURE;
//    }
    // Logging can be set up before a backed is initialized and after a backend shared library has been dynamically loaded.
    if (log::isLogInitialized()) {
        auto logCallback = log::getLogCallback();
        auto logLevel = log::getLogLevel();
        QNN_INFO("Initializing logging in the backend. Callback: [%p], Log Level: [%d]",
                 logCallback,
                 logLevel);
        if (QNN_SUCCESS != m_qnnFunctionPointers.qnnInterface.logCreate(logCallback, logLevel, &m_logHandle)) {
            QNN_ERROR("Unable to initialize logging in the backend.");
            return StatusCode::FAILURE;
        }
    } else {
        QNN_ERROR("Logging not available in the backend.");
        return StatusCode::FAILURE;
    }
    QNN_INFO("Logging initialized");
    return StatusCode::SUCCESS;
}


StatusCode QnnManager::initializeBackend() {
    auto qnnStatus = m_qnnFunctionPointers.qnnInterface.backendCreate(
            m_logHandle, (const QnnBackend_Config_t **)m_backendConfig, &m_backendHandle);
    if (QNN_BACKEND_NO_ERROR != qnnStatus) {
        QNN_ERROR("Could not initialize backend due to error = {}", (unsigned int)qnnStatus);
        return StatusCode::FAILURE;
    }
    QNN_INFO("Initialize Backend Returned Status = %d", (unsigned int)qnnStatus);
    m_isBackendInitialized = true;
    return StatusCode::SUCCESS;
}


StatusCode QnnManager::initializeProfiling() {
    if (ProfilingLevel::OFF != m_profilingLevel) {
        QNN_INFO("Profiling turned on; level = %d", (int)m_profilingLevel);
        if (ProfilingLevel::BASIC == m_profilingLevel) {
            QNN_INFO("Basic profiling requested. Creating Qnn Profile object.");
            if (QNN_PROFILE_NO_ERROR != m_qnnFunctionPointers.qnnInterface.profileCreate(m_backendHandle, QNN_PROFILE_LEVEL_BASIC, &m_profileBackendHandle)) {
                QNN_WARN("Unable to create profile handle in the backend.");
                return StatusCode::FAILURE;
            }
        } else if (ProfilingLevel::DETAILED == m_profilingLevel) {
            QNN_INFO("Detailed profiling requested. Creating Qnn Profile object.");
            if (QNN_PROFILE_NO_ERROR != m_qnnFunctionPointers.qnnInterface.profileCreate(m_backendHandle, QNN_PROFILE_LEVEL_DETAILED, &m_profileBackendHandle)) {
                QNN_ERROR("Unable to create profile handle in the backend.");
                return StatusCode::FAILURE;
            }
        }
    }
    return StatusCode::SUCCESS;
}


StatusCode QnnManager::createContext() {
    if (QNN_CONTEXT_NO_ERROR !=
        m_qnnFunctionPointers.qnnInterface.contextCreate(m_backendHandle,
                                                         m_deviceHandle,
                                                         (const QnnContext_Config_t**)m_contextConfig,
                                                         &m_context)) {
        QNN_ERROR("Could not create context");
        return StatusCode::FAILURE;
    }
    m_isContextCreated = true;
    return StatusCode::SUCCESS;
}


// Calls composeGraph function in QNN's model.so.
// composeGraphs is supposed to populate graph related
// information in m_graphsInfo and m_graphsCount.
// m_debug is the option supplied to composeGraphs to
// say that all intermediate tensors including output tensors
// are expected to be read by the app.
StatusCode QnnManager::composeGraphs() {
    auto returnStatus = StatusCode::SUCCESS;
    auto composeGraphStatus = m_qnnFunctionPointers.composeGraphsFnHandle(
            m_backendHandle,
            m_qnnFunctionPointers.qnnInterface,
            m_context,
            (const qnn_wrapper_api::GraphConfigInfo_t**)m_graphConfigsInfo,
            m_graphConfigsInfoCount,
            &m_graphsInfo,
            &m_graphsCount,
            false
//            log::getLogCallback(),
//            log::getLogLevel()
            );
    if (qnn_wrapper_api::ModelError_t::MODEL_NO_ERROR != composeGraphStatus) {
        QNN_ERROR("Failed in composeGraphs()");
        returnStatus = StatusCode::FAILURE;
    }
    QNN_INFO("Graph compose status: %d", (unsigned int)composeGraphStatus);
    return returnStatus;
}


StatusCode QnnManager::finalizeGraphs() {
    for (size_t graphIdx = 0; graphIdx < m_graphsCount; graphIdx++) {
        auto status = m_qnnFunctionPointers.qnnInterface.graphFinalize(
                (*m_graphsInfo)[graphIdx].graph, m_profileBackendHandle, nullptr);
        if (QNN_COMMON_ERROR_MEM_ALLOC == status) {
            QNN_ERROR("Memory allocation error in graph finalize");
            return StatusCode::FAILURE;
        }
        if (QNN_GRAPH_NO_ERROR != status) {
            return StatusCode::FAILURE;
        }

    }
    if (ProfilingLevel::OFF != m_profilingLevel) {
        extractBackendProfilingInfo(m_profileBackendHandle);
    }
//    graphInfoMap_[qnnModelIndex_] = graphInfo;    mllm에서는 여러 그래프를 map에 저장
    return StatusCode::SUCCESS;
}



StatusCode QnnManager::createFromBinary() {
    QNN_WARN("createFromBinary() not implemented");
    return StatusCode::SUCCESS;
}



StatusCode QnnManager::createDevice() {
    QnnHtpDevice_CustomConfig_t customConfig;
    customConfig.option   = QNN_HTP_DEVICE_CONFIG_OPTION_SOC;
    customConfig.unnamedUnion.socModel = QNN_SOC_MODEL_SM8550;
    QnnDevice_Config_t devConfig;
    devConfig.option = QNN_DEVICE_CONFIG_OPTION_CUSTOM;
    devConfig.customConfig = &customConfig;

    const QnnDevice_Config_t* devConfigArray[] = {&devConfig, nullptr};

    if (nullptr != m_qnnFunctionPointers.qnnInterface.deviceCreate) {
        auto qnnStatus =
                m_qnnFunctionPointers.qnnInterface.deviceCreate(m_logHandle, devConfigArray, &m_deviceHandle);
        QNN_INFO("Device Creation Status: %d", qnnStatus);
        if (QNN_SUCCESS != qnnStatus && QNN_DEVICE_ERROR_UNSUPPORTED_FEATURE != qnnStatus) {
            QNN_ERROR("Failed to create device %u", qnnStatus);
            return verifyFailReturnStatus(qnnStatus);
        }
    }
    return StatusCode::SUCCESS;
}


StatusCode QnnManager::isDevicePropertySupported() {
    if (nullptr != m_qnnFunctionPointers.qnnInterface.propertyHasCapability) {
        auto qnnStatus =
                m_qnnFunctionPointers.qnnInterface.propertyHasCapability(QNN_PROPERTY_GROUP_DEVICE);
        if (QNN_PROPERTY_NOT_SUPPORTED == qnnStatus) {
            QNN_WARN("Device property is not supported");
        }
        if (QNN_PROPERTY_ERROR_UNKNOWN_KEY == qnnStatus) {
            QNN_ERROR("Device property is not known to backend");
            return StatusCode::FAILURE;
        }
    }
    return StatusCode::SUCCESS;
}


std::string QnnManager::getBackendBuildId() {
    char *backendBuildId{nullptr};
    if (QNN_SUCCESS != m_qnnFunctionPointers.qnnInterface.backendGetBuildId((const char **)&backendBuildId)) {
        QNN_ERROR("Unable to get build Id from the backend.");
    }
    return (backendBuildId == nullptr ? std::string("") : std::string(backendBuildId));
}

// Simple method to report error from app to lib.
void QnnManager::reportError(const std::string &err) {
    QNN_ERROR("%s", err.c_str());
    exit(1);
}


StatusCode QnnManager::verifyFailReturnStatus(Qnn_ErrorHandle_t errCode) {
    auto returnStatus = StatusCode::FAILURE;
    switch (errCode) {
        case QNN_COMMON_ERROR_SYSTEM_COMMUNICATION:
            returnStatus = StatusCode::FAILURE_SYSTEM_COMMUNICATION_ERROR;
            break;
        case QNN_COMMON_ERROR_SYSTEM:
            returnStatus = StatusCode::FAILURE_SYSTEM_ERROR;
            break;
        case QNN_COMMON_ERROR_NOT_SUPPORTED:
            returnStatus = StatusCode::QNN_FEATURE_UNSUPPORTED;
            break;
        default:
            break;
    }
    return returnStatus;
}


StatusCode QnnManager::extractBackendProfilingInfo (Qnn_ProfileHandle_t profileHandle) {
    if (nullptr == m_profileBackendHandle) {
        QNN_ERROR("Backend Profile handle is nullptr; may not be initialized.");
        return StatusCode::FAILURE;
    }
    const QnnProfile_EventId_t* profileEvents{nullptr};
    uint32_t numEvents{0};
    if (QNN_PROFILE_NO_ERROR != m_qnnFunctionPointers.qnnInterface.profileGetEvents(
            profileHandle, &profileEvents, &numEvents)) {
        QNN_ERROR("Failure in profile get events.");
        return StatusCode::FAILURE;
    }
    QNN_DEBUG("ProfileEvents: [%p], numEvents: [%d]", profileEvents, numEvents);
    for (size_t event = 0; event < numEvents; event++) {
        extractProfilingEvent(*(profileEvents + event));
        extractProfilingSubEvents(*(profileEvents + event));
    }
    return StatusCode::SUCCESS;
}


StatusCode QnnManager::extractProfilingSubEvents(QnnProfile_EventId_t profileEventId) {
    const QnnProfile_EventId_t* profileSubEvents{nullptr};
    uint32_t numSubEvents{0};
    if (QNN_PROFILE_NO_ERROR != m_qnnFunctionPointers.qnnInterface.profileGetSubEvents(
            profileEventId, &profileSubEvents, &numSubEvents)) {
        QNN_ERROR("Failure in profile get sub events.");
        return StatusCode::FAILURE;
    }
    QNN_DEBUG("ProfileSubEvents: [%p], numSubEvents: [%d]", profileSubEvents, numSubEvents);
    for (size_t subEvent = 0; subEvent < numSubEvents; subEvent++) {
        extractProfilingEvent(*(profileSubEvents + subEvent));
        extractProfilingSubEvents(*(profileSubEvents + subEvent));
    }
    return StatusCode::SUCCESS;
}


StatusCode QnnManager::extractProfilingEvent(QnnProfile_EventId_t profileEventId) {
    QnnProfile_EventData_t eventData;
    if (QNN_PROFILE_NO_ERROR !=
        m_qnnFunctionPointers.qnnInterface.profileGetEventData(profileEventId, &eventData)) {
        QNN_ERROR("Failure in profile get event type.");
        return StatusCode::FAILURE;
    }
    QNN_DEBUG("Printing Event Info - Event Type: [%d], Event Value: [%" "llu"
                      "], Event Identifier: [%s], Event Unit: [%d]",
              eventData.type,
              eventData.value,
              eventData.identifier,
              eventData.unit);
    return StatusCode::SUCCESS;
}



StatusCode QnnManager::fillDims(std::vector<size_t>& dims,uint32_t* inDimensions, uint32_t rank) {
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



StatusCode QnnManager::copyFromFloatToNative(float* floatBuffer, Qnn_Tensor_t* tensor) {
    if (nullptr == floatBuffer || nullptr == tensor) {
        QNN_ERROR("copyFromFloatToNative(): received a nullptr");
        return StatusCode::FAILURE;
    }

    StatusCode returnStatus = StatusCode::SUCCESS;
    std::vector<size_t> dims;
    fillDims(dims, QNN_TENSOR_GET_DIMENSIONS(tensor), QNN_TENSOR_GET_RANK(tensor));

    for(int i = 0; i < dims.size(); i++)
    {
        __android_log_print(ANDROID_LOG_ERROR, "QNN ", "******************************************************************** input dims = %lu\n >>>>",dims[i]);
    }
    switch (QNN_TENSOR_GET_DATA_TYPE(tensor)) {
        case QNN_DATATYPE_UFIXED_POINT_8:
            __android_log_print(ANDROID_LOG_ERROR, "QNN ","ufp8\n");
            datautil::floatToTfN<uint8_t>(static_cast<uint8_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
                                          floatBuffer,
                                          QNN_TENSOR_GET_QUANT_PARAMS(tensor).scaleOffsetEncoding.offset,
                                          QNN_TENSOR_GET_QUANT_PARAMS(tensor).scaleOffsetEncoding.scale,
                                          datautil::calculateElementCount(dims));
            break;

        case QNN_DATATYPE_UFIXED_POINT_16:
            __android_log_print(ANDROID_LOG_ERROR, "QNN ","ufp16\n");
            datautil::floatToTfN<uint16_t>(static_cast<uint16_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
                                           floatBuffer,
                                           QNN_TENSOR_GET_QUANT_PARAMS(tensor).scaleOffsetEncoding.offset,
                                           QNN_TENSOR_GET_QUANT_PARAMS(tensor).scaleOffsetEncoding.scale,
                                           datautil::calculateElementCount(dims));
            break;

        case QNN_DATATYPE_UINT_8:
            __android_log_print(ANDROID_LOG_ERROR, "QNN ","uint8\n");
            if (datautil::StatusCode::SUCCESS !=
                datautil::castFromFloat<uint8_t>(
                        static_cast<uint8_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
                        floatBuffer,
                        datautil::calculateElementCount(dims))) {
                QNN_ERROR("failure in castFromFloat<uint8_t>");
                returnStatus = StatusCode::FAILURE;
            }
            break;

        case QNN_DATATYPE_UINT_16:
            __android_log_print(ANDROID_LOG_ERROR, "QNN ","uint16\n");
            if (datautil::StatusCode::SUCCESS !=
                datautil::castFromFloat<uint16_t>(
                        static_cast<uint16_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
                        floatBuffer,
                        datautil::calculateElementCount(dims))) {
                QNN_ERROR("failure in castFromFloat<uint16_t>");
                returnStatus = StatusCode::FAILURE;
            }
            break;

        case QNN_DATATYPE_UINT_32:
            __android_log_print(ANDROID_LOG_ERROR, "QNN ","uint32\n");
            if (datautil::StatusCode::SUCCESS !=
                datautil::castFromFloat<uint32_t>(
                        static_cast<uint32_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
                        floatBuffer,
                        datautil::calculateElementCount(dims))) {
                QNN_ERROR("failure in castFromFloat<uint32_t>");
                returnStatus = StatusCode::FAILURE;
            }
            break;

        case QNN_DATATYPE_INT_8:
            __android_log_print(ANDROID_LOG_ERROR, "QNN ","int8\n");
            if (datautil::StatusCode::SUCCESS !=
                datautil::castFromFloat<int8_t>(
                        static_cast<int8_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
                        floatBuffer,
                        datautil::calculateElementCount(dims))) {
                QNN_ERROR("failure in castFromFloat<int8_t>");
                returnStatus = StatusCode::FAILURE;
            }
            break;

        case QNN_DATATYPE_INT_16:
            __android_log_print(ANDROID_LOG_ERROR, "QNN ","int16\n");
            if (datautil::StatusCode::SUCCESS !=
                datautil::castFromFloat<int16_t>(
                        static_cast<int16_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
                        floatBuffer,
                        datautil::calculateElementCount(dims))) {
                QNN_ERROR("failure in castFromFloat<int16_t>");
                returnStatus = StatusCode::FAILURE;
            }
            break;

        case QNN_DATATYPE_INT_32:
            __android_log_print(ANDROID_LOG_ERROR, "QNN ","int32\n");
            if (datautil::StatusCode::SUCCESS !=
                datautil::castFromFloat<int32_t>(
                        static_cast<int32_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
                        floatBuffer,
                        datautil::calculateElementCount(dims))) {
                QNN_ERROR("failure in castFromFloat<int32_t>");
                __android_log_print(ANDROID_LOG_ERROR, "QNN ","\"failure in castFromFloat<int32_t>\"\n");
                returnStatus = StatusCode::FAILURE;
            }
            break;

        case QNN_DATATYPE_BOOL_8:
            __android_log_print(ANDROID_LOG_ERROR, "QNN ","bool8\n");
            if (datautil::StatusCode::SUCCESS !=
                datautil::castFromFloat<uint8_t>(
                        static_cast<uint8_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
                        floatBuffer,
                        datautil::calculateElementCount(dims))) {
                QNN_ERROR("failure in castFromFloat<bool>");
                returnStatus = StatusCode::FAILURE;
            }
            break;

        default:
            QNN_ERROR("Datatype not supported yet!");
            __android_log_print(ANDROID_LOG_ERROR, "QNN ", "copyFromFloatToNative -> Datatype not supported yet!\n");
            returnStatus = StatusCode::FAILURE;
            break;
    }
    return returnStatus;
}