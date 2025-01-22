package com.lovelyzzkei.qnnSkeleton.common;

import android.content.Context;
import android.os.Environment;

import org.opencv.core.Mat;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.channels.FileChannel;
import java.util.List;

public class FileUtil {

    public static void writeFloatsToFileAsText(Context context, String fileName, float num1, float num2) {
        FileOutputStream fos = null;
        BufferedWriter writer = null;

        // Get the path to the external storage (Downloads directory)
        File downloadFolder = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        File file = new File(downloadFolder, fileName);


        try {
            fos = new FileOutputStream(file);
            writer = new BufferedWriter(new OutputStreamWriter(fos));

            // Write the two float numbers as text
            writer.write(String.format("Average encoder latency: %fms", num1));
            writer.newLine(); // Write a newline character
            writer.write(String.format("Average rnn latency: %fms", num2));
            writer.newLine(); // Write another newline character

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (writer != null) {
                    writer.close();
                }
                if (fos != null) {
                    fos.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }


    private ByteBuffer readBinary(String filePath, int dim1, int dim2, int dim3) throws IOException {
        FileInputStream fis = null;
        byte[] byteArray = null;

        float[][][] data = new float[dim1][dim2][dim3];
        try {
            fis = new FileInputStream(filePath);
            byteArray = new byte[dim1 * dim2 * dim3 * 4]; // 4 bytes for each float
            fis.read(byteArray);
            fis.close();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
            e.printStackTrace();
        }

        ByteBuffer byteBuffer = ByteBuffer.wrap(byteArray).order(ByteOrder.nativeOrder());
        return byteBuffer;
    }


    public static void writeListToCSV(List<float[]> data, String filePath) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(filePath))) {
            for (float[] row : data) {
                writer.write(convertRowToCSV(row));
                writer.newLine();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static String convertRowToCSV(float[] row) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < row.length; i++) {
            sb.append(row[i]);
            if (i < row.length - 1) {
                sb.append(",");
            }
        }
        return sb.toString();
    }



    public static void writeFloatArrayToFile(float[][] array, String filename) throws IOException {
        int rows = array.length;
        int cols = array[0].length;
        float[] flatArray = new float[rows*cols];
        for (int i = 0; i < rows; i++) {
            System.arraycopy(array[i], 0, flatArray, i * cols + 0, cols);
        }
        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(rows*cols*4);
        byteBuffer.order(ByteOrder.nativeOrder());

        FloatBuffer floatBuffer = byteBuffer.asFloatBuffer();
        floatBuffer.put(flatArray);
        floatBuffer.position(0);

        FileOutputStream fos = new FileOutputStream(filename);
        FileChannel channel = fos.getChannel();
        // Write the ByteBuffer to the file channel
        channel.write(byteBuffer);

        fos.close();
    }

    public static void writeByteBufferToFile(ByteBuffer buffer, String filename) throws IOException {
        buffer.position(0);

        // Open a file output stream and get the channel
        try (FileOutputStream fos = new FileOutputStream(filename);
             FileChannel channel = fos.getChannel()) {
            // Write the ByteBuffer to the file channel
            channel.write(buffer);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private ByteBuffer convertMatToByteBuffer(Mat mat) {
        // Total number of float elements in the Mat
        int size = (int) (mat.total() * mat.channels());

        // Allocate a ByteBuffer with the appropriate size
        ByteBuffer buffer = ByteBuffer.allocateDirect(size * Float.BYTES);
        buffer.order(ByteOrder.nativeOrder());

        // Get the float array from the Mat
        float[] floatArray = new float[size];
        mat.get(0, 0, floatArray);

        // Put the float array into the ByteBuffer
        buffer.asFloatBuffer().put(floatArray);
        return buffer;
    }

    private ByteBuffer convertFloatArrayToByteBuffer(float[][] fArr) {
        // Flatten 2D array to 1D
        int rows = fArr.length;
        int cols = fArr[0].length;
        int totalFloats = rows * cols;
        int index = 0;
        float[] flatArray = new float[totalFloats];
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                flatArray[index++] = fArr[i][j];
            }
        }


        ByteBuffer byteBuffer = ByteBuffer.allocate(flatArray.length * Float.BYTES);
        byteBuffer.order(ByteOrder.nativeOrder());

        FloatBuffer floatBuffer = byteBuffer.asFloatBuffer();
        floatBuffer.put(flatArray);

        byteBuffer.position(0);
        return byteBuffer;
    }

}
