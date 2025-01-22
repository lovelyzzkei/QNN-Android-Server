//
// Created by user on 2024-12-23.
//
#include "TFLiteManager.hpp"

TFLiteManager::TFLiteManager() {
    LOGD("[TFLiteManager] Initialized");
}

TFLiteManager::TFLiteManager(const char* nativeLibPath) {
    getSupportedRuntime(nativeLibPath);
    LOGD("[TFLiteManager] Initialized");
}

TFLiteManager::~TFLiteManager() {
    LOGD("[TFLiteManager] Destructed");
}


bool TFLiteManager::setAdspLibraryPath(std::string nativeLibPath) {
    nativeLibPath += ";/vendor/lib/rfsa/adsp;/vendor/dsp/cdsp;/system/lib/rfsa/adsp;/system/vendor/lib/rfsa/adsp;/dsp";

    LOGI("ADSP Lib Path = %s \n",  nativeLibPath.c_str());

    return setenv("ADSP_LIBRARY_PATH", nativeLibPath.c_str(), 1 /*override*/) == 0;
}

void TFLiteManager::getSupportedRuntime(const char* cstr) {
    std::string runT_Status;
    std::string nativeLibPath = std::string(cstr);

    // Set Adsp library path
    if (!setAdspLibraryPath(nativeLibPath)) {
        LOGI("Failed to set ADSP Library Path for setting DSP Runtime\n");
    }
}


// ------------------------ Delegate option functions ------------------------
TfLiteGpuDelegateOptionsV2 TFLiteManager::setGpuDelegateOptions() {
    TfLiteGpuDelegateOptionsV2 gpuDelegateOptionsV2 = TfLiteGpuDelegateOptionsV2Default();
    gpuDelegateOptionsV2.inference_preference = TFLITE_GPU_INFERENCE_PREFERENCE_FAST_SINGLE_ANSWER;
    gpuDelegateOptionsV2.inference_priority1 = TFLITE_GPU_INFERENCE_PRIORITY_MIN_LATENCY;
    gpuDelegateOptionsV2.inference_priority2 = TFLITE_GPU_INFERENCE_PRIORITY_MAX_PRECISION;
    gpuDelegateOptionsV2.inference_priority3 = TFLITE_GPU_INFERENCE_PRIORITY_MIN_MEMORY_USAGE;

    gpuDelegateOptionsV2.experimental_flags |= TFLITE_GPU_EXPERIMENTAL_FLAGS_ENABLE_SERIALIZATION;
    return gpuDelegateOptionsV2;
}

TfLiteQnnDelegateOptions TFLiteManager::setQnnGpuDelegateOptions() {
    TfLiteQnnDelegateOptions qnnGpuDelegateOptions = TfLiteQnnDelegateOptionsDefault();
    qnnGpuDelegateOptions.backend_type = kGpuBackend;
    qnnGpuDelegateOptions.gpu_options = TfLiteQnnDelegateGpuBackendOptions();
    qnnGpuDelegateOptions.gpu_options.performance_mode = kGpuHigh;
    qnnGpuDelegateOptions.gpu_options.precision = kGpuFp16;
    qnnGpuDelegateOptions.log_level = kLogLevelWarn;
    return qnnGpuDelegateOptions;
}

TfLiteQnnDelegateOptions TFLiteManager::setQnnDelegateOptions() {
    // Create QNN Delegate options structure.
    TfLiteQnnDelegateOptions qnnDelegateOptions = TfLiteQnnDelegateOptionsDefault();

    // Set the mandatory backend_type option. All other options have default values.
    // Set options same as ai hub
    qnnDelegateOptions.backend_type = kHtpBackend;
    qnnDelegateOptions.log_level = kLogLevelWarn;
    qnnDelegateOptions.htp_options = TfLiteQnnDelegateHtpBackendOptions();
    qnnDelegateOptions.htp_options.precision = kHtpFp16;
    qnnDelegateOptions.htp_options.performance_mode = kHtpBurst;
    qnnDelegateOptions.htp_options.useConvHmx = true;

    qnnDelegateOptions.profiling = kPerOpProfiling; // Profiling
    return qnnDelegateOptions;
}



