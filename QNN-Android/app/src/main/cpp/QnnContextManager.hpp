//
// Created by user on 2025-02-07.
//

#ifndef APP_QNNCONTEXTMANAGER_HPP
#define APP_QNNCONTEXTMANAGER_HPP


#pragma once
#include <vector>
#include "QnnMem.h"
#include "QnnTypeMacros.hpp"
#include "QnnLoader.hpp"
#include "QnnBackendManager.hpp"

class QnnContextManager {
public:
    StatusCode setup(QnnBackendManager& backendMgr, QnnLoader& loader, uint32_t vtcmSizeInMB);

    Qnn_ContextHandle_t getContext() { return m_context; }
    QnnContext_Config_t* getContextConfig() { return *m_contextConfig; }
    qnn_wrapper_api::GraphInfo_t** getGraphsInfo() const { return m_graphsInfo; }
    size_t getGraphsCount() const { return m_graphsCount; }
    void setVtcmSizeInMB(uint32_t vtcmSize) { vtcmSizeInMB = vtcmSize; }

private:
    StatusCode createContext(QnnBackendManager& backendMgr, QnnLoader& loader);
    StatusCode composeGraph(QnnBackendManager& backendMgr,QnnLoader& loader);
    StatusCode getGraphName(QnnBackendManager& backendMgr, QnnLoader& loader, char* graphName);
    StatusCode finalizeGraphs(QnnBackendManager& backendMgr, QnnLoader& loader);


private:
    Qnn_ContextHandle_t m_context       = nullptr;
    qnn_wrapper_api::GraphInfo_t** m_graphsInfo = nullptr;
    QnnContext_Config_t **m_contextConfig = nullptr;

    uint32_t m_graphsCount;
    uint32_t m_graphConfigsInfoCount;
    uint32_t vtcmSizeInMB;
};



#endif //APP_QNNCONTEXTMANAGER_HPP
