//
// Created by user on 2025-02-17.
//

#include <fstream>
#include <sstream>
#include "ImageClassificationModel.hpp"


ImageClassificationModel::ImageClassificationModel() {
    loadClassNames();
    LOGD("[ImageClassificationModel] Load ImageNet class names");
}


void* ImageClassificationModel::postprocess() {
    auto results = qnnManager->m_inferData;

    auto dims = results[0].first;
    auto fPtr = results[0].second;

    // 1) Calculate the total number of elements from dims
    size_t numElements = 1;
    for (auto d: dims) {
        numElements *= d;
    }

    // 2) Construct a std::vector<float> by copying from the float32_t* buffer
//    std::vector<float> ret(fPtr, fPtr + numElements);
    std::vector<float>* ret = new std::vector<float>(fPtr, fPtr + numElements);
    return static_cast<void*>(ret);
}



void ImageClassificationModel::loadClassNames() {
    const std::string& filePath = "/data/local/tmp/qnnSkeleton/imagenet_labels.txt";
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open class names file: " + filePath);
    }

    int idx = 0;
    std::string line;
    while (std::getline(file, line)) {
        classNames.push_back(line);
        idx++;
    }
    file.close();

    LOGD("Number of classes: %d", idx);
}
