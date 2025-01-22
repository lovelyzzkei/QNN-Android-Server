package com.lovelyzzkei.qnnSkeleton.common;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class DummyInputGenerator {

    public static ByteBuffer generateDummyData(int[] dataShape) {
        int totalSize = 1;
        for (int s : dataShape) {
            totalSize *= s;
        }
        ByteBuffer dataBuffer = ByteBuffer.allocateDirect(4 * totalSize);
        dataBuffer.order(ByteOrder.nativeOrder());

        for (int i = 0; i < totalSize; i++) {
            dataBuffer.putFloat(0.0f); // Fill with dummy value (e.g., 0.0f)
        }

        dataBuffer.rewind();
        return dataBuffer;
    }


    public static ByteBuffer generateDummyImageInput(int[] inputShape) {
        int batchSize = inputShape[0];
        int height = inputShape[1];
        int width = inputShape[2];
        int channels = inputShape[3];

        ByteBuffer inputBuffer = ByteBuffer.allocateDirect(4 * batchSize * height * width * channels);
        inputBuffer.order(ByteOrder.nativeOrder());

        for (int i = 0; i < batchSize * height * width * channels; i++) {
            inputBuffer.putFloat(0.0f); // Fill with dummy value (e.g., 0.0f)
        }

        inputBuffer.rewind();
        return inputBuffer;
    }

    public static ByteBuffer generateDummyIMUInput(int[] inputShape) {
        int batchSize = inputShape[0];
        int imuDataSize = inputShape[1];

        ByteBuffer inputBuffer = ByteBuffer.allocateDirect(4 * batchSize * imuDataSize);
        inputBuffer.order(ByteOrder.nativeOrder());

        for (int i = 0; i < batchSize * imuDataSize; i++) {
            inputBuffer.putFloat(0.0f); // Fill with dummy value (e.g., 0.0f)
        }

        inputBuffer.rewind();
        return inputBuffer;
    }
}
