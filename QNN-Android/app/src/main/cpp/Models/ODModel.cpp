////
//// Created by user on 2025-02-03.
////
//
//#include "ODModel.hpp"
//#include "Log/Logger.hpp"
//#include "../QnnTypeMacros.hpp"
//#include "../QnnManager.hpp"
//
//ODModel::ODModel(const char* modelPath,
//                 const char* backend,
//                 const char* precision)
//: ModelBase(modelPath, backend) {
//    std::string model = std::string(modelPath) + "_" + std::string(precision) + ".so";
//    qnnManager = new QnnManager(model.c_str(), backend);
//
//    auto dim = getQnnTensorDimensions((*qnnManager->getGraphsInfo())[0].inputTensors);
//    width = dim[2];
//    height = dim[1];
//    LOGD("width: %d, height: %d", width, height);
//}
//
//
//auto logExecutionTime = [](const std::string &stage, auto &&func) {
//    auto start = std::chrono::high_resolution_clock::now();
//    func();
//    auto end = std::chrono::high_resolution_clock::now();
//    auto duration = std::chrono::duration<double, std::milli>(end - start);
//    LOGD("%s time: %.3f ms", stage.c_str(), duration.count());
//};
//
//
////void ODModel::initializeModel() {}(const char* device) {
////    std::unique_ptr<tflite::Interpreter> interp;
////
////    std::string npuModelName = "yolov6_480x640.tflite";
////    mODModel = tfLiteManager->buildTFLiteNetwork(npuModelName.c_str(), "QNN").first;
////
////    width = mODModel->input_tensor(0)->dims->data[2];
////    height = mODModel->input_tensor(0)->dims->data[1];
////    numObjects = mODModel->output_tensor(2)->dims->data[1];
////
////    LOGD("width: %d, height: %d, numClasses: %d", width, height, numObjects);
////    LOGD("Object detection model is prepared");
////}
//
//
//void ODModel::preprocess(cv::Mat &mrgb) {
//    cv::resize(mrgb, mrgb, cv::Size(width, height), cv::INTER_CUBIC);
//    cv::cvtColor(mrgb, mrgb, CV_BGR2RGB);
//    cv::cvtColor(mrgb, mrgb, CV_BGR2RGB);
//    mrgb.convertTo(mrgb, CV_32FC3, 1.0/255.0);
//
//    // Normalize the image -> mean and std are fit to NYU dataset. Need to change to use camera
//    cv::Scalar mean(NORM_MEAN[0], NORM_MEAN[1], NORM_MEAN[2]);
//    cv::Scalar std(NORM_STD[0], NORM_STD[1], NORM_STD[2]);
//
//    cv::subtract(mrgb, mean, mrgb);
//    cv::divide(mrgb, std, mrgb);
//}
//
//
//
//std::vector<Detection> ODModel::doODInference(cv::Mat &mrgb) {
//    std::vector<Detection> dets;
//
//    // Preprocess
//    logExecutionTime("Preprocess", [&]() {
//        preprocess(mrgb);
//    });
//
////    auto start = std::chrono::high_resolution_clock::now();
////    preprocessImage(mrgb);
////    auto end = std::chrono::high_resolution_clock::now();
////    auto duration = std::chrono::duration<double, std::milli>(end - start);
////    LOGD("Preprocesss time: %.3f ms", duration.count());
//
//    if (strcmp(framework, "TFLite") == 0) {
//        // TFLite version of executing model
//        logExecutionTime("Inference", [&]() {
//            const TfLiteTensor *inputTensor = mODModel->input_tensor(0);
//            memcpy(&inputTensor->data.raw[0], reinterpret_cast<const char *>(mrgb.data),
//                   mrgb.total() * mrgb.elemSize());
//            mrgb.release();
//            mODModel->Invoke();
//        });
//
//        logExecutionTime("Postprocess", [&]() {
//            dets = postprocessResults();
//        });
//
//    }
//    else if (strcmp(framework, "QNN") == 0) {
//        // QNN version of executing model
//        logExecutionTime("Inference", [&]() {
//            qnnManager->inferenceModel(reinterpret_cast<float32_t *>(mrgb.data));
//        });
//
//        logExecutionTime("Postprocess", [&]() {
//            dets = postprocess(qnnManager->m_inferData);
//        });
//    }
//
//    return dets;
//}
//
//
//std::vector<Detection> ODModel::postprocess(
//        std::vector<std::pair<std::vector<size_t>, float32_t*>> dets) {
//
//    // Dim, Data pair -> Vector
//    const float* boxesPtr = dets[0].second;     // (1, 6300, 4)
//    const float* scoresPtr = dets[1].second;
//    const float* classIdxPtr = dets[2].second;
//
//    // (1, 6300, 4) -> std::vector<std::vector<float>> boxes
//    int numBoxes = dets[0].first[1];
//    int boxElements = dets[0].first[2]; // 4 (x1, y1, x2, y2)
//
//
//    std::vector<std::vector<float>> boxes;
//    {
//        std::vector<float> flatBoxes(boxesPtr, boxesPtr + numBoxes * boxElements);
//        for (int i = 0; i < numBoxes; ++i) {
//            std::vector<float> box(flatBoxes.begin() + i * boxElements,
//                                   flatBoxes.begin() + (i + 1) * boxElements);
//            boxes.push_back(box);
//        }
//
////        for (int i = 0; i < 10; i++) {
////            LOGD("QNN output[%d]: %f", i, flatBoxes[i]);
////        }
//    }
//
//    std::vector<float> scores(scoresPtr, scoresPtr + numBoxes);
//    std::vector<float> classIdx(classIdxPtr, classIdxPtr + numBoxes);
//
//    // NMS
//    float scoreThreshold = 0.5;
//    float iouThreshold = 0.4;
//    auto finalDetections = filterHighConfBoxes(boxes, scores, classIdx, scoreThreshold, iouThreshold);
//
//    return finalDetections;
//}
//
//
//
//
//
//std::vector<Detection> ODManager::postprocessResults() {
//    const TfLiteTensor *boxesTensor = mODModel->output_tensor(0);
//    const TfLiteTensor *scoresTensor = mODModel->output_tensor(1);
//    const TfLiteTensor *classIdxTensor = mODModel->output_tensor(2);
//
//    // Output 0: Box (1, N_CLS, 4)
//    std::vector<std::vector<float>> boxes;
//    {
//        std::vector<float> flatBoxes = tensorToVector<float>(boxesTensor);
//
//        // (1, 6300, 4) -> std::vector<std::vector<float>> boxes
//        int numBoxes = boxesTensor->dims->data[1];
//        int boxElements = boxesTensor->dims->data[2]; // 4 (x1, y1, x2, y2)
//        for (int i = 0; i < numBoxes; ++i) {
//            std::vector<float> box(flatBoxes.begin() + i * boxElements,
//                                   flatBoxes.begin() + (i + 1) * boxElements);
//            boxes.push_back(box);
//        }
//
////        for (int i = 0; i < 10; i++) {
////            LOGD("output[%d]: %f", i, flatBoxes[i]);
////        }
//    }
//
//    // Output 1: Score (1, N_CLS)
//    std::vector<float> scores = tensorToVector<float>(scoresTensor);
//
//    // Output 2: Class Idx (1, N_CLS)
//    std::vector<float> classIdx = tensorToVector<float>(classIdxTensor);
//
//    // NMS
//    float scoreThreshold = 0.5;
//    float iouThreshold = 0.4;
//    auto finalDetections = filterHighConfBoxes(boxes, scores, classIdx, scoreThreshold, iouThreshold);
//
//    return finalDetections;
//}
//
//
//// YOLO Post-processing
//std::vector<Detection> ODManager::filterHighConfBoxes(
//        const std::vector<std::vector<float>>& boxes,       // (1, N_OBJS, 4)
//        const std::vector<float>& scores,                   // (1, N_OBJS)
//        const std::vector<float>& class_idx,                // (1, N_OBJS)
//        float score_threshold, float iou_threshold) {
//
//    std::vector<Detection> detections;
//    for (size_t i = 0; i < scores.size(); ++i) {
//        if (scores[i] >= score_threshold) {
//            Detection det;
//            det.x1 = boxes[i][0];
//            det.y1 = boxes[i][1];
//            det.x2 = boxes[i][2];
//            det.y2 = boxes[i][3];
//            det.cls = classNames.at((int)class_idx[i]);
//            det.score = scores[i];
//            detections.push_back(det);
//        }
//    }
//
//    // Non-Maximum Suppression
//    return nonMaximumSuppression(detections, iou_threshold);
//}
//
//
//// Intersection over Union (IoU)
//float computeIoU(const Detection& box1, const Detection& box2) {
//    float x1 = std::max(box1.x1, box2.x1);
//    float y1 = std::max(box1.y1, box2.y1);
//    float x2 = std::min(box1.x2, box2.x2);
//    float y2 = std::min(box1.y2, box2.y2);
//
//    float intersection = std::max(0.0f, x2 - x1) * std::max(0.0f, y2 - y1);
//    float area1 = (box1.x2 - box1.x1) * (box1.y2 - box1.y1);
//    float area2 = (box2.x2 - box2.x1) * (box2.y2 - box2.y1);
//
//    return intersection / (area1 + area2 - intersection);
//}
//
//
//// Non-Maximum Suppression (NMS)
//std::vector<Detection> ODManager::nonMaximumSuppression(const std::vector<Detection>& detections, float iou_threshold) {
//    std::vector<Detection> result;
//    std::vector<bool> suppressed(detections.size(), false);
//
//    // Descending sort by score
//    std::vector<int> indices(detections.size());
//    std::iota(indices.begin(), indices.end(), 0);
//    std::sort(indices.begin(), indices.end(), [&](int i, int j) {
//        return detections[i].score > detections[j].score;
//    });
//
//    for (size_t i = 0; i < indices.size(); ++i) {
//        if (suppressed[indices[i]]) continue;
//
//        result.push_back(detections[indices[i]]);
//        for (size_t j = i + 1; j < indices.size(); ++j) {
//            if (computeIoU(detections[indices[i]], detections[indices[j]]) > iou_threshold) {
//                suppressed[indices[j]] = true;
//            }
//        }
//    }
//    return result;
//}
//
//
//void ODManager::loadClassNames() {
//    const std::string& filePath = "/data/local/tmp/qnnSkeleton/coco_labels.txt";
//    std::ifstream file(filePath);
//
//    if (!file.is_open()) {
//        throw std::runtime_error("Failed to open class names file: " + filePath);
//    }
//
//    int idx = 0;
//    std::string line;
//    while (std::getline(file, line)) {
//        classNames.push_back(line);
//        idx++;
//    }
//    file.close();
//
//    LOGD("Number of classes: %d", idx);
//}
//
//
//// Convert TfLiteTensor data to std::vector
//template <typename T>
//std::vector<T> ODManager::tensorToVector(const TfLiteTensor* tensor) {
//    if (!tensor) {
//        throw std::invalid_argument("Tensor is null");
//    }
//
//    const T* tensorData = reinterpret_cast<const T*>(tensor->data.raw);
//    size_t tensorSize = 1;
//
//    for (int i = 0; i < tensor->dims->size; ++i) {
//        tensorSize *= tensor->dims->data[i];
//    }
//
//    return std::vector<T>(tensorData, tensorData + tensorSize);
//}