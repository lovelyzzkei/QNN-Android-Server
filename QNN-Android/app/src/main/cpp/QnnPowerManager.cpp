//
// Created by user on 2025-02-07.
//

#include "Log/Logger.hpp"
#include "QnnPowerManager.hpp"
#include "QNN/HTP/QnnHtpDevice.h"


StatusCode QnnPowerManager::setup(QnnLoader& loader) {
    if (createPowerConfigId(loader) != StatusCode::SUCCESS) {
        QNN_ERROR("createPowerConfigId failed");
        return StatusCode::FAILURE;
    }

    QNN_INFO("[QnnPowerManager] Power configuration success. PowerConfigId: %d", m_powerConfigId);
    return StatusCode::SUCCESS;
}


StatusCode QnnPowerManager::createPowerConfigId(QnnLoader& loader) {
    QnnDevice_Infrastructure_t deviceInfra = nullptr;
    Qnn_ErrorHandle_t devErr = loader.getFunctionPointers()
            .qnnInterface.deviceGetInfrastructure(&deviceInfra);
    if (devErr != QNN_SUCCESS) {
        QNN_ERROR("device error");
        return StatusCode::FAILURE;
    }
    QnnHtpDevice_Infrastructure_t *htpInfra = static_cast<QnnHtpDevice_Infrastructure_t *>(deviceInfra);
    QnnHtpDevice_PerfInfrastructure_t perfInfra = htpInfra->perfInfra;
    Qnn_ErrorHandle_t perfInfraErr = perfInfra.createPowerConfigId(
            m_deviceId,
            m_coreId,
            &m_powerConfigId);
    if (perfInfraErr != QNN_SUCCESS) {
        QNN_ERROR("createPowerConfigId failed");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}



StatusCode QnnPowerManager::setPowerConfig(PowerModeType powerMode, QnnLoader& loader) {
    QnnDevice_Infrastructure_t deviceInfra = nullptr;
    Qnn_ErrorHandle_t devErr = loader.getFunctionPointers()
            .qnnInterface.deviceGetInfrastructure(&deviceInfra);
    if (devErr != QNN_SUCCESS) {
        QNN_ERROR("device error");
        return StatusCode::FAILURE;
    }
    QnnHtpDevice_Infrastructure_t *htpInfra = static_cast<QnnHtpDevice_Infrastructure_t *>(deviceInfra);
    QnnHtpDevice_PerfInfrastructure_t perfInfra = htpInfra->perfInfra;

    // Prepare a power configuration structure.
    QnnHtpPerfInfrastructure_PowerConfig_t powerConfig;
    memset(&powerConfig, 0, sizeof(powerConfig));

    // Set common options
    powerConfig.option                     = QNN_HTP_PERF_INFRASTRUCTURE_POWER_CONFIGOPTION_DCVS_V3;
    powerConfig.dcvsV3Config.contextId     = m_powerConfigId;  //use the power config id created
    powerConfig.dcvsV3Config.sleepDisable    = 1; //True to disable sleep, False to re-enable sleep
    powerConfig.dcvsV3Config.setSleepDisable = 1; //True to consider sleep disable/enable parameter otherwise False
    powerConfig.dcvsV3Config.powerMode = QNN_HTP_PERF_INFRASTRUCTURE_POWERMODE_PERFORMANCE_MODE;
    powerConfig.dcvsV3Config.setBusParams = 1;
    powerConfig.dcvsV3Config.setCoreParams   = 1;

    // Configure the parameters based on the user-selected power mode.
    switch (powerMode) {
        case PowerModeType::BURST:
            powerConfig.dcvsV3Config.setSleepLatency = 1;
            powerConfig.dcvsV3Config.sleepLatency = 40;  // 40 us
            powerConfig.dcvsV3Config.dcvsEnable = 0;   // DCVS disabled
            powerConfig.dcvsV3Config.setDcvsEnable = 1;
            powerConfig.dcvsV3Config.busVoltageCornerMin = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
            powerConfig.dcvsV3Config.busVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
            powerConfig.dcvsV3Config.busVoltageCornerMax = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
            powerConfig.dcvsV3Config.setSleepDisable = 1;
            powerConfig.dcvsV3Config.coreVoltageCornerMin = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
            powerConfig.dcvsV3Config.coreVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
            powerConfig.dcvsV3Config.coreVoltageCornerMax = DCVS_VOLTAGE_VCORNER_MAX_VOLTAGE_CORNER;
            // Disable sleep (for burst mode, you may want to avoid sleep)
            break;

        case PowerModeType::SUSTAINED_HIGH_PERFORMANCE:
        case PowerModeType::HIGH_PERFORMANCE:
            powerConfig.dcvsV3Config.setSleepLatency = 1;
            powerConfig.dcvsV3Config.sleepLatency = 100;  // 100 us
            powerConfig.dcvsV3Config.dcvsEnable = 0;    // DCVS disabled
            powerConfig.dcvsV3Config.busVoltageCornerMin = DCVS_VOLTAGE_VCORNER_TURBO;
            powerConfig.dcvsV3Config.busVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_TURBO;
            powerConfig.dcvsV3Config.busVoltageCornerMax = DCVS_VOLTAGE_VCORNER_TURBO;
            powerConfig.dcvsV3Config.setSleepDisable = 1;
            powerConfig.dcvsV3Config.coreVoltageCornerMin = DCVS_VOLTAGE_VCORNER_TURBO;
            powerConfig.dcvsV3Config.coreVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_TURBO;
            powerConfig.dcvsV3Config.coreVoltageCornerMax = DCVS_VOLTAGE_VCORNER_TURBO;
            break;
        case PowerModeType::BALANCED:
            powerConfig.dcvsV3Config.setSleepLatency = 1;
            powerConfig.dcvsV3Config.sleepLatency = 1000;  // 100 us
            powerConfig.dcvsV3Config.dcvsEnable = 1;    // DCVS disabled
            powerConfig.dcvsV3Config.busVoltageCornerMin = DCVS_VOLTAGE_VCORNER_NOM_PLUS;
            powerConfig.dcvsV3Config.busVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_NOM_PLUS;
            powerConfig.dcvsV3Config.busVoltageCornerMax = DCVS_VOLTAGE_VCORNER_NOM_PLUS;
            powerConfig.dcvsV3Config.setSleepDisable = 1;
            powerConfig.dcvsV3Config.coreVoltageCornerMin = DCVS_VOLTAGE_VCORNER_NOM_PLUS;
            powerConfig.dcvsV3Config.coreVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_NOM_PLUS;
            powerConfig.dcvsV3Config.coreVoltageCornerMax = DCVS_VOLTAGE_VCORNER_NOM_PLUS;
            break;
        case PowerModeType::LOW_BALANCED:
            powerConfig.dcvsV3Config.setSleepLatency = 1;
            powerConfig.dcvsV3Config.sleepLatency = 1000;
            powerConfig.dcvsV3Config.dcvsEnable = 1;
            powerConfig.dcvsV3Config.busVoltageCornerMin = DCVS_VOLTAGE_VCORNER_NOM;
            powerConfig.dcvsV3Config.busVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_NOM;
            powerConfig.dcvsV3Config.busVoltageCornerMax = DCVS_VOLTAGE_VCORNER_NOM;
            powerConfig.dcvsV3Config.setSleepDisable = 1;
            powerConfig.dcvsV3Config.coreVoltageCornerMin = DCVS_VOLTAGE_VCORNER_NOM;
            powerConfig.dcvsV3Config.coreVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_NOM;
            powerConfig.dcvsV3Config.coreVoltageCornerMax = DCVS_VOLTAGE_VCORNER_NOM;
            break;
        case PowerModeType::HIGH_POWER_SAVER:
            powerConfig.dcvsV3Config.setSleepLatency = 1;
            powerConfig.dcvsV3Config.sleepLatency = 1000;
            powerConfig.dcvsV3Config.dcvsEnable = 1;
            powerConfig.dcvsV3Config.busVoltageCornerMin = DCVS_VOLTAGE_VCORNER_SVS_PLUS;
            powerConfig.dcvsV3Config.busVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_SVS_PLUS;
            powerConfig.dcvsV3Config.busVoltageCornerMax = DCVS_VOLTAGE_VCORNER_SVS_PLUS;
            powerConfig.dcvsV3Config.setSleepDisable = 1;
            powerConfig.dcvsV3Config.coreVoltageCornerMin = DCVS_VOLTAGE_VCORNER_SVS_PLUS;
            powerConfig.dcvsV3Config.coreVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_SVS_PLUS;
            powerConfig.dcvsV3Config.coreVoltageCornerMax = DCVS_VOLTAGE_VCORNER_SVS_PLUS;
            break;
        case PowerModeType::POWER_SAVER:
            powerConfig.dcvsV3Config.setSleepLatency = 1;
            powerConfig.dcvsV3Config.sleepLatency = 1000;
            powerConfig.dcvsV3Config.dcvsEnable = 1;
            powerConfig.dcvsV3Config.busVoltageCornerMin = DCVS_VOLTAGE_VCORNER_SVS;
            powerConfig.dcvsV3Config.busVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_SVS;
            powerConfig.dcvsV3Config.busVoltageCornerMax = DCVS_VOLTAGE_VCORNER_SVS;
            powerConfig.dcvsV3Config.setSleepDisable = 1;
            powerConfig.dcvsV3Config.coreVoltageCornerMin = DCVS_VOLTAGE_VCORNER_SVS;
            powerConfig.dcvsV3Config.coreVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_SVS;
            powerConfig.dcvsV3Config.coreVoltageCornerMax = DCVS_VOLTAGE_VCORNER_SVS;
            break;
        case PowerModeType::LOW_POWER_SAVER:
            powerConfig.dcvsV3Config.setSleepLatency = 1;
            powerConfig.dcvsV3Config.sleepLatency = 1000;
            powerConfig.dcvsV3Config.dcvsEnable = 1;
            powerConfig.dcvsV3Config.busVoltageCornerMin = DCVS_VOLTAGE_VCORNER_SVS2;
            powerConfig.dcvsV3Config.busVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_SVS2;
            powerConfig.dcvsV3Config.busVoltageCornerMax = DCVS_VOLTAGE_VCORNER_SVS2;
            powerConfig.dcvsV3Config.setSleepDisable = 1;
            powerConfig.dcvsV3Config.coreVoltageCornerMin = DCVS_VOLTAGE_VCORNER_SVS2;
            powerConfig.dcvsV3Config.coreVoltageCornerTarget = DCVS_VOLTAGE_VCORNER_SVS2;
            powerConfig.dcvsV3Config.coreVoltageCornerMax = DCVS_VOLTAGE_VCORNER_SVS2;
            break;
        case PowerModeType::EXTREME_POWER_SAVER:
            powerConfig.dcvsV3Config.setSleepLatency = 1;
            powerConfig.dcvsV3Config.sleepLatency = 1000;
            powerConfig.dcvsV3Config.dcvsEnable = 1;
            powerConfig.dcvsV3Config.busVoltageCornerMin = DCVS_VOLTAGE_CORNER_DISABLE;
            powerConfig.dcvsV3Config.busVoltageCornerTarget = DCVS_VOLTAGE_CORNER_DISABLE;
            powerConfig.dcvsV3Config.busVoltageCornerMax = DCVS_VOLTAGE_CORNER_DISABLE;
            powerConfig.dcvsV3Config.setSleepDisable = 1;
            powerConfig.dcvsV3Config.coreVoltageCornerMin = DCVS_VOLTAGE_CORNER_DISABLE;
            powerConfig.dcvsV3Config.coreVoltageCornerTarget = DCVS_VOLTAGE_CORNER_DISABLE;
            powerConfig.dcvsV3Config.coreVoltageCornerMax = DCVS_VOLTAGE_CORNER_DISABLE;
            break;
        default:
            QNN_ERROR("Unsupported power mode");
            return StatusCode::FAILURE;
    }

    // Set power config with different performance parameters
    const QnnHtpPerfInfrastructure_PowerConfig_t *powerConfigs[] = {&powerConfig, NULL};

    Qnn_ErrorHandle_t perfInfraErr = perfInfra.setPowerConfig(m_powerConfigId, powerConfigs);
    if (perfInfraErr != QNN_SUCCESS) {
        QNN_ERROR("setPowerConfig failed");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}




StatusCode QnnPowerManager::setRpcLatencyAndPolling() {
    QnnDevice_Infrastructure_t deviceInfra = nullptr;
    QnnInterface_t qnnInterface;
    Qnn_ErrorHandle_t devErr = qnnInterface.QNN_INTERFACE_VER_NAME.deviceGetInfrastructure(&deviceInfra);
    if (devErr != QNN_SUCCESS) {
        QNN_ERROR("device error");
        return StatusCode::FAILURE;
    }
    QnnHtpDevice_Infrastructure_t *htpInfra = static_cast<QnnHtpDevice_Infrastructure_t *>(deviceInfra);
    QnnHtpDevice_PerfInfrastructure_t perfInfra = htpInfra->perfInfra;

    // set RPC Control Latency
    QnnHtpPerfInfrastructure_PowerConfig_t rpcControlLatency;            // refer QnnHtpPerfInfrastructure.h
    memset(&rpcControlLatency, 0, sizeof(rpcControlLatency));
    rpcControlLatency.option = QNN_HTP_PERF_INFRASTRUCTURE_POWER_CONFIGOPTION_RPC_CONTROL_LATENCY;
    rpcControlLatency.rpcControlLatencyConfig = 100;         // use rpc control latency recommended 100 us, refer hexagon sdk
    const QnnHtpPerfInfrastructure_PowerConfig_t *powerConfigs1[] = {&rpcControlLatency, NULL};

    Qnn_ErrorHandle_t perfInfraErr = perfInfra.setPowerConfig(m_powerConfigId, powerConfigs1);  // set RPC latency config on power config ID created
    if (perfInfraErr != QNN_SUCCESS) {
        QNN_ERROR("setPowerConfig failed");
        return StatusCode::FAILURE;
    }

    // set RPC Polling
    QnnHtpPerfInfrastructure_PowerConfig_t rpcPollingTime;   // refer QnnHtpPerfInfrastructure.h
    memset(&rpcPollingTime, 0, sizeof(rpcPollingTime));
    rpcPollingTime.option = QNN_HTP_PERF_INFRASTRUCTURE_POWER_CONFIGOPTION_RPC_POLLING_TIME;
    rpcPollingTime.rpcPollingTimeConfig = 9999;     // use rpc polling time recommended 0-10000 us
    const QnnHtpPerfInfrastructure_PowerConfig_t* powerConfigs2[] = {&rpcPollingTime, NULL};

    perfInfraErr = perfInfra.setPowerConfig(m_powerConfigId, powerConfigs2); // set RPC polling config on power config ID created
    if (perfInfraErr != QNN_SUCCESS) {
        QNN_ERROR("setPowerConfig failed");
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}