// ------------------------ TFLite Interpreter build functions ------------------------
std::pair<std::unique_ptr<tflite::Interpreter>, TfLiteDelegate*>
    TFLiteManager::buildTFLiteNetwork(const char* modelName, const char* delegate)
{
    tflite::ops::builtin::BuiltinOpResolver resolver;
    std::unique_ptr<tflite::Interpreter> mTFLiteInterpreter;
    TfLiteDelegate* selectedDelegate = nullptr;

    char modelPath[300];
    std::sprintf(modelPath, "/data/local/tmp/qnnSkeleton/models/%s", modelName);

    LOGI("===================== CREATING INTERPRETER =====================\n");
    LOGI("[MODEL NAME  ] : %s \n", modelPath);
    LOGI("[DELEGATE    ] : %s \n", delegate);

    std::unique_ptr<tflite::FlatBufferModel> mTFLiteModel =
            tflite::FlatBufferModel::BuildFromFile(modelPath);

    if (!mTFLiteModel) {
        LOGE("Unable to load TFLite Model in %s", modelPath);
//        return static_cast<ModelInterpreter &&>(nullptr_t);
    }
    if (tflite::InterpreterBuilder(*mTFLiteModel, resolver)(&mTFLiteInterpreter) != kTfLiteOk) {
        LOGE("Unable to create TFLite Interpreter");
//        return nullptr_t;
    }

    // Delegate 적용
    if (strcmp(delegate, "GPU") == 0) {
        TfLiteGpuDelegateOptionsV2 gpuDelegateOptions = setGpuDelegateOptions();
        selectedDelegate = TfLiteGpuDelegateV2Create(&gpuDelegateOptions);
        if (mTFLiteInterpreter->ModifyGraphWithDelegate(selectedDelegate) != kTfLiteOk)
            LOGE("Unable to modify graph with GPU delegate");
        else
            LOGD("Loaded TFLite interpreter with GPU delegate");

        mTFLiteInterpreter->SetNumThreads(4);
        mTFLiteInterpreter->SetAllowFp16PrecisionForFp32(false);
        if (mTFLiteInterpreter->AllocateTensors() != kTfLiteOk) {
            LOGE("Failed to allocate tensors");
        } else {
            LOGI("Allocating tensors success");
        }
    }
    else if (strcmp(delegate, "QNN-GPU") == 0) {
        TfLiteQnnDelegateOptions qnnGpuDelegateOptions = setQnnGpuDelegateOptions();
        selectedDelegate = TfLiteQnnDelegateCreate(&qnnGpuDelegateOptions);
        mTFLiteInterpreter->SetNumThreads(4);
        mTFLiteInterpreter->ModifyGraphWithDelegate(selectedDelegate);
        if (mTFLiteInterpreter->AllocateTensors() != kTfLiteOk) {
            LOGE("Failed to allocate tensors (QNN-GPU)");
        } else {
            LOGI("Allocating tensors success (QNN-GPU)");
        }
    }
    else if (strcmp(delegate, "QNN") == 0) {
        TfLiteQnnDelegateOptions qnnDelegateOptions = setQnnDelegateOptions();
        selectedDelegate = TfLiteQnnDelegateCreate(&qnnDelegateOptions);
        mTFLiteInterpreter->SetNumThreads(4);
        mTFLiteInterpreter->ModifyGraphWithDelegate(selectedDelegate);
        if (mTFLiteInterpreter->AllocateTensors() != kTfLiteOk) {
            LOGE("Failed to allocate tensors (QNN)");
        } else {
            LOGI("Allocating tensors success (QNN)");
        }
    }
    else if (strcmp(delegate, "CPU") == 0) {
        TfLiteXNNPackDelegateOptions xnnpackOptions = TfLiteXNNPackDelegateOptionsDefault();
        xnnpackOptions.num_threads = 4;
        selectedDelegate = TfLiteXNNPackDelegateCreate(&xnnpackOptions);

        if (mTFLiteInterpreter->ModifyGraphWithDelegate(selectedDelegate) != kTfLiteOk) {
            LOGE("Failed to modify graph with XNNPACK delegate");
            TfLiteXNNPackDelegateDelete(selectedDelegate);
        } else {
            LOGI("XNNPACK delegate successfully applied");
        }
        if (mTFLiteInterpreter->AllocateTensors() != kTfLiteOk) {
            LOGE("Failed to allocate tensors with XNNPACK");
        } else {
            LOGI("Allocating tensors with XNNPACK success");
        }
    }

    // Output tensor information
    for (int oIdx = 0; oIdx < (int)mTFLiteInterpreter->outputs().size(); ++oIdx) {
        LOGD(" - Output: %s", mTFLiteInterpreter->output_tensor(oIdx)->name);
    }
    LOGI("TFLite model build success!");
    return std::make_pair(std::move(mTFLiteInterpreter), selectedDelegate);
}

