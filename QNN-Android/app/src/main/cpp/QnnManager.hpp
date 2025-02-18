//
// Created by user on 2025-01-10.
//
// Loading QNN Model by QNN API, not using TFLite

#ifndef QNNSKELETON_QNNMANAGER_HPP
#define QNNSKELETON_QNNMANAGER_HPP

#include "QnnLoader.hpp"
#include "QnnPowerManager.hpp"
#include "QnnBackendManager.hpp"
#include "QnnContextManager.hpp"
#include "QnnInferenceRunner.hpp"

#include "QnnInterface.h"
#include "QnnWrapperUtils.hpp"
#include "Utils/IOTensor.hpp"
#include "Utils/QnnSampleAppUtils.hpp"
#include "QNN/System/QnnSystemInterface.h"

#include <opencv2/core/types_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>


using namespace qnn::tools::sample_app;


class QnnManager {
public:
    QnnManager(const char* models, const char* backend, const int vtcmSizeInMB, const int offset);
    ~QnnManager() = default;

    std::unique_ptr<QnnLoader> &getLoader() { return m_loader; }
    std::unique_ptr<QnnBackendManager> &getBackendMgr() { return m_backendMgr; }
    std::unique_ptr<QnnPowerManager> &getPowerMgr() { return m_powerMgr; }
    std::unique_ptr<QnnContextManager> &getContextMgr() { return m_contextMgr; }

    StatusCode setPowerMode(int powerMode);
    StatusCode runInference(float32_t* inputBuffer);
    std::vector<std::pair<std::vector<size_t>, float32_t*>> m_inferData;

    double getAvgInferenceTime(int frameIdx) { return avgInferenceTime / frameIdx; }
    void addInferenceTime(double inferenceTime) {
        avgInferenceTime += inferenceTime;
        frameIdx++;
    }

private:
    static qnn_wrapper_api::ModelError_t QnnModel_freeGraphsInfo(qnn_wrapper_api::GraphInfoPtr_t **graphsInfo, uint32_t numGraphsInfo) {
        return qnn_wrapper_api::freeGraphsInfo(graphsInfo, numGraphsInfo);
    }

    StatusCode setup();
    StatusCode createFromBinary();
    StatusCode copyFromFloatToNative(float32_t* floatBuffer, Qnn_Tensor_t* tensor);


    // Sub-managers
    std::unique_ptr<QnnLoader> m_loader;
    std::unique_ptr<QnnBackendManager> m_backendMgr;
    std::unique_ptr<QnnPowerManager> m_powerMgr;
    std::unique_ptr<QnnContextManager> m_contextMgr;
    std::unique_ptr<QnnInferenceRunner> m_inferenceRunner;

    int frameIdx = 0;

    int offset = 0;
    int vtcmSizeInMB = 8;   // Default
    double avgInferenceTime = 0.0f;

};

#endif //QNNSKELETON_QNNMANAGER_HPP
