#============================================================================
# Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#============================================================================

#RESOLVING DEPENDENCIES

RED='\033[1;31m'
GREEN='\033[1;32m'  
NC='\033[0m'        
ANDROID_PATH="QNN-Android"        

echo -e "${GREEN}[*] Download OpenCV library...${NC}"

wget https://sourceforge.net/projects/opencvlibrary/files/4.5.5/opencv-4.5.5-android-sdk.zip/download
unzip download
rm download
mv OpenCV-android-sdk/sdk QNN-Android/
rm -r OpenCV-android-sdk

echo -e "${GREEN}[✔] OpenCV download and folder setup completed! Check the 'QNN-Android' folder.${NC}"


mkdir -p QNN-Android/app/src/main/jniLibs/arm64-v8a
mkdir -p app/src/main/assets


if [ -z "$QNN_SDK_ROOT" ]; then
  echo -e "${RED}QNN_SDK_ROOT not set.${NC}"
  exit 1
else
  echo -e "${GREEN}QNN_SDK_ROOT Root = ${QNN_SDK_ROOT}${NC}"
fi


# Copying libraries
echo -n "[*] Copying hexagon-v73 libraries to ${ANDROID_PATH}/app/src/main/jniLibs/arm64-v8a/... "
if cp "$QNN_SDK_ROOT/lib/hexagon-v73/unsigned/"*.so "${ANDROID_PATH}/app/src/main/jniLibs/arm64-v8a/"; then
  echo -e "${GREEN}Complete${NC}"
else
  echo -e "${RED}Failed${NC}"
  exit 1
fi

echo -n "[*] Copying hexagon-v75 libraries to ${ANDROID_PATH}/app/src/main/jniLibs/arm64-v8a/... "
if cp "$QNN_SDK_ROOT/lib/hexagon-v75/unsigned/"*.so "${ANDROID_PATH}/app/src/main/jniLibs/arm64-v8a/"; then
  echo -e "${GREEN}Complete${NC}"
else
  echo -e "${RED}Failed${NC}"
  exit 1
fi

echo -n "[*] Copying aarch64-android libraries to ${ANDROID_PATH}/app/src/main/jniLibs/arm64-v8a/... "
if cp "$QNN_SDK_ROOT/lib/aarch64-android/"*.so "${ANDROID_PATH}/app/src/main/jniLibs/arm64-v8a/"; then
  echo -e "${GREEN}Complete${NC}"
else
  echo -e "${RED}Failed${NC}"
  exit 1
fi

echo -e "${GREEN}[*] All dependencies are resolved!${NC}"