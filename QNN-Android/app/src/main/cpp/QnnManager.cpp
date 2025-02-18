// Mainly reference from UbiquitousLearning/mllm and SampleApp from QNN

#include <string>

#include "Log/Logger.hpp"

#include "QnnManager.hpp"



using namespace qnn;
using namespace qnn::tools;
using namespace qnn::tools::sample_app;


QnnManager::QnnManager(const char* model, const char* backend, const int vtcmSizeInMB, const int offset) {
    std::string backEndPath = std::string(backend);
    this->vtcmSizeInMB = vtcmSizeInMB;
    this->offset = offset;

    m_loader        = std::make_unique<QnnLoader>(model, backEndPath);
    m_backendMgr    = std::make_unique<QnnBackendManager>();
    m_powerMgr      = std::make_unique<QnnPowerManager>();
    m_contextMgr    = std::make_unique<QnnContextManager>();
    m_inferenceRunner = std::make_unique<QnnInferenceRunner>();

    // Prepare QNN resources
    this->setup();

    QNN_INFO("All preparation for model done");
}




StatusCode QnnManager::setup() {
    // Step A: load the QNN function pointers
    StatusCode status = m_loader->initializeQnnFunctionPointers();
    if (status != StatusCode::SUCCESS) {
        QNN_ERROR("Failed in loader->initializeQnnFunctionPointers()");
        return status;
    }

    // Step B: initialize backend + device
    status = m_backendMgr->setup(*m_loader);
    if (status != StatusCode::SUCCESS) {
        QNN_ERROR("Backend init failed");
        return status;
    }
    QNN_INFO("Backend build version: %s", m_backendMgr->getBackendBuildId(*m_loader).c_str());


    // Step C: set up power manager
    status = m_powerMgr->setup(*m_loader);
    if (status != StatusCode::SUCCESS) {
        QNN_ERROR("Power manager setup failed");
        return status;
    }

    // Step D: create context, compose graphs
    status = m_contextMgr->setup(*m_backendMgr, *m_loader, vtcmSizeInMB);
    if (status != StatusCode::SUCCESS) {
        QNN_ERROR("Context creation or graph compose failed");
        return status;
    }
    QNN_INFO("Context manager Created");

    // Step F: set up inference runner with the context, etc.
    m_inferenceRunner->setup(m_contextMgr->getGraphsInfo(),
                              m_contextMgr->getGraphsCount(),
                              m_backendMgr->getProfileHandle());

    QNN_INFO("prepare() finished");
    return StatusCode::SUCCESS;
}


