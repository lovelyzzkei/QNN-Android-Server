//
// Created by user on 2025-02-06.
//

#include "QnnLoader.hpp"
#include "Log/Logger.hpp"
#include "Utils/DynamicLoadUtil.hpp"


using namespace qnn;
using namespace qnn::tools;
//using namespace qnn::tools::sample_app;


QnnLoader::QnnLoader(const std::string& modelPath, const std::string& backendName)
        : m_modelPath(modelPath), m_backendName(backendName) {}


QnnLoader::~QnnLoader() {
    // free backend, model .so if needed
}


StatusCode QnnLoader::initializeQnnFunctionPointers() {
    std::string backEndLib;
    if (m_backendName == "NPU") {
        backEndLib = "libQnnHtp.so";
    } else if (m_backendName == "CPU") {
        backEndLib = "libQnnCpu.so";
    }

    QNN_INFO("Model: %s", m_modelPath.c_str());
    QNN_INFO("Backend: %s", backEndLib.c_str());

    // Load backend and model .so and validate all the required function symbols are resolved
    // QNN Interface mechanism can be used to set up a table of function pointers to QNN APIs
    // in the backend instead of manually resolving symbols to each and every API, which makes
    // resolving symbols easy.
    auto statusCode = dynamicloadutil::getQnnFunctionPointers(
            backEndLib,
            m_modelPath,
            &m_qnnFunctionPointers,
            &m_backendHandle,
            true,        // True
            &m_modelHandle);


    if (dynamicloadutil::StatusCode::SUCCESS != statusCode) {
        if (dynamicloadutil::StatusCode::FAIL_LOAD_BACKEND == statusCode) {
            QNN_ERROR("Error initializing QNN Function Pointers: could not load backend: %s", backEndLib.c_str());
        } else if (dynamicloadutil::StatusCode::FAIL_LOAD_MODEL == statusCode) {
            QNN_ERROR("Error initializing QNN Function Pointers: could not load model: %s", m_modelPath.c_str());
        } else {
            QNN_ERROR("Error initializing QNN Function Pointers");
        }
        return StatusCode::FAILURE;
    }
    QNN_INFO("[Loader] QNN Function Pointers initialized");
    return StatusCode::SUCCESS;
}