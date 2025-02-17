# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

project("QnnAndroid")

set(CMAKE_CXX_STANDARD 17)
set(BUILD_SUBDIR ${ANDROID_ABI})

find_package(OpenMP)
find_package(OpenCV REQUIRED)

include_directories(${TFLITE_INCLUDE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/QNN)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/Models)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/include)
include_directories(${pathOPENCV}/native/jni/include)


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

#add_library(lib_opencv SHARED IMPORTED)
#set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${pathLIBOPENCV_JAVA})


# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library

        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/arm64-v8a/libQnnSystem.so
        ${OpenCV_LIBS}
        android
        -ljnigraphics
        -fopenmp
        -static-openmp
        z
        log)