//
// Created by user on 2024-12-23.
//

#ifndef ARIA_TFLITEMANAGER_HPP
#define ARIA_TFLITEMANAGER_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/interpreter_builder.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/string_util.h"
#include "tensorflow/lite/delegates/gpu/delegate.h"
#include "tensorflow/lite/delegates/xnnpack/xnnpack_delegate.h"

#include "tensorflow/lite/delegates/serialization.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "TFLiteDelegate/QnnTFLiteDelegate.h"

#include "android/log.h"

#define LOG_TAG "CYJUNG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)


class TFLiteManager {
public:
    TFLiteManager();
    TFLiteManager(const char* nativeLibPath);
    ~TFLiteManager();

    /**
     * @brief Build a TFLite interpreter from a model file + delegate
     * @param modelName   (e.g. "dav2_hypersim_406x658_not_norm.tflite")
     * @param delegate    (e.g. "GPU", "QNN", "CPU", ...)
     * @param returnInterpreter [out] 결과 Interpreter
     * @param ret         만약 true면, returnInterpreter에 담아서 반환만 하고
     *                    mInterpreters에는 넣지 않는다.
     *
     * 250110: Profiling을 위해 delegate가 필요하여 ModelInterpreter 구조체를 반환하도록 수정
     */
    std::pair<std::unique_ptr<tflite::Interpreter>, TfLiteDelegate*>
        buildTFLiteNetwork(const char* modelName, const char* delegate);



private:
    bool setAdspLibraryPath(std::string nativeLibPath);
    void getSupportedRuntime(const char* native_dir_path);

    // 내부적으로 사용하는 Delegate 설정 함수들
    TfLiteGpuDelegateOptionsV2 setGpuDelegateOptions();
    TfLiteQnnDelegateOptions setQnnGpuDelegateOptions();
    TfLiteQnnDelegateOptions setQnnDelegateOptions();
};


#endif //ARIA_TFLITEMANAGER_HPP
