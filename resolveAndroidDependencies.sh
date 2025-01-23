#============================================================================
# Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#============================================================================

#RESOLVING DEPENDENCIES

RED='\033[1;31m'
GREEN='\033[1;32m'  
NC='\033[0m'        

echo -e "${GREEN}[*] Download OpenCV library...${NC}"

# wget https://sourceforge.net/projects/opencvlibrary/files/4.5.5/opencv-4.5.5-android-sdk.zip/download
# unzip download
# rm download
# mv OpenCV-android-sdk/sdk OpenCV
# mv OpenCV QNN-Android/
# rm -r OpenCV-android-sdk

echo -e "${GREEN}[✔] OpenCV download and folder setup completed! Check the 'QNN-Android' folder.${NC}"


# mkdir -p QNN-Android/app/src/main/jniLibs/arm64-v8a
# mkdir -p app/src/main/assets


if [ -z "$QNN_SDK_ROOT" ]; then
  echo -e "${RED}QNN_SDK_ROOT not set.${NC}"
  exit 1
else
  echo -e "${GREEN}QNN_SDK_ROOT Root = ${QNN_SDK_ROOT}${NC}"
fi


# cp -R $QNN_SDK_ROOT/examples/QNN/SampleApp/src/Log   app/src/main/cpp/
# cp -R $QNN_SDK_ROOT/examples/QNN/SampleApp/src/Utils app/src/main/cpp/
# cp -R $QNN_SDK_ROOT/examples/QNN/SampleApp/src/PAL   app/src/main/cpp/
# cp -R $QNN_SDK_ROOT/include/QNN                      app/src/main/cpp/
# cp $QNN_SDK_ROOT/examples/QNN/SampleApp/src/QnnTypeMacros.hpp  app/src/main/cpp/include/
# cp $QNN_SDK_ROOT/examples/QNN/SampleApp/src/WrapperUtils/QnnWrapperUtils.hpp  app/src/main/cpp/include/
# mv app/src/main/cpp/QNN/*.h app/src/main/cpp/include/
# cp $QNN_SDK_ROOT/examples/QNN/SampleApp/src/WrapperUtils/QnnWrapperUtils.cpp  app/src/main/cpp/src/

# ##writing jniLibs
# cp $QNN_SDK_ROOT/lib/hexagon-v73/unsigned/*.so app/src/main/jniLibs/arm64-v8a/
# cp $QNN_SDK_ROOT/lib/hexagon-v75/unsigned/*.so app/src/main/jniLibs/arm64-v8a/
# cp $QNN_SDK_ROOT/lib/aarch64-android/*.so app/src/main/jniLibs/arm64-v8a/
