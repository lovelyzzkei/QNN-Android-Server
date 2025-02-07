//
// Created by user on 2025-02-06.
//

#include "QnnBackendManager.hpp"
#include "Log/Logger.hpp"
#include "HTP/QnnHtpDevice.h"

using namespace qnn;
using namespace qnn::tools;

StatusCode QnnBackendManager::setup(QnnLoader& loader) {
    // 1) logging -> Currently turn off
//    auto status = initializeLogging(loader);
//    if (status != StatusCode::SUCCESS) return status;

    // 2) backend
    auto status = initializeBackend(loader);
    if (status != StatusCode::SUCCESS) return status;

    // 3) device
    auto devicePropertySupportStatus = isDevicePropertySupported(loader);
    if (StatusCode::FAILURE != devicePropertySupportStatus) {
        auto createDeviceStatus = this->createDevice(loader);
        if (StatusCode::SUCCESS != createDeviceStatus) {
            QNN_ERROR("Device Creation failure");
        }
    }
    QNN_INFO("Device Created");


    // 4) profiling
    status = initializeProfiling(loader); // or pass an argument
    if (status != StatusCode::SUCCESS) return status;

    return StatusCode::SUCCESS;
}

StatusCode QnnBackendManager::initializeLogging(QnnLoader& loader) {
    //    if (!log::initializeLogging()) {
//        QNN_ERROR("ERROR: Unable to initialize logging!");
//        return StatusCode::FAILURE;
//    }
    // Logging can be set up before a backed is initialized and after a backend shared library has been dynamically loaded.
    if (log::isLogInitialized()) {
        auto logCallback = log::getLogCallback();
        auto logLevel = log::getLogLevel();
        QNN_INFO("Initializing logging in the backend. Callback: [%p], Log Level: [%d]",
                 logCallback,
                 logLevel);
        if (QNN_SUCCESS != loader.getFunctionPointers().qnnInterface.logCreate(logCallback, logLevel, &m_logHandle)) {
            QNN_ERROR("Unable to initialize logging in the backend.");
            return StatusCode::FAILURE;
        }
    } else {
        QNN_ERROR("Logging not available in the backend.");
        return StatusCode::FAILURE;
    }
    QNN_INFO("Logging initialized");
    return StatusCode::SUCCESS;
}


StatusCode QnnBackendManager::initializeBackend(QnnLoader& loader) {
    auto qnnStatus = loader.getFunctionPointers().qnnInterface
            .backendCreate(m_logHandle,
                           (const QnnBackend_Config_t **)m_backendConfig,
                           loader.getBackendHandlePtr());
    if (QNN_BACKEND_NO_ERROR != qnnStatus) {
        QNN_ERROR("Could not initialize backend due to error = {}", (unsigned int)qnnStatus);
        return StatusCode::FAILURE;
    }
    QNN_INFO("Initialize Backend Returned Status = %d", (unsigned int)qnnStatus);
    return StatusCode::SUCCESS;
}


StatusCode QnnBackendManager::initializeProfiling(QnnLoader& loader) {
    auto m_backendHandle = loader.getBackendHandle();
    if (ProfilingLevel::OFF != m_profilingLevel) {
        QNN_INFO("Profiling turned on; level = %d", (int)m_profilingLevel);
        if (ProfilingLevel::BASIC == m_profilingLevel) {
            QNN_INFO("Basic profiling requested. Creating Qnn Profile object.");
            if (QNN_PROFILE_NO_ERROR != loader.getFunctionPointers().qnnInterface.profileCreate(
                    m_backendHandle,
                    QNN_PROFILE_EVENTTYPE_EXECUTE,
                    &m_profileBackendHandle)) {
                QNN_WARN("Unable to create profile handle in the backend.");
                return StatusCode::FAILURE;
            }
        } else if (ProfilingLevel::DETAILED == m_profilingLevel) {
            QNN_INFO("Detailed profiling requested. Creating Qnn Profile object.");
            if (QNN_PROFILE_NO_ERROR != loader.getFunctionPointers().qnnInterface.profileCreate(
                    m_backendHandle,
                    QNN_PROFILE_LEVEL_DETAILED,
                    &m_profileBackendHandle)) {
                QNN_ERROR("Unable to create profile handle in the backend.");
                return StatusCode::FAILURE;
            }
        }
    }
    return StatusCode::SUCCESS;
}


StatusCode QnnBackendManager::isDevicePropertySupported(QnnLoader& loader) {
    if (nullptr != loader.getFunctionPointers().qnnInterface.propertyHasCapability) {
        auto qnnStatus =
                loader.getFunctionPointers().qnnInterface.propertyHasCapability(QNN_PROPERTY_GROUP_DEVICE);
        if (QNN_PROPERTY_NOT_SUPPORTED == qnnStatus) {
            QNN_WARN("Device property is not supported");
        }
        if (QNN_PROPERTY_ERROR_UNKNOWN_KEY == qnnStatus) {
            QNN_ERROR("Device property is not known to backend");
            return StatusCode::FAILURE;
        }
    }
    return StatusCode::SUCCESS;
}


