package com.lovelyzzkei.qnnSkeleton.tasks;

import com.lovelyzzkei.qnnSkeleton.NativeInterface;
import com.lovelyzzkei.qnnSkeleton.common.LogUtils;
import com.lovelyzzkei.qnnSkeleton.tasks.base.BaseManager;
import com.lovelyzzkei.qnnSkeleton.tasks.base.InferenceResult;
import com.lovelyzzkei.qnnSkeleton.tasks.base.TaskType;

import java.util.ArrayList;
import java.util.List;

public class SuperResolutionManager implements BaseManager {
    private boolean isInitialized = false;
    private double inferenceTime = 0;
    private static List<String> labels = new ArrayList<>();
    private final TaskType taskType = TaskType.SUPER_RESOLUTION;

    public static class SuperResolutionResult implements InferenceResult {
        public float[] upscaledImage;
        public SuperResolutionResult(float[] upscaledImage) {
            this.upscaledImage = upscaledImage;
        }
    }

    @Override
    public void initialize(String device, String nativeLibDir,
                           String model, String backend,
                           String precision, String framework, int vtcmSize, int offset) {
        NativeInterface.initializeModelJNI(
                taskType.ordinal(),
                device,
                nativeLibDir,
                model,
                backend,
                precision,
                framework,
                vtcmSize,
                offset
        );
        isInitialized = true;
        LogUtils.info("[ImageClassificationManager] Image classification manager initialized.");
    }

    @Override
    public boolean isInitialized() {
        return isInitialized;
    }

    @Override
    public TaskType getTaskType() {
        return taskType;
    }

    /**
     * Run the depth-estimation model and return a float[] depth map
     */
    @Override
    public InferenceResult runInference(byte[] cameraImageData, int width, int height) {
        if (!isInitialized) {
            LogUtils.error("[ImageClassificationManager] runInference called before initialization!");
            return new ImageClassificationManager.ImageClassificationResult(new float[0]);
        }
        // Actually run depth estimation
        double startTime = System.nanoTime();
        float[] upscaledImage = NativeInterface.getUpscaledImageJNI(cameraImageData, width, height);
        double endTime = System.nanoTime();
        inferenceTime = (endTime - startTime) / 1e6; // ms
        setInferenceTime(inferenceTime);

        return new SuperResolutionManager.SuperResolutionResult(upscaledImage);
    }

    @Override
    public void setPowerMode(int powerMode) {
        NativeInterface.setPowerModeJNI(powerMode);
    }

    @Override
    public void setInferenceTime(double inferenceTime) {
        this.inferenceTime = inferenceTime;
    }

    @Override
    public double getInferenceTime() {
        return inferenceTime;
    }

}
