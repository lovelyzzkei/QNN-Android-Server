package com.lovelyzzkei.qnnSkeleton.tasks;


import android.content.Context;
import android.content.res.AssetManager;

import com.lovelyzzkei.qnnSkeleton.NativeInterface;
import com.lovelyzzkei.qnnSkeleton.common.LogUtils;
import com.lovelyzzkei.qnnSkeleton.tasks.base.BaseManager;
import com.lovelyzzkei.qnnSkeleton.tasks.base.InferenceResult;
import com.lovelyzzkei.qnnSkeleton.tasks.base.TaskType;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class ImageClassificationManager implements BaseManager {
    private boolean isInitialized = false;
    private double inferenceTime = 0;
    private static List<String> labels = new ArrayList<>();
    private final TaskType taskType = TaskType.IMAGE_CLASSIFICATION;

    public static class ImageClassificationResult implements InferenceResult {
        public float[] classLogits;
        private List<String> topKLabels = new ArrayList<>();
        public ImageClassificationResult(float[] classLogits) {
            this.classLogits = classLogits;

            int k = 5;
            List<Integer> topK = topKIndices(classLogits, k);
            for (int i : topK) {
                topKLabels.add(labels.get(i));
            }
        }
        public List<String> getTopKLabels() {
            return topKLabels;
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
            return new ImageClassificationResult(new float[0]);
        }
        // Actually run depth estimation
        double startTime = System.nanoTime();
        float[] classLogits = NativeInterface.getClassLogitsJNI(cameraImageData, width, height);
        double endTime = System.nanoTime();
        inferenceTime = (endTime - startTime) / 1e6; // ms
        setInferenceTime(inferenceTime);

        return new ImageClassificationResult(classLogits);
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


    public static void loadLabels(Context context, String fileName) {
        AssetManager assetManager = context.getAssets();
        try (InputStream inputStream = assetManager.open(fileName);
             BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream))) {

            String line;
            while ((line = reader.readLine()) != null) {
                // Trim to avoid trailing spaces/newline issues
                labels.add(line.trim());
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static List<Integer> topKIndices(final float[] logits, final int k) {
        // Create a list of indices 0..n-1
        List<Integer> indices = new ArrayList<>();
        for (int i = 0; i < logits.length; i++) {
            indices.add(i);
        }

        // Sort indices by descending logits
        // i2 vs i1 to get descending order
        Collections.sort(indices, new Comparator<Integer>() {
            @Override
            public int compare(Integer i1, Integer i2) {
                return Float.compare(logits[i2], logits[i1]);
            }
        });

        // Return first k indices
        return indices.subList(0, k);
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
