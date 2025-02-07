//
// Created by user on 2025-02-07.
//

#ifndef APP_QNNPOWERMANAGER_HPP
#define APP_QNNPOWERMANAGER_HPP

#include "QnnLoader.hpp"

class QnnPowerManager {
public:
    QnnPowerManager() = default;

    StatusCode setup(QnnLoader& loader);
    StatusCode createPowerConfigId(QnnLoader& loader);
    StatusCode setPowerConfig(QnnLoader& loader);
    StatusCode setRpcLatencyAndPolling();


private:
    uint32_t m_powerConfigId;
    uint32_t m_deviceId = 0;
    uint32_t m_coreId = 0;
};


#endif //APP_QNNPOWERMANAGER_HPP
