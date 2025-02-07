package com.lovelyzzkei.qnnSkeleton.tasks;

import com.lovelyzzkei.qnnSkeleton.NativeInterface;
import com.lovelyzzkei.qnnSkeleton.common.LogUtils;
import com.lovelyzzkei.qnnSkeleton.tasks.base.BaseManager;
import com.lovelyzzkei.qnnSkeleton.tasks.base.InferenceResult;
import com.lovelyzzkei.qnnSkeleton.tasks.base.TaskType;

import java.lang.annotation.Native;

public class ObjectDetectionManager implements BaseManager {
    private final TaskType taskType = TaskType.OBJECT_DETECTION;
    public static class YoloDetection {
        public float x1, y1, x2, y2;
        public float score;
        public String cls;

        public YoloDetection(float x1, float y1, float x2, float y2, float score, String cls) {
            this.x1 = x1;
            this.y1 = y1;
            this.x2 = x2;
            this.y2 = y2;
            this.score = score;
            this.cls = cls;
        }
    }

    public static class DetectionResult implements InferenceResult {
        public ObjectDetectionManager.YoloDetection[] detections;
        public DetectionResult(ObjectDetectionManager.YoloDetection[] detections) {
            this.detections = detections;
        }
    }

    private boolean isInitialized = false;
    private double inferenceTime = 0;

    @Override
    public void initialize(String device, String nativeLibDir,
                           String model, String backend,
                           String precision, String framework) {
//        NativeInterface.initializeODManagerJNI(device, nativeLibDir, model, backend, precision, framework);
        NativeInterface.initializeModelJNI(
                taskType.ordinal(),
                device,
                nativeLibDir,
                model,
                backend,
                precision,
                framework
        );
        isInitialized = true;
        LogUtils.info("[ObjectDetectionManager] Object detection manager initialized.");
    }

    @Override
    public boolean isInitialized() {
        return isInitialized;
    }

    @Override
    public TaskType getTaskType() {
        return taskType;
    }

    @Override
    public void setInferenceTime(double inferenceTime) {
        this.inferenceTime = inferenceTime;
    }

    @Override
    public double getInferenceTime() {
        return inferenceTime;
    }


    /**
     * Runs the object detection model on YUV image data and returns YoloDetection results.
     */
    @Override
    public InferenceResult runInference(byte[] cameraImageData, int width, int height) {
        if (!isInitialized) {
            LogUtils.error("[ObjectDetectionManager] runInference called before initialization!");
            return new DetectionResult(new YoloDetection[0]);
        }
        // Actually run detection
        double startTime = System.nanoTime();
        YoloDetection[] boxes = NativeInterface.getObjectBoxesJNI(cameraImageData, width, height);
        double endTime = System.nanoTime();
        inferenceTime = (endTime - startTime) / 1e6; // ms
        setInferenceTime(inferenceTime);

        return new DetectionResult(boxes); // specialized result
    }

    @Override
    public void setPowerMode(int powerMode) {
        NativeInterface.setPowerModeJNI(powerMode);
    }



}
