package com.lovelyzzkei.qnnSkeleton;

import com.lovelyzzkei.qnnSkeleton.tasks.ObjectDetectionManager.YoloDetection;

public class NativeInterface {
    static {
        // Load your JNI libraries here
        System.loadLibrary("QnnAndroid");
    }

    // ADSP / QNN library init
    public static native String setAdspLibraryPathJNI(String nativeLibDir);

    // Manager initialization
    public static native void initializeODManagerJNI(String device, String nativeLibDir,
                                                     String model, String backend,
                                                     String precision, String framework);

    public static native void initializeDEManagerJNI(String device, String nativeLibDir,
                                                     String model, String backend,
                                                     String precision, String framework);

    // Object detection
    public static native YoloDetection[] getObjectBoxesJNI(byte[] cameraImageBuffer, int width, int height);

    // Depth estimation
    public static native float[] getDepthMapJNI(byte[] cameraImageBuffer, int width, int height);

}
