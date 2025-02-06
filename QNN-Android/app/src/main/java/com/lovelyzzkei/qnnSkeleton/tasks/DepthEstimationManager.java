package com.lovelyzzkei.qnnSkeleton.tasks;

import com.lovelyzzkei.qnnSkeleton.NativeInterface;
import com.lovelyzzkei.qnnSkeleton.common.LogUtils;
import com.lovelyzzkei.qnnSkeleton.tasks.base.BaseManager;
import com.lovelyzzkei.qnnSkeleton.tasks.base.InferenceResult;
import com.lovelyzzkei.qnnSkeleton.tasks.base.TaskType;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class DepthEstimationManager implements BaseManager {
    private boolean isInitialized = false;
    private final TaskType taskType = TaskType.DEPTH_ESTIMATION;

    public static class DepthResult implements InferenceResult {
        public float[] depthMap;
        public DepthResult(float[] depthMap) {
            this.depthMap = depthMap;
        }
    }

    @Override
    public void initialize(String device, String nativeLibDir,
                           String model, String backend,
                           String precision, String framework) {
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
        LogUtils.info("[DepthEstimationManager] Depth manager initialized.");
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
            LogUtils.error("[DepthEstimationManager] runInference called before initialization!");
            return new DepthResult(new float[0]);
        }
        // Actually run depth estimation
        float[] depthMap = NativeInterface.getDepthMapJNI(cameraImageData, width, height);
        return new DepthResult(depthMap);
    }

    /**
     * Optionally: convert the float[] to 16-bit buffer for GPU usage, etc.
     */
    public ByteBuffer convertFloatArrayTo16BitByteBuffer(float[] depthArray) {
        short[] shortArray = new short[depthArray.length];
        for (int i = 0; i < depthArray.length; i++) {
            // Depth is in meters => store as millimeters
            shortArray[i] = (short) (depthArray[i] * 1000.0f);
        }

        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(shortArray.length * Short.BYTES);
        byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
        for (short value : shortArray) {
            byteBuffer.putShort(value);
        }
        byteBuffer.rewind();
        return byteBuffer;
    }
}
