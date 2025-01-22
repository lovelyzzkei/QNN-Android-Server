package com.lovelyzzkei.qnnSkeleton.GL;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;
import android.opengl.Matrix;
import android.util.Log;

import com.lovelyzzkei.qnnSkeleton.R;
import com.lovelyzzkei.qnnSkeleton.common.LogUtils;

import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyGLRenderer implements GLSurfaceView.Renderer {
    private final Context context;
    private int textureId;
    private int modelTextureId;
    private Cube cube;
    private ModelObject modelObject;
    private SurfaceTexture surfaceTexture;

    // Matrices
    private final float[] projectionMatrix = new float[16];
    private final float[] viewMatrix = new float[16];
    private final float[] vpMatrix = new float[16];
    private final float[] modelMatrix = new float[16];
    private final float[] rotationMatrix = new float[16];
    private final float[] currentRotationMatrix = new float[16];
    private final float[] translateMatrix = new float[16];
    private final float[] scaleMatrix = new float[16];
    private final float[] mvpMatrix = new float[16];
    private final float[] transformationMatrix = new float[16];

    private float angle;
    private float objectDepth = -1.0f;
    private boolean isTransformationMatrixSet = false;

    // Positions
    private final float[] objectPosition = new float[]{0f, 0f, 4f};
    private final float[] cameraPosition = new float[]{0f, 0f, 0f, 0f};
    private float cameraX = 0f;
    private float cameraY = 0f;

    // Screen dimensions
    private int screenWidth;
    private int screenHeight;
    private float aspectRatio;

    static  {
        System.loadLibrary("imageclassifiers");
    }


    public MyGLRenderer(Context context, int screenWidth, int screenHeight) {
        this.context = context;
        this.screenWidth = screenWidth;
        this.screenHeight = screenHeight;
        Matrix.setRotateM(currentRotationMatrix, 0, 0.0f, 1.0f, 0f, 0);

    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        // Set the background frame color
        initGLSettings();
        // Initialize other rendering settings
        try {
            modelObject = new ModelObject(context, "desk2.obj");
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private void initGLSettings() {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        GLES20.glEnable(GLES20.GL_DEPTH_TEST);
        GLES20.glEnable(GLES20.GL_BLEND);
        GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);
    }

    private void setupProjectionMatrix() {
        float near = 1f;
        float far = 100f;
        float top = (float) Math.tan(Math.toRadians(60.0 / 2)) * near;
        float bottom = -top;
        float left = bottom * aspectRatio;
        float right = top * aspectRatio;

        // Use a symmetric frustum for the projection
        Matrix.frustumM(projectionMatrix, 0, left, right, bottom, top, near, far);
    }

    private void setupViewMatrix() {
        Matrix.setLookAtM(viewMatrix, 0,
                cameraPosition[0], cameraPosition[1], cameraPosition[2],
                cameraPosition[0], cameraPosition[1], 3f,
                0f, 1.0f, 0.0f);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
        setScreenSize(width, height);

        // Setup projection matrix
        setupProjectionMatrix();

        // Setup View Matrix
        setupViewMatrix();
    }


    @Override
    public void onDrawFrame(GL10 gl) {
//        LogUtils.info("======== DRAW OBJECTS ========");
        double start = System.nanoTime();
        // Redraw background color
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

        // Apply scaling, translation, rotation
        prepareModelMatrix();

        // Update the view matrix with odometry results
        updateViewMatrixWithOdometry();

        // MVP = P * V * M
        calculateMvpMatrix();

        modelObject.draw(mvpMatrix);

        double end = System.nanoTime();
        double renderLatency = (end-start) / 1000000;
//        LogUtils.info(String.format("[RENDERING]\n" +
//                "- drawFrame latency: %fms", renderLatency));
    }

    private void prepareModelMatrix() {
        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(scaleMatrix, 0);
        Matrix.setIdentityM(rotationMatrix, 0);
        Matrix.setIdentityM(translateMatrix, 0);

        float scaleX = 1f;
        Matrix.scaleM(scaleMatrix, 0, scaleX, scaleX, scaleX);
        Matrix.translateM(translateMatrix, 0, objectPosition[0], objectPosition[1], objectDepth);
        Matrix.setRotateM(rotationMatrix, 0, angle, 1.0f, 0f, 0);

        // Apply rotation and translation from odometry
        // Scaling first, then rotation, and finally translation (!!ORDER IS IMPORTANT!!)
        float[] tmpMatrix = new float[16];
        Matrix.multiplyMM(tmpMatrix, 0, currentRotationMatrix, 0, scaleMatrix, 0);
        Matrix.multiplyMM(modelMatrix, 0, translateMatrix, 0, tmpMatrix, 0);
    }

    private void calculateMvpMatrix() {
        Matrix.multiplyMM(vpMatrix, 0, projectionMatrix, 0, viewMatrix, 0);
        Matrix.multiplyMM(mvpMatrix, 0, vpMatrix, 0, modelMatrix, 0);
    }


    public void setTranslation(float x, float y, float z) {
        objectPosition[0] = x;
        objectPosition[1] = y;
        objectPosition[2] = z;
    }

    public float getTranslation(String axis) {
        if (axis.equals("X"))
            return objectPosition[0];
        else if (axis.equals("Y"))
            return objectPosition[1];
        else
            return objectPosition[2];
    }

    public void setScreenSize(int width, int height) {
        this.screenWidth = width;
        this.screenHeight = height;
        this.aspectRatio = (float) width / height;
    }

    public void setObjectDepth(float depth) {
        this.objectDepth = depth;
    }


    private void updateViewMatrixWithOdometry() {
        if (isTransformationMatrixSet) {
            // Apply the transformation matrix to the view matrix
            Matrix.multiplyMV(cameraPosition, 0, transformationMatrix, 0, cameraPosition, 0);

            LogUtils.info("Camera position: " + Arrays.toString(cameraPosition));

            // View Matrix
            Matrix.setLookAtM(viewMatrix, 0,
                    cameraPosition[0], cameraPosition[1], cameraPosition[2],
                    cameraPosition[0], cameraPosition[1], 3f,
                    0f,1.0f, 0.0f);

            isTransformationMatrixSet = false;
        }
    }


    public void updateCameraPosition(float dx, float dy) {
        cameraX += dx;
        cameraY += dy;
    }


    public void handleTouchEvent(float x, float y) {
        // Auxiliary matrix and vectors to deal with openGL
        float[] invertedMatrix = new float[16];
        float[] transformMatrix = new float[16];
        float[] normalizedInPoint = new float[4];
        float[] outPoint = new float[4];

        // Invert y coordinate, as android uses top-left, and openGL bottom-left
        int oglTouchY = (int) (screenHeight - y);

        // Generate random number between 1 and 10
        int randomDepth = (int) (Math.random() * 6) + 1;
        randomDepth = 4;

        // Transform the screen point to clip space in openGL (-1, 1)
        normalizedInPoint[0] = (float) ((2f * x / screenWidth - 1f));
        normalizedInPoint[1] = (float) (2f * (oglTouchY) / screenHeight - 1f);
        normalizedInPoint[2] = -1.0f;
        normalizedInPoint[3] = 1.0f;

        // Obtain the transform matrix and then the inverse
        Log.i("APP", "Proj: " + Arrays.toString(projectionMatrix));
        Log.i("APP", "Model: " + Arrays.toString(viewMatrix));
        Matrix.multiplyMM(transformMatrix, 0, projectionMatrix, 0, viewMatrix, 0);
        Matrix.invertM(invertedMatrix, 0, transformMatrix, 0);

        // Apply the inverse to the ponit in clip space
        Matrix.multiplyMV(outPoint, 0, invertedMatrix, 0, normalizedInPoint, 0);

        if (outPoint[3] == 0.0) {
            // Avoid division zero error
            Log.e("APP", "World coords ERROR!");
            objectPosition[0] = 0;
            objectPosition[1] = 0;
        }

        // Divide by 3rd component to find out the real position
        objectPosition[0] = (outPoint[0] / outPoint[3]) * randomDepth;
        objectPosition[1] = (outPoint[1] / outPoint[3]) * randomDepth;
        objectPosition[2] = randomDepth;

    }

    private int loadTexture(String textureFileName) {
        try {
            InputStream inputStream = context.getAssets().open(textureFileName);
            Bitmap bitmap = BitmapFactory.decodeStream(inputStream);

            int[] textureHandle = new int[1];
            GLES20.glGenTextures(1, textureHandle, 0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureHandle[0]);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);


            GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);
            bitmap.recycle();

            return textureHandle[0];
        } catch (IOException e) {
            e.printStackTrace();
            return 0;
        }
    }

    public SurfaceTexture getGLSurfaceTexture() {
        return surfaceTexture;
    }

}

