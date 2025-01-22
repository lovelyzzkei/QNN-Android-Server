# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("imageclassifiers")


# OPENCV
#set(pathPROJECT C:/Users/user/Documents/QNN-Android/QNN-Skeleton)
#set(pathOPENCV ${pathPROJECT}/OpenCV)
set(pathOPENCV ${CMAKE_CURRENT_SOURCE_DIR}/../OpenCV)
set(pathLIBOPENCV_JAVA ${pathOPENCV}/native/libs/${ANDROID_ABI}/libopencv_java4.so)

set(TFLITE_VERSION 2.18.0)

set(TFLITE_LIBPATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/tensorflowlite/${TFLITE_VERSION}/jni")
set(TFLITE_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/libs/tensorflowlite/${TFLITE_VERSION}/headers")
set(BUILD_SUBDIR ${ANDROID_ABI})

find_package(OpenMP)

include_directories(${TFLITE_INCLUDE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/QNN)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/include)
include_directories(${pathOPENCV}/native/jni/include)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
set(NATIVE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/main/cpp")

file(GLOB SOURCES "${NATIVE_SOURCE_DIR}/*.cpp")
file(GLOB PAL "${NATIVE_SOURCE_DIR}/PAL/*.cpp")
file(GLOB Log "${NATIVE_SOURCE_DIR}/Log/*.cpp")
file(GLOB Utils "${NATIVE_SOURCE_DIR}/Utils/*.cpp")
file(GLOB MODELS "${NATIVE_SOURCE_DIR}/Models/*.cpp")


add_library(${CMAKE_PROJECT_NAME}
        SHARED
        ${SOURCES}
        ${PAL}
        ${Log}
        ${Utils}
        ${MODELS}
)

add_library(lib_opencv SHARED IMPORTED)

# OPENCV
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${pathLIBOPENCV_JAVA})

add_library(lib_tensorflowlite_gpu SHARED IMPORTED)
set_target_properties(lib_tensorflowlite_gpu PROPERTIES IMPORTED_LOCATION
        ${TFLITE_LIBPATH}/${CMAKE_ANDROID_ARCH_ABI}/libtensorflowlite_gpu_delegate.so)
add_library(lib_tensorflowlite_cpp SHARED IMPORTED)
set_target_properties(lib_tensorflowlite_cpp PROPERTIES IMPORTED_LOCATION
        ${TFLITE_LIBPATH}/${CMAKE_ANDROID_ARCH_ABI}/libtensorflowlite.so)
add_library(lib_tensorflowlite_qnn SHARED IMPORTED)
set_target_properties(lib_tensorflowlite_qnn PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/arm64-v8a/libQnnTFLiteDelegate.so)


# SNPE
#set(SNPE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/main/")
#add_library(libSNPE SHARED IMPORTED)
#set_target_properties(libSNPE PROPERTIES IMPORTED_LOCATION
#        ${SNPE_SOURCE_DIR}/jniLibs/arm64-v8a/libSNPE.so)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library

        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/arm64-v8a/libQnnSystem.so

        lib_opencv
        lib_tensorflowlite_gpu
        lib_tensorflowlite_cpp
        lib_tensorflowlite_qnn

        android
        -ljnigraphics
        -fopenmp
        -static-openmp
        z
        log)