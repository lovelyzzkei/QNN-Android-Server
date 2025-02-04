#include <jni.h>
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "Models/ODManager.hpp"
#include "QnnManager.hpp"

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
Java_com_lovelyzzkei_qnnSkeleton_ARActivity_setAdspLibraryPathJNI(JNIEnv *env,
                                                         jobject thiz,
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


ODManager* gODManager = nullptr;


extern "C"
JNIEXPORT void JNICALL
Java_com_lovelyzzkei_qnnSkeleton_ARActivity_initializeODManagerJNI(JNIEnv *env, jobject thiz,
                                                         jstring jDevice, jstring jNativeLibPath,
                                                         jstring jModel, jstring jBackend,
                                                         jstring jPrecision, jstring jFramework) {
    const char* device = env->GetStringUTFChars(jDevice, NULL);
    const char* model = env->GetStringUTFChars(jModel, NULL);
    const char* backend = env->GetStringUTFChars(jBackend, NULL);
    const char* precision = env->GetStringUTFChars(jPrecision, NULL);
    const char* framework = env->GetStringUTFChars(jFramework, NULL);
    const char* nativeLibPath = env->GetStringUTFChars(jNativeLibPath, NULL);

    // Set Adsp library path
    if (!SetAdspLibraryPath(nativeLibPath)) {
        LOGI("Failed to set ADSP Library Path\n");
    }
    gODManager = new ODManager(device, model, backend, precision, framework);
}



extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_lovelyzzkei_qnnSkeleton_ARActivity_getObjectBoxesJNI(JNIEnv *env, jobject thiz,
                                                    jbyteArray YUVFrameData, jint width,
                                                    jint height) {
    jbyte * pYUVFrameData = env->GetByteArrayElements(YUVFrameData, 0);

    // Convert to OpenCV Mat, YUV -> RGB (Originally YUV_420_888)
    cv::Mat myuv(height + height / 2, width, CV_8UC1, (unsigned char*)pYUVFrameData);
    cv::Mat mrgb(height, width, CV_8UC3);
    LOGD("mrgb size: %d x %d", mrgb.cols, mrgb.rows);
    cv::cvtColor(myuv, mrgb, cv::COLOR_YUV2RGB_NV21, 3);


    std::vector<Detection> detectionResults = gODManager->doODInference(mrgb);

    auto start = std::chrono::high_resolution_clock::now();
    jclass detectionClass = env->FindClass("com/lovelyzzkei/qnnSkeleton/ARActivity$YoloDetection");
    jobjectArray detectionArray = env->NewObjectArray(detectionResults.size(), detectionClass, nullptr);
    jmethodID constructor = env->GetMethodID(detectionClass, "<init>", "(FFFFFLjava/lang/String;)V");

    for (size_t i = 0; i < detectionResults.size(); ++i) {
        const auto &det = detectionResults[i];
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
