//
// Created by user on 2025-02-06.
//

#ifndef APP_QNNBACKENDMANAGER_HPP
#define APP_QNNBACKENDMANAGER_HPP

#include <vector>
#include <unordered_map>
#include "QnnLoader.hpp"

struct OpStats {
    std::string identifier;
    uint64_t totalCycles = 0;
    uint32_t count = 0;
};


class QnnBackendManager {
public:
    QnnBackendManager() = default;
    StatusCode setup(QnnLoader& loader);
    std::string getBackendBuildId(QnnLoader& loader);

    Qnn_DeviceHandle_t getDeviceHandle() const { return m_deviceHandle; }
    Qnn_ProfileHandle_t getProfileHandle() const { return m_profileBackendHandle; }
    ProfilingLevel getProfilingLevel() const { return m_profilingLevel; }

    StatusCode extractBackendProfilingInfo (QnnLoader& loader);

    void addDataToStats(std::string identifier, uint64_t cycles);
    void saveStatsAsCsv(const std::string& fileName);
    int nodeNum = 0;
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

    std::vector<OpStats> stats;
};



#endif //APP_QNNBACKENDMANAGER_HPP
