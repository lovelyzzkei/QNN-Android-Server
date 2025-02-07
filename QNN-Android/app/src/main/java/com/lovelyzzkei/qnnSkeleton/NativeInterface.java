package com.lovelyzzkei.qnnSkeleton;

import com.lovelyzzkei.qnnSkeleton.tasks.ObjectDetectionManager.YoloDetection;

public class NativeInterface {
    static {
        // Load your JNI libraries here
        System.loadLibrary("QnnAndroid");
    }

    // ADSP / QNN library init
    public static native String setAdspLibraryPathJNI(String nativeLibDir);

    // Model initialization
    public static native boolean initializeModelJNI(
            int taskType,
            String device,
            String nativeLibDir,
            String modelFile,
            String backend,
            String precision,
            String framework
    );

    // Power mode
    public static native void setPowerModeJNI(int powerMode);


    // Object detection
    public static native YoloDetection[] getObjectBoxesJNI(byte[] cameraImageBuffer, int width, int height);

    // Depth estimation
    public static native float[] getDepthMapJNI(byte[] cameraImageBuffer, int width, int height);

}
