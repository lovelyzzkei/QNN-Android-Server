package com.lovelyzzkei.qnnSkeleton.common;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.media.Image;
import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

import org.opencv.android.Utils;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.MatOfInt;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

public class BitmapUtils {
//    private static final int INPUT_WIDTH = 224;
    private static final int INPUT_WIDTH = 256;

    public static byte[] bitmapToByteArray(Bitmap bitmap) {
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.JPEG, 100, stream);
        return stream.toByteArray();
    }

    public static Bitmap imageToBitmap(Image image) {
        ByteBuffer yBuffer = image.getPlanes()[0].getBuffer();
        ByteBuffer uBuffer = image.getPlanes()[1].getBuffer();
        ByteBuffer vBuffer = image.getPlanes()[2].getBuffer();

        int ySize = yBuffer.remaining();
        int uSize = uBuffer.remaining();
        int vSize = vBuffer.remaining();


        byte[] nv21 = new byte[ySize + uSize + vSize];
        yBuffer.get(nv21, 0, ySize);
        vBuffer.get(nv21, ySize, vSize);
        uBuffer.get(nv21, ySize + vSize, uSize);

        YuvImage yuvImage = new YuvImage(nv21, ImageFormat.NV21, image.getWidth(), image.getHeight(), null);
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        yuvImage.compressToJpeg(new Rect(0, 0, yuvImage.getWidth(), yuvImage.getHeight()), 100, out);
        byte[] imageBytes = out.toByteArray();

        Bitmap originalBitmap = BitmapFactory.decodeByteArray(imageBytes, 0, imageBytes.length);
        originalBitmap = rotateBitmap(originalBitmap, 90.0f);

        // TODO: Resizing Bitmap
//        if (originalBitmap != null) {
//            int dstWidth = 1248;
//            int dstHeight = 384;
//
//            resizedBitmap = Bitmap.createScaledBitmap(originalBitmap, dstWidth, dstHeight, false);
////            byte[] resizedBytes = BitmapUtils.bitmapToByteArray(resizedBitmap);
//        }

        return originalBitmap;
    }

    public static Bitmap rotateBitmap(Bitmap bitmap, float rotationDegrees) {
        Matrix mat = new Matrix();
        mat.postRotate(rotationDegrees);
        return Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), mat, false);
    }

    public static Bitmap reorderBitmap(Bitmap inputBitmap) {
        // Get the dimensions of the input Bitmap
        Mat beforeMat = new Mat();
        Mat afterMat = new Mat();
        Utils.bitmapToMat(inputBitmap, beforeMat);

        List<Mat> src = new ArrayList<>();
        List<Mat> dst = new ArrayList<>();

        src.add(beforeMat);
        dst.add(afterMat);
        Core.mixChannels(src, dst, new MatOfInt(new int[]{0,0, 1,3, 2,2, 3,1}));

        Bitmap outputBitmap = Bitmap.createBitmap(inputBitmap.getWidth(), inputBitmap.getHeight(), Bitmap.Config.RGB_565);
        return outputBitmap;
    }

    public static float[] convertBitmapToFloatArray(Bitmap bitmap) {
        // Define the dimensions of the input image
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        int channelCount = 3; // Assuming RGB image, adjust accordingly if needed

//        Log.i(Constants.TAG, String.format("width: %d height: %d", width, height));
        // Initialize the float array to store pixel values
        float[] floatArray = new float[width * height * channelCount];

        // Extract pixel values from the Bitmap
        int[] pixels = new int[width * height];
        bitmap.getPixels(pixels, 0, width, 0, 0, width, height);

        // Convert pixel values to floating-point numbers and normalize them
        for (int i = 0; i < pixels.length; i++) {
            int pixel = pixels[i];
            // Extract the color channels (R, G, B)
            float r = (float) ((pixel >> 16) & 0xFF) / 255.0f;
            float g = (float) ((pixel >> 8) & 0xFF) / 255.0f;
            float b = (float) (pixel & 0xFF) / 255.0f;
            // Set the pixel values in the float array
            floatArray[i * 3] = r;
            floatArray[i * 3 + 1] = g;
            floatArray[i * 3 + 2] = b;
        }

        return floatArray;
    }

    public static float[] reorderArray(float[] flatArray) {
        // Ensure the input array size is correct
        if (flatArray.length != 3 * 224 * 224) {
            throw new IllegalArgumentException("Input array size must be 3 * 256 * 256");
        }

        // Create a 3D array to hold the reordered values
        float[] reorderedArray = new float[256*256*3];

        // Iterate over the original array and reorder the values
        int index = 0;
        for (int y = 0; y < 256; y++) {
            for (int x = 0; x < 256; x++) {
                int idx = y * 256 + x;
                reorderedArray[idx] = flatArray[index];        // Red channel
                reorderedArray[idx+1] = flatArray[index + 256*256];  // Green channel
                reorderedArray[idx+2] = flatArray[index + 2*256*256];  // Blue channel
                index++;
            }
        }

        return reorderedArray;
    }


    private void saveBitmapAsImage(Context context, Bitmap bitmap) {
        // Check if external storage is available for write
        String state = Environment.getExternalStorageState();
        if (!Environment.MEDIA_MOUNTED.equals(state)) {
            Toast.makeText(context, "External storage not available", Toast.LENGTH_SHORT).show();
            return;
        }

        // Create a file in the external storage
        File file = new File("/sdcard/download/depthmap.png");

        FileOutputStream outputStream = null;
        try {
            outputStream = new FileOutputStream(file);
            // Compress and write the bitmap to the file
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, outputStream);
            Toast.makeText(context, "Image saved successfully", Toast.LENGTH_SHORT).show();
        } catch (IOException e) {
            e.printStackTrace();
            Toast.makeText(context, "Failed to save image", Toast.LENGTH_SHORT).show();
        } finally {
            if (outputStream != null) {
                try {
                    outputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public static float[] convertImageToFloatArray(Image image) {
        Bitmap bitmap = imageToBitmap(image);
        return convertBitmapToFloatArray(bitmap);
    }

    public static Bitmap convertByteBufferToBitmap(ByteBuffer buffer, int width, int height) {
        buffer.rewind(); // Ensure the buffer's position is at the beginning
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        bitmap.copyPixelsFromBuffer(buffer);
        return bitmap;
    }

    public static ByteBuffer clone(ByteBuffer original) {
        ByteBuffer clone = ByteBuffer.allocate(original.capacity());
        original.rewind();//copy from the beginning
        clone.put(original);
        original.rewind();
        clone.flip();
        return clone;
    }
}

