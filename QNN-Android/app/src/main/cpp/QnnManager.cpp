// Mainly reference from UbiquitousLearning/mllm and SampleApp from QNN

#include <string>

#include "Log/Logger.hpp"

#include "QnnManager.hpp"



using namespace qnn;
using namespace qnn::tools;
using namespace qnn::tools::sample_app;


QnnManager::QnnManager(const char* model, const char* backend) {
    std::string modelPath = std::string(model);
    std::string backEndPath = std::string(backend);

    m_loader        = std::make_unique<QnnLoader>(modelPath, backEndPath);
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
    status = m_contextMgr->setup(*m_backendMgr, *m_loader);
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
    // Delegates the actual run to m_inferenceRunner
    auto status = m_inferenceRunner->execute(inputBuffer, *m_loader, *m_backendMgr);
    if (status != StatusCode::SUCCESS) {
        return status;
    }
    // Retrieve the outputs from the runner if needed
    m_inferData = m_inferenceRunner->getOutputs();
    return status;
}




StatusCode QnnManager::createFromBinary() {
    QNN_WARN("createFromBinary() not implemented");
    return StatusCode::SUCCESS;
}




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