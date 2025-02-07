#include <jni.h>
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "QnnManager.hpp"
#include "IModel.hpp"
#include "TaskType.hpp"
#include "ModelFactory.hpp"

#include "Log/Logger.hpp"

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>


using namespace cv;


// Set LD library path for loading libraries for QNN
bool SetAdspLibraryPath(std::string nativeLibPath)
{
    nativeLibPath += ";/vendor/lib/rfsa/adsp;/vendor/dsp/cdsp;/system/lib/rfsa/adsp;/system/vendor/lib/rfsa/adsp;/dsp";

    LOGI("ADSP Lib Path = %s \n",  nativeLibPath.c_str());
    std::cout << "ADSP Lib Path = " << nativeLibPath << std::endl;

    return setenv("ADSP_LIBRARY_PATH", nativeLibPath.c_str(), 1 /*override*/) == 0;
}



extern "C"
JNIEXPORT jstring JNICALL
Java_com_lovelyzzkei_qnnSkeleton_NativeInterface_setAdspLibraryPathJNI(JNIEnv *env,
                                                                       jclass thiz,
                                                         jstring native_dir_path) {
    const char *cstr = env->GetStringUTFChars(native_dir_path, NULL);
    env->ReleaseStringUTFChars(native_dir_path, cstr);

    std::string runT_Status;
    std::string nativeLibPath = std::string(cstr);

    // Set Adsp library path
    if (!SetAdspLibraryPath(nativeLibPath)) {
        LOGI("Failed to set ADSP Library Path for setting DSP Runtime\n");
    }
    return env->NewStringUTF(runT_Status.c_str());
}



bool createDirectory(const std::string& path) {
    size_t pos = 0;
    std::string currentPath;
    bool success = true;

    while ((pos = path.find('/', pos)) != std::string::npos) {
        currentPath = path.substr(0, pos++);
        if (!currentPath.empty() && mkdir(currentPath.c_str(), 0777) && errno != EEXIST) {
            success = false;
            break;
        }
    }
    if (success) {
        success = (mkdir(path.c_str(), 0777) == 0 || errno == EEXIST);
        LOGI("Result folder: %s created", path.c_str());
    }
    return success;
}


// Global pointer to the current model, or store in a manager, etc.
static std::unique_ptr<IModel> gModel;



extern "C"
JNIEXPORT jboolean JNICALL
Java_com_lovelyzzkei_qnnSkeleton_NativeInterface_initializeModelJNI(JNIEnv* env, jclass clazz,
                                                                    jint jTaskType, // passed from Java
                                                                    jstring jDevice, jstring jNativeLibDir,
                                                                    jstring jModelFile, jstring jBackend,
                                                                    jstring jPrecision, jstring jFramework) {
    // TODO: implement initializeModelJNI()
    const char* device   = env->GetStringUTFChars(jDevice, NULL);
    const char* nativeLibDir   = env->GetStringUTFChars(jNativeLibDir, NULL);
    const char* modelFile= env->GetStringUTFChars(jModelFile, NULL);
    const char* backend  = env->GetStringUTFChars(jBackend, NULL);
    const char* precision= env->GetStringUTFChars(jPrecision, NULL);
    const char* framework= env->GetStringUTFChars(jFramework, NULL);

    // Convert Java's jint taskType to our C++ enum
    auto taskType = static_cast<TaskType>(jTaskType);

    // Set Adsp library path
    if (!SetAdspLibraryPath(nativeLibDir)) {
        LOGI("Failed to set ADSP Library Path\n");
    }

    // Create the model
    gModel = createModel(taskType);
    if (!gModel) {
        // Could not create a model
        return JNI_FALSE;
    }

    bool success = gModel->initialize(device, modelFile, backend, precision, framework);

    env->ReleaseStringUTFChars(jDevice, device);
    env->ReleaseStringUTFChars(jModelFile, modelFile);
    env->ReleaseStringUTFChars(jBackend, backend);
    env->ReleaseStringUTFChars(jPrecision, precision);
    env->ReleaseStringUTFChars(jFramework, framework);


    return success ? JNI_TRUE : JNI_FALSE;
}


extern "C"
JNIEXPORT void JNICALL
Java_com_lovelyzzkei_qnnSkeleton_NativeInterface_setPowerModeJNI(JNIEnv *env, jclass clazz,
                                                                 jint powerMode) {
    gModel->setPowerMode(powerMode);
}


extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_lovelyzzkei_qnnSkeleton_NativeInterface_getObjectBoxesJNI(JNIEnv *env, jclass thiz,
                                                    jbyteArray YUVFrameData, jint width,
                                                    jint height) {
    jbyte * pYUVFrameData = env->GetByteArrayElements(YUVFrameData, 0);

    std::vector<Detection>* detectionResults;

    logExecutionTime("Preprocess", [&]() {
        gModel->preprocess((unsigned char*)pYUVFrameData, width, height);
    });
    logExecutionTime("Inference", [&]() {
        gModel->inference();
    });
    logExecutionTime("Postprocess", [&]() {
        detectionResults = (std::vector<Detection>*)gModel->postprocess();
    });

    // Conversion to pass back to Java
    auto start = std::chrono::high_resolution_clock::now();
    jclass detectionClass = env->FindClass("com/lovelyzzkei/qnnSkeleton/tasks/ObjectDetectionManager$YoloDetection");
    jobjectArray detectionArray = env->NewObjectArray(detectionResults->size(), detectionClass, nullptr);
    jmethodID constructor = env->GetMethodID(detectionClass, "<init>", "(FFFFFLjava/lang/String;)V");

    for (size_t i = 0; i < detectionResults->size(); ++i) {
        const auto &det = detectionResults->at(i);
        jstring clsName = env->NewStringUTF(det.cls.c_str());
        jobject detectionObject = env->NewObject(
                detectionClass, constructor,
                det.x1, det.y1, det.x2, det.y2, det.score, clsName
        );
        env->SetObjectArrayElement(detectionArray, i, detectionObject);
        env->DeleteLocalRef(clsName);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    LOGD("jni conversion time: %.3f ms", duration.count());

    return detectionArray;
}




extern "C"
JNIEXPORT jfloatArray JNICALL
Java_com_lovelyzzkei_qnnSkeleton_NativeInterface_getDepthMapJNI(JNIEnv *env, jclass clazz,
                                                          jbyteArray YUVFrameData, jint width,
                                                          jint height) {
    jbyte * pYUVFrameData = env->GetByteArrayElements(YUVFrameData, 0);
    std::vector<float>* depthData;
    logExecutionTime("Preprocess", [&]() {
        gModel->preprocess((unsigned char*)pYUVFrameData, width, height);
    });
    logExecutionTime("Inference", [&]() {
        gModel->inference();
    });
    logExecutionTime("Postprocess", [&]() {
        depthData = (std::vector<float>*)gModel->postprocess();
    });

    jfloatArray depthDataArray = env->NewFloatArray(depthData->size());
    env->SetFloatArrayRegion(depthDataArray, 0, depthData->size(), depthData->data());
    return depthDataArray;
}



