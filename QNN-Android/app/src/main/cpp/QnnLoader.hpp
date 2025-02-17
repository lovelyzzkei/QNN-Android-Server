//
// Created by user on 2025-02-06.
//

#ifndef APP_QNNLOADER_HPP
#define APP_QNNLOADER_HPP


#pragma once
#include <vector>
#include <string>
#include "QnnTypeMacros.hpp"
#include "Utils/DynamicLoadUtil.hpp"

class QnnLoader {
public:
    QnnLoader(const std::string& modelPath, const std::string& backendName);
//    QnnLoader(const std::vector<std::string>& modelPaths, const std::string& backendName);
    ~QnnLoader();

    StatusCode initializeQnnFunctionPointers();

    // Getters for loaded handles or function pointers
    QnnFunctionPointers& getFunctionPointers() { return m_qnnFunctionPointers; }

    void* getBackendHandle() const { return m_backendHandle; }
    void* getModelHandle() const { return m_modelHandle; }

    void* setBackendHandle(void* handle) { m_backendHandle = handle; }

    // Getter that returns the pointer to the backend handle variable.
    Qnn_BackendHandle_t* getBackendHandlePtr() {
        return &m_backendHandle;
    }


private:
    std::string m_backendName;
    std::string m_modelPath;

    void* m_backendHandle = nullptr;
    void* m_modelHandle   = nullptr;

    QnnFunctionPointers m_qnnFunctionPointers;
};

#endif //APP_QNNLOADER_HPP
