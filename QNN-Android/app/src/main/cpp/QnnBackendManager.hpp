//
// Created by user on 2025-02-06.
//

#ifndef APP_QNNBACKENDMANAGER_HPP
#define APP_QNNBACKENDMANAGER_HPP

#include "QnnLoader.hpp"

class QnnBackendManager {
public:
    QnnBackendManager() = default;
    StatusCode setup(QnnLoader& loader);
    std::string getBackendBuildId(QnnLoader& loader);

    Qnn_DeviceHandle_t getDeviceHandle() const { return m_deviceHandle; }
    Qnn_ProfileHandle_t getProfileHandle() const { return m_profileBackendHandle; }
    ProfilingLevel getProfilingLevel() const { return m_profilingLevel; }

    StatusCode extractBackendProfilingInfo (QnnLoader& loader);

private:
    StatusCode initializeLogging(QnnLoader& loader);
    StatusCode initializeBackend(QnnLoader& loader);
    StatusCode initializeProfiling(QnnLoader& loader);
    StatusCode isDevicePropertySupported(QnnLoader& loader);

    // Possibly device creation
    StatusCode createDevice(QnnLoader& loader);

    StatusCode extractProfilingSubEvents(QnnLoader& loader, QnnProfile_EventId_t profileEventId);
    StatusCode extractProfilingEvent(QnnLoader& loader, QnnProfile_EventId_t profileEventId);


    Qnn_LogHandle_t m_logHandle          = nullptr;
    Qnn_DeviceHandle_t m_deviceHandle    = nullptr;
    Qnn_ProfileHandle_t m_profileBackendHandle = nullptr;

    QnnBackend_Config_t **m_backendConfig = nullptr;
    ProfilingLevel m_profilingLevel = ProfilingLevel::BASIC;


};



#endif //APP_QNNBACKENDMANAGER_HPP
