//
// Created by user on 2025-02-07.
//

#include "Log/Logger.hpp"
#include "HTP/QnnHtpGraph.h"
#include "HTP/QnnHtpContext.h"
#include "QnnContextManager.hpp"


// Create context and prepare the model
StatusCode QnnContextManager::setup(QnnBackendManager& backendMgr, QnnLoader& loader) {
    // Create context
    if (StatusCode::SUCCESS != createContext(backendMgr, loader)) {
        QNN_ERROR("Failed to create context");
        return StatusCode::FAILURE;
    }
    QNN_INFO("Context Created");

    // Compose graph
    if (StatusCode::SUCCESS != composeGraph(backendMgr, loader)) {
        QNN_ERROR("Failed to compose graph");
        return StatusCode::FAILURE;
    }
    QNN_INFO("Graph Prepared");

    if (StatusCode::SUCCESS != finalizeGraphs(backendMgr, loader)) {
        QNN_ERROR("Graph Finalize failure");
        return StatusCode::FAILURE;
    }
    QNN_INFO("Graph Finalized");

    return StatusCode::SUCCESS;
}


StatusCode QnnContextManager::createContext(QnnBackendManager& backendMgr, QnnLoader& loader) {
    if (QNN_CONTEXT_NO_ERROR !=
        loader.getFunctionPointers()
            .qnnInterface.contextCreate(loader.getBackendHandle(),
                                        backendMgr.getDeviceHandle(),
                                         (const QnnContext_Config_t**)m_contextConfig,
                                         &m_context)) {
        QNN_ERROR("Could not create context");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}



// Calls composeGraph function in QNN's model.so.
// composeGraphs is supposed to populate graph related
// information in m_graphsInfo and m_graphsCount.
// m_debug is the option supplied to composeGraphs to
// say that all intermediate tensors including output tensors
// are expected to be read by the app.
// TODO: Create graph without using composeGraph()?
StatusCode QnnContextManager::composeGraph(QnnBackendManager& backendMgr, QnnLoader& loader) {
    // Get graph name
    char graphName[500];
    if (StatusCode::SUCCESS != getGraphName(backendMgr, loader, graphName)) {
        QNN_ERROR("Failed to get graph name");
        return StatusCode::FAILURE;
    }

    // 1) A custom config structure for FLOAT16 precision
    static QnnHtpGraph_CustomConfig_t g_fp16PrecisionConfig = {
            .option    = QNN_HTP_GRAPH_CONFIG_OPTION_PRECISION,
            .precision = QNN_PRECISION_FLOAT16
    };

    // 2) Wrap that in a standard QnnGraph_Config_t
    static QnnGraph_Config_t g_fp16GraphConfig = {
            .option       = QNN_GRAPH_CONFIG_OPTION_CUSTOM,
            .unnamedUnion = { .customConfig = &g_fp16PrecisionConfig }
    };

    static const QnnGraph_Config_t* g_myGraphConfigs[] = {
            &g_fp16GraphConfig,
            nullptr             // always null-terminated
    };

    static qnn_wrapper_api::GraphConfigInfo_t g_fp16GraphConfigInfo = {
            /* name */         graphName,
            /* graphConfigs */ g_myGraphConfigs,
            // ... if there are other fields in GraphConfigInfo_t, set them as needed
    };

    static qnn_wrapper_api::GraphConfigInfo_t* g_allGraphConfigs[] = {
            &g_fp16GraphConfigInfo,
            nullptr // null-terminator if your code expects that
    };

    // Then the count:
    static constexpr uint32_t g_numGraphConfigs = 1; // One entry above

    auto returnStatus = StatusCode::SUCCESS;
    auto composeGraphStatus = loader.getFunctionPointers().composeGraphsFnHandle(
            loader.getBackendHandle(),
            loader.getFunctionPointers().qnnInterface,
            m_context,
            (const qnn_wrapper_api::GraphConfigInfo_t**)g_allGraphConfigs,
            g_numGraphConfigs,
            &m_graphsInfo,
            &m_graphsCount,
            false
    );
    if (qnn_wrapper_api::ModelError_t::MODEL_NO_ERROR != composeGraphStatus) {
        QNN_ERROR("Failed in composeGraphs()");
        returnStatus = StatusCode::FAILURE;
    }

    // Config FP16 after compose graph?
    for (int i = 0; i < m_graphsCount; i++) {
        QNN_INFO("Graph Config Info: %s", m_graphsInfo[i]->graphName);
    }

    QNN_INFO("Graph compose status: %d", (unsigned int)composeGraphStatus);
    return returnStatus;
}


StatusCode QnnContextManager::finalizeGraphs(QnnBackendManager& backendMgr, QnnLoader& loader) {
    for (size_t graphIdx = 0; graphIdx < m_graphsCount; graphIdx++) {
        auto status = loader.getFunctionPointers().qnnInterface.graphFinalize(
                (*m_graphsInfo)[graphIdx].graph, backendMgr.getProfileHandle(), nullptr);
        if (QNN_COMMON_ERROR_MEM_ALLOC == status) {
            QNN_ERROR("Memory allocation error in graph finalize");
            return StatusCode::FAILURE;
        }
        if (QNN_GRAPH_NO_ERROR != status) {
            return StatusCode::FAILURE;
        }

    }
    if (ProfilingLevel::OFF != backendMgr.getProfilingLevel()) {
        backendMgr.extractBackendProfilingInfo(loader);
    }
    return StatusCode::SUCCESS;
}




// Get graph name from user provided model by calling composeGraphs()
StatusCode QnnContextManager::getGraphName(QnnBackendManager& backendManager, QnnLoader& loader, char* graphName) {
    Qnn_ContextHandle_t tmp_context = nullptr;
    QnnContext_Config_t **tmp_contextConfig = nullptr;
    qnn_wrapper_api::GraphInfo_t **tmp_graphsInfo = nullptr;
    qnn_wrapper_api::GraphConfigInfo_t **tmp_graphConfigsInfo = nullptr;

    uint32_t tmp_graphsCount;
    uint32_t tmp_graphConfigsInfoCount;

    // Temporarily create context to get graph name
    loader.getFunctionPointers().qnnInterface.contextCreate(loader.getBackendHandle(),
                                                     backendManager.getDeviceHandle(),
                                                     (const QnnContext_Config_t**)tmp_contextConfig,
                                                     &tmp_context);

    auto composeGraphStatus = loader.getFunctionPointers().composeGraphsFnHandle(
            loader.getBackendHandle(),
            loader.getFunctionPointers().qnnInterface,
            tmp_context,
            (const qnn_wrapper_api::GraphConfigInfo_t**)tmp_graphConfigsInfo,
            tmp_graphConfigsInfoCount,
            &tmp_graphsInfo,
            &tmp_graphsCount,
            false
    );
    if (qnn_wrapper_api::ModelError_t::MODEL_NO_ERROR != composeGraphStatus) {
        QNN_ERROR("Failed in composeGraphs()");
        return StatusCode::FAILURE;
    }

    // Assume single graph for now
    if (tmp_graphsCount == 1) {
        strcpy(graphName, tmp_graphsInfo[0]->graphName);
    } else {
        QNN_ERROR("Multiple graphs not supported");
        return StatusCode::FAILURE;
    }
    QNN_INFO("Graph Name: %s", graphName);

    // Free graph
//    auto status = QnnModel_freeGraphsInfo(
//            reinterpret_cast<qnn_wrapper_api::GraphInfoPtr_t **>(tmp_graphsInfo), tmp_graphsCount);
//    if (qnn_wrapper_api::ModelError_t::MODEL_NO_ERROR != status) {
//        return StatusCode::FAILURE;
//    }
    loader.getFunctionPointers().qnnInterface.contextFree(tmp_context, nullptr);
    return StatusCode::SUCCESS;
}