StatusCode QnnBackendManager::createDevice(QnnLoader& loader) {
    // TODO: Adapt to the user's device
    QnnHtpDevice_CustomConfig_t customConfig;
    customConfig.option   = QNN_HTP_DEVICE_CONFIG_OPTION_SOC;
    customConfig.unnamedUnion.socModel = QNN_SOC_MODEL_SM8550;
    QnnDevice_Config_t devConfig;
    devConfig.option = QNN_DEVICE_CONFIG_OPTION_CUSTOM;
    devConfig.customConfig = &customConfig;

    const QnnDevice_Config_t* devConfigArray[] = {&devConfig, nullptr};

    if (nullptr != loader.getFunctionPointers().qnnInterface.deviceCreate) {
        auto qnnStatus =
                loader.getFunctionPointers().qnnInterface.deviceCreate(
                        m_logHandle,
                        devConfigArray,
                        &m_deviceHandle);
        QNN_INFO("Device Creation Status: %d", qnnStatus);
        if (QNN_SUCCESS != qnnStatus && QNN_DEVICE_ERROR_UNSUPPORTED_FEATURE != qnnStatus) {
            QNN_ERROR("Failed to create device %u", qnnStatus);
            return StatusCode::FAILURE;
        }
    }
    return StatusCode::SUCCESS;
}


StatusCode QnnBackendManager::extractBackendProfilingInfo (QnnLoader& loader) {
    if (nullptr == m_profileBackendHandle) {
        QNN_ERROR("Backend Profile handle is nullptr; may not be initialized.");
        return StatusCode::FAILURE;
    }
    const QnnProfile_EventId_t* profileEvents{nullptr};
    uint32_t numEvents{0};
    if (QNN_PROFILE_NO_ERROR != loader.getFunctionPointers().qnnInterface.profileGetEvents(
            m_profileBackendHandle, &profileEvents, &numEvents)) {
        QNN_ERROR("Failure in profile get events.");
        return StatusCode::FAILURE;
    }
    QNN_DEBUG("ProfileEvents: [%p], numEvents: [%d]", profileEvents, numEvents);
    for (size_t event = 0; event < numEvents; event++) {
        extractProfilingEvent(loader, *(profileEvents + event));
        extractProfilingSubEvents(loader, *(profileEvents + event));
    }
    return StatusCode::SUCCESS;
}



StatusCode QnnBackendManager::extractProfilingSubEvents(QnnLoader& loader, QnnProfile_EventId_t profileEventId) {
    const QnnProfile_EventId_t* profileSubEvents{nullptr};
    uint32_t numSubEvents{0};
    if (QNN_PROFILE_NO_ERROR != loader.getFunctionPointers().qnnInterface.profileGetSubEvents(
            profileEventId, &profileSubEvents, &numSubEvents)) {
        QNN_ERROR("Failure in profile get sub events.");
        return StatusCode::FAILURE;
    }
    QNN_DEBUG("ProfileSubEvents: [%p], numSubEvents: [%d]", profileSubEvents, numSubEvents);
    for (size_t subEvent = 0; subEvent < numSubEvents; subEvent++) {
        extractProfilingEvent(loader, *(profileSubEvents + subEvent));
        extractProfilingSubEvents(loader, *(profileSubEvents + subEvent));
    }
    return StatusCode::SUCCESS;
}


StatusCode QnnBackendManager::extractProfilingEvent(QnnLoader& loader, QnnProfile_EventId_t profileEventId) {
    QnnProfile_EventData_t eventData;
    if (QNN_PROFILE_NO_ERROR !=
            loader.getFunctionPointers().qnnInterface.profileGetEventData(profileEventId, &eventData)) {
        QNN_ERROR("Failure in profile get event type.");
        return StatusCode::FAILURE;
    }
    QNN_DEBUG("Printing Event Info - Event Type: [%d], Event Value: [%" "llu"
              "], Event Identifier: [%s], Event Unit: [%d]",
              eventData.type,
              eventData.value,
              eventData.identifier,
              eventData.unit);
    return StatusCode::SUCCESS;
}




std::string QnnBackendManager::getBackendBuildId(QnnLoader& loader) {
    char *backendBuildId{nullptr};
    if (QNN_SUCCESS != loader.getFunctionPointers()
        .qnnInterface.backendGetBuildId((const char **)&backendBuildId)) {
        QNN_ERROR("Unable to get build Id from the backend.");
    }
    return (backendBuildId == nullptr ? std::string("") : std::string(backendBuildId));
}
