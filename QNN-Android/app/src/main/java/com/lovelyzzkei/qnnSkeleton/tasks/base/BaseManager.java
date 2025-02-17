package com.lovelyzzkei.qnnSkeleton.tasks.base;

public interface BaseManager {
    /**
     * Initialize the model with the user’s configuration.
     */
    void initialize(String device,
                    String nativeLibDir,
                    String model,
                    String backend,
                    String precision,
                    String framework,
                    int vtcmSize,
                    int offset);

    /**
     * Returns true if everything is loaded and ready to run.
     */
    boolean isInitialized();

    /**
     * Returns the enum representing which task this manager handles
     * (OBJECT_DETECTION, DEPTH_ESTIMATION, SUPER_RESOLUTION, etc.)
     */
    TaskType getTaskType();

    /**
     * Runs inference on YUV image data and returns an object representing results.
     * The result can be either bounding boxes, depth map, etc.
     */
    InferenceResult runInference(byte[] cameraImageData,
                                 int width,
                                 int height);

    /*
    * Set power mode
    */
    void setPowerMode(int powerMode);

    void setInferenceTime(double inferenceTime);
    double getInferenceTime();
}