StatusCode QnnManager::runInference(float32_t* inputBuffer) {
    // Set power configuration
//    m_powerMgr->setPowerConfig(*m_loader);

    // Delegates the actual run to m_inferenceRunner
    auto start = std::chrono::high_resolution_clock::now();
    auto status = m_inferenceRunner->execute(inputBuffer, *m_loader, *m_backendMgr, *m_contextMgr, vtcmSizeInMB, offset);
    if (status != StatusCode::SUCCESS) {
        return status;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    addInferenceTime(duration.count());
    QNN_INFO("Average inference time: %.3f ms", getAvgInferenceTime(frameIdx));

    // Retrieve the outputs from the runner if needed
    m_inferData = m_inferenceRunner->getOutputs();

    // Store stats
//    if (frameIdx == 100) {
//        char filename[100];
//        sprintf(filename, "/sdcard/download/yolov6_480x640_vtcm_size_%dMB.csv", vtcmSizeInMB);
//        m_backendMgr->saveStatsAsCsv(filename);
//    }

    return status;
}


StatusCode QnnManager::setPowerMode(int powerMode) {
    auto status = m_powerMgr->setPowerConfig(static_cast<PowerModeType>(powerMode), *m_loader);
    if (status != StatusCode::SUCCESS) {
        QNN_ERROR("Failed to set power mode");
        return status;
    }
    QNN_INFO("Power mode set to %d", powerMode);
    return status;
}




//StatusCode QnnManager::createFromBinary(char* buffer, long bufferSize) {
//    if (nullptr == m_qnnFunctionPointers.qnnSystemInterface.systemContextCreate ||
//        nullptr == m_qnnFunctionPointers.qnnSystemInterface.systemContextGetBinaryInfo ||
//        nullptr == m_qnnFunctionPointers.qnnSystemInterface.systemContextFree) {
//        __android_log_print(ANDROID_LOG_ERROR, "QNN ", "QNN System function pointers are not populated\n");
//        return StatusCode::FAILURE;
//    }
//
//    if (0 == bufferSize) {
//        __android_log_print(ANDROID_LOG_ERROR, "QNN ", "Received path to an empty file. Nothing to deserialize\n");
//        return StatusCode::FAILURE;
//    }
//    if (!buffer) {
//        __android_log_print(ANDROID_LOG_ERROR, "QNN ", "Failed to allocate memory.\n");
//        return StatusCode::FAILURE;
//    }
//
//    // inspect binary info
//    auto returnStatus = StatusCode::SUCCESS;
//    QnnSystemContext_Handle_t sysCtxHandle{nullptr};
//    if (QNN_SUCCESS != m_qnnFunctionPointers.qnnSystemInterface.systemContextCreate(&sysCtxHandle)) {
//        __android_log_print(ANDROID_LOG_ERROR, "QNN ", "Could not create system handle.\n");
//        returnStatus = StatusCode::FAILURE;
//    }
//    const QnnSystemContext_BinaryInfo_t* binaryInfo{nullptr};
//    Qnn_ContextBinarySize_t binaryInfoSize{0};
//    if (StatusCode::SUCCESS == returnStatus &&
//        QNN_SUCCESS != m_qnnFunctionPointers.qnnSystemInterface.systemContextGetBinaryInfo(
//                sysCtxHandle,
//                static_cast<void*>(buffer),
//                bufferSize,
//                &binaryInfo,
//                &binaryInfoSize)) {
//        __android_log_print(ANDROID_LOG_ERROR, "QNN ", "Failed to get context binary info\n");
//        returnStatus = StatusCode::FAILURE;
//    }
//
//    // fill GraphInfo_t based on binary info
//    if (StatusCode::SUCCESS == returnStatus &&
//        !copyMetadataToGraphsInfo(binaryInfo, m_graphsInfo, m_graphsCount)) {
//        __android_log_print(ANDROID_LOG_ERROR, "QNN ", "Failed to copy metadata.\n");
//        returnStatus = StatusCode::FAILURE;
//    }
//    m_qnnFunctionPointers.qnnSystemInterface.systemContextFree(sysCtxHandle);
//    sysCtxHandle = nullptr;
//
//    if (StatusCode::SUCCESS == returnStatus &&
//        nullptr == m_qnnFunctionPointers.qnnInterface.contextCreateFromBinary) {
//        __android_log_print(ANDROID_LOG_ERROR, "QNN ", "contextCreateFromBinaryFnHandle is nullptr.\n");
//        returnStatus = StatusCode::FAILURE;
//    }
//    if (StatusCode::SUCCESS == returnStatus &&
//        m_qnnFunctionPointers.qnnInterface.contextCreateFromBinary(
//                m_backendHandle,
//                m_deviceHandle,
//                (const QnnContext_Config_t**)&m_contextConfig,
//                static_cast<void*>(buffer),
//                bufferSize,
//                &m_context,
//                m_profileBackendHandle)) {
//        __android_log_print(ANDROID_LOG_ERROR, "QNN ", "Could not create context from binary.\n");
//        returnStatus = StatusCode::FAILURE;
//    }
//    if (ProfilingLevel::OFF != m_profilingLevel) {
//        extractBackendProfilingInfo(m_profileBackendHandle);
//    }
//    m_isContextCreated = true;
//    if (StatusCode::SUCCESS == returnStatus) {
//        for (size_t graphIdx = 0; graphIdx < m_graphsCount; graphIdx++) {
//            if (nullptr == m_qnnFunctionPointers.qnnInterface.graphRetrieve) {
//                __android_log_print(ANDROID_LOG_ERROR, "QNN ", "graphRetrieveFnHandle is nullptr.\n");
//                returnStatus = StatusCode::FAILURE;
//                break;
//            }
//            if (QNN_SUCCESS !=
//                m_qnnFunctionPointers.qnnInterface.graphRetrieve(
//                        m_context, (*m_graphsInfo)[graphIdx].graphName, &((*m_graphsInfo)[graphIdx].graph))) {
//                __android_log_print(ANDROID_LOG_ERROR, "QNN ", "Unable to retrieve graph handle for graph Idx.\n");
//                returnStatus = StatusCode::FAILURE;
//            }
//        }
//    }
//    if (StatusCode::SUCCESS != returnStatus) {
//        //qnn_wrapper_api::freeGraphsInfo(&m_graphsInfo, m_graphsCount);
//    }
//    return returnStatus;
//}




//StatusCode QnnManager::copyFromFloatToNative(float* floatBuffer, Qnn_Tensor_t* tensor) {
//    if (nullptr == floatBuffer || nullptr == tensor) {
//        QNN_ERROR("copyFromFloatToNative(): received a nullptr");
//        return StatusCode::FAILURE;
//    }
//
//    StatusCode returnStatus = StatusCode::SUCCESS;
//    std::vector<size_t> dims;
//    fillDims(dims, QNN_TENSOR_GET_DIMENSIONS(tensor), QNN_TENSOR_GET_RANK(tensor));
//
//    for(int i = 0; i < dims.size(); i++)
//    {
//        __android_log_print(ANDROID_LOG_ERROR, "QNN ", "******************************************************************** input dims = %lu\n >>>>",dims[i]);
//    }
//    switch (QNN_TENSOR_GET_DATA_TYPE(tensor)) {
//        case QNN_DATATYPE_UFIXED_POINT_8:
//            __android_log_print(ANDROID_LOG_ERROR, "QNN ","ufp8\n");
//            datautil::floatToTfN<uint8_t>(static_cast<uint8_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
//                                          floatBuffer,
//                                          QNN_TENSOR_GET_QUANT_PARAMS(tensor).scaleOffsetEncoding.offset,
//                                          QNN_TENSOR_GET_QUANT_PARAMS(tensor).scaleOffsetEncoding.scale,
//                                          datautil::calculateElementCount(dims));
//            break;
//
//        case QNN_DATATYPE_UFIXED_POINT_16:
//            __android_log_print(ANDROID_LOG_ERROR, "QNN ","ufp16\n");
//            datautil::floatToTfN<uint16_t>(static_cast<uint16_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
//                                           floatBuffer,
//                                           QNN_TENSOR_GET_QUANT_PARAMS(tensor).scaleOffsetEncoding.offset,
//                                           QNN_TENSOR_GET_QUANT_PARAMS(tensor).scaleOffsetEncoding.scale,
//                                           datautil::calculateElementCount(dims));
//            break;
//
//        case QNN_DATATYPE_UINT_8:
//            __android_log_print(ANDROID_LOG_ERROR, "QNN ","uint8\n");
//            if (datautil::StatusCode::SUCCESS !=
//                datautil::castFromFloat<uint8_t>(
//                        static_cast<uint8_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
//                        floatBuffer,
//                        datautil::calculateElementCount(dims))) {
//                QNN_ERROR("failure in castFromFloat<uint8_t>");
//                returnStatus = StatusCode::FAILURE;
//            }
//            break;
//
//        case QNN_DATATYPE_UINT_16:
//            __android_log_print(ANDROID_LOG_ERROR, "QNN ","uint16\n");
//            if (datautil::StatusCode::SUCCESS !=
//                datautil::castFromFloat<uint16_t>(
//                        static_cast<uint16_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
//                        floatBuffer,
//                        datautil::calculateElementCount(dims))) {
//                QNN_ERROR("failure in castFromFloat<uint16_t>");
//                returnStatus = StatusCode::FAILURE;
//            }
//            break;
//
//        case QNN_DATATYPE_UINT_32:
//            __android_log_print(ANDROID_LOG_ERROR, "QNN ","uint32\n");
//            if (datautil::StatusCode::SUCCESS !=
//                datautil::castFromFloat<uint32_t>(
//                        static_cast<uint32_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
//                        floatBuffer,
//                        datautil::calculateElementCount(dims))) {
//                QNN_ERROR("failure in castFromFloat<uint32_t>");
//                returnStatus = StatusCode::FAILURE;
//            }
//            break;
//
//        case QNN_DATATYPE_INT_8:
//            __android_log_print(ANDROID_LOG_ERROR, "QNN ","int8\n");
//            if (datautil::StatusCode::SUCCESS !=
//                datautil::castFromFloat<int8_t>(
//                        static_cast<int8_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
//                        floatBuffer,
//                        datautil::calculateElementCount(dims))) {
//                QNN_ERROR("failure in castFromFloat<int8_t>");
//                returnStatus = StatusCode::FAILURE;
//            }
//            break;
//
//        case QNN_DATATYPE_INT_16:
//            __android_log_print(ANDROID_LOG_ERROR, "QNN ","int16\n");
//            if (datautil::StatusCode::SUCCESS !=
//                datautil::castFromFloat<int16_t>(
//                        static_cast<int16_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
//                        floatBuffer,
//                        datautil::calculateElementCount(dims))) {
//                QNN_ERROR("failure in castFromFloat<int16_t>");
//                returnStatus = StatusCode::FAILURE;
//            }
//            break;
//
//        case QNN_DATATYPE_INT_32:
//            __android_log_print(ANDROID_LOG_ERROR, "QNN ","int32\n");
//            if (datautil::StatusCode::SUCCESS !=
//                datautil::castFromFloat<int32_t>(
//                        static_cast<int32_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
//                        floatBuffer,
//                        datautil::calculateElementCount(dims))) {
//                QNN_ERROR("failure in castFromFloat<int32_t>");
//                __android_log_print(ANDROID_LOG_ERROR, "QNN ","\"failure in castFromFloat<int32_t>\"\n");
//                returnStatus = StatusCode::FAILURE;
//            }
//            break;
//
//        case QNN_DATATYPE_BOOL_8:
//            __android_log_print(ANDROID_LOG_ERROR, "QNN ","bool8\n");
//            if (datautil::StatusCode::SUCCESS !=
//                datautil::castFromFloat<uint8_t>(
//                        static_cast<uint8_t*>(QNN_TENSOR_GET_CLIENT_BUF(tensor).data),
//                        floatBuffer,
//                        datautil::calculateElementCount(dims))) {
//                QNN_ERROR("failure in castFromFloat<bool>");
//                returnStatus = StatusCode::FAILURE;
//            }
//            break;
//
//        default:
//            QNN_ERROR("Datatype not supported yet!");
//            __android_log_print(ANDROID_LOG_ERROR, "QNN ", "copyFromFloatToNative -> Datatype not supported yet!\n");
//            returnStatus = StatusCode::FAILURE;
//            break;
//    }
//    return returnStatus;
//}