package com.lovelyzzkei.qnnSkeleton.GL;

import android.content.Context;
import android.opengl.GLES20;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

public class Cube {

    private final Context context;

    // Vertex shader code
//    private final String vertexShaderCode =
//            "attribute vec4 aPosition;" +
//            "attribute vec4 aColor;" +
//            "uniform mat4 uMVPMatrix;" +
//            "varying vec4 vColor;" +
//            "void main() {" +
//            "  vColor = aColor;" +
//            "  gl_Position = uMVPMatrix * aPosition;" +
//            "}";

    private final String vertexShaderCode =
        "uniform mat4 uMVPMatrix;" +
        "attribute vec4 vPosition;" +
        "attribute vec4 aColor;" +
        "varying vec4 vColor;" +
        "void main() {" +
        "  gl_Position = uMVPMatrix * vPosition;" +
        "  vColor = aColor;" +
        "}";

    // Fragment shader code
    private final String fragmentShaderCode =
        "precision mediump float;" +
        "varying vec4 vColor;" +
        "void main() {" +
        "  gl_FragColor = vColor;" +
        "}";

    private final FloatBuffer vertexBuffer;
    private final FloatBuffer colorBuffer;
    private final ShortBuffer drawListBuffer;
    private final int mProgram;
    private int positionHandle;
    private int colorHandle;
    private int mvpMatrixHandle;

    private static final int COORDS_PER_VERTEX = 3;
    private static final int COLORS_PER_VERTEX = 4;
    private static final int vertexStride = COORDS_PER_VERTEX * 4; // 4 bytes per vertex
    private final int colorStride = COLORS_PER_VERTEX * 4; // bytes per color

    // Define a cube with 8 vertices and 6 faces
//    private final float[] cubeCoords = {
//            -1.0f, 1.0f, -1.0f,  // top left front
//            -1.0f, -1.0f, -1.0f, // bottom left front
//            1.0f, -1.0f, -1.0f,  // bottom right front
//            1.0f, 1.0f, -1.0f,   // top right front
//            -1.0f, 1.0f, 1.0f,   // top left back
//            -1.0f, -1.0f, 1.0f,  // bottom left back
//            1.0f, -1.0f, 1.0f,   // bottom right back
//            1.0f, 1.0f, 1.0f     // top right back
//    };

//    private final short[] drawOrder = {
//            0, 1, 2, 0, 2, 3,   // front face
//            4, 5, 6, 4, 6, 7,   // back face
//            0, 1, 5, 0, 5, 4,   // left face
//            2, 3, 7, 2, 7, 6,   // right face
//            0, 3, 7, 0, 7, 4,   // top face
//            1, 2, 6, 1, 6, 5    // bottom face
//    };
    private static final float[] cubeCoords = { // 24
        -1.0f,  1.0f,  1.0f,  // front top left
        1.0f,  1.0f,  1.0f,  // front top right
        -1.0f, -1.0f,  1.0f,  // front bottom left
        1.0f, -1.0f,  1.0f,  // front bottom right
        -1.0f,  1.0f, -1.0f,  // back top left
        1.0f,  1.0f, -1.0f,  // back top right
        -1.0f, -1.0f, -1.0f,  // back bottom left
        1.0f, -1.0f, -1.0f   // back bottom right
    };

    private final short[] drawOrder = { // 36
            0, 1, 2, 1, 2, 3, // front face
            4, 5, 6, 5, 6, 7, // back face
            0, 1, 4, 1, 4, 5, // top face
            2, 3, 6, 3, 6, 7, // bottom face
            0, 2, 4, 2, 4, 6, // left face
            1, 3, 5, 3, 5, 7  // right face
    };

    private final float[] colors = {    // 32
            1.0f, 0.0f, 0.0f, 1.0f, // red,     front top left
            0.0f, 1.0f, 0.0f, 1.0f, // green,   front top right
            0.0f, 0.0f, 1.0f, 1.0f, // blue,    front bottom left
            1.0f, 1.0f, 0.0f, 1.0f, // yellow,  front bottom right
            1.0f, 0.0f, 1.0f, 1.0f, // magenta, back top left
            0.0f, 1.0f, 1.0f, 1.0f, // cyan,    back top right
            1.0f, 0.5f, 0.0f, 1.0f, // orange,  back bottom left
            0.5f, 0.0f, 1.0f, 1.0f  // purple,  back bottom right
    };

    public Cube(Context context) {
        this.context = context;

        // Initialize vertex byte buffer for shape coordinates
        ByteBuffer bb = ByteBuffer.allocateDirect(cubeCoords.length * 4); // 4 bytes per float
        bb.order(ByteOrder.nativeOrder());
        vertexBuffer = bb.asFloatBuffer();
        vertexBuffer.put(cubeCoords);
        vertexBuffer.position(0);

        // Initialize byte buffer for the draw list
        ByteBuffer dlb = ByteBuffer.allocateDirect(drawOrder.length * 2); // 2 bytes per short
        dlb.order(ByteOrder.nativeOrder());
        drawListBuffer = dlb.asShortBuffer();
        drawListBuffer.put(drawOrder);
        drawListBuffer.position(0);

        // Initialize color byte buffer
        ByteBuffer cb = ByteBuffer.allocateDirect(colors.length * 4); // 4 bytes per float
        cb.order(ByteOrder.nativeOrder());
        colorBuffer = cb.asFloatBuffer();
        colorBuffer.put(colors);
        colorBuffer.position(0);

        // Prepare shaders and OpenGL program
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, vertexShaderCode);
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentShaderCode);

        mProgram = GLES20.glCreateProgram(); // create empty OpenGL Program
        GLES20.glAttachShader(mProgram, vertexShader); // add the vertex shader to program
        GLES20.glAttachShader(mProgram, fragmentShader); // add the fragment shader to program
        GLES20.glLinkProgram(mProgram); // create OpenGL program executables
    }

    public void draw(float[] mvpMatrix) {
        // Add program to OpenGL environment
        GLES20.glUseProgram(mProgram);

        // Get handle to vertex shader's vPosition member
        positionHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");

        // Enable a handle to the cube vertices
        GLES20.glEnableVertexAttribArray(positionHandle);

        // Prepare the cube coordinate data
        GLES20.glVertexAttribPointer(positionHandle, COORDS_PER_VERTEX,
                GLES20.GL_FLOAT, false,
                vertexStride, vertexBuffer);


        // Get handle to fragment shader's vColor member
        colorHandle = GLES20.glGetAttribLocation(mProgram, "aColor");

        // Enable a handle to the cube colors
        GLES20.glEnableVertexAttribArray(colorHandle);

        // Prepare the cube color data
        GLES20.glVertexAttribPointer(colorHandle, COLORS_PER_VERTEX,
                GLES20.GL_FLOAT, false,
                COLORS_PER_VERTEX * 4, colorBuffer);


        // Get handle to shape's transformation matrix
        mvpMatrixHandle = GLES20.glGetUniformLocation(mProgram, "uMVPMatrix");

        // Pass the projection and view transformation to the shader
        GLES20.glUniformMatrix4fv(mvpMatrixHandle, 1, false, mvpMatrix, 0);

        // Draw the cube
        GLES20.glDrawElements(GLES20.GL_TRIANGLES, drawOrder.length,
                GLES20.GL_UNSIGNED_SHORT, drawListBuffer);

        // Disable vertex array
        GLES20.glDisableVertexAttribArray(positionHandle);
        GLES20.glDisableVertexAttribArray(colorHandle);
    }

    private int loadShader(int type, String shaderCode) {
        // Create a vertex shader type (GLES20.GL_VERTEX_SHADER)
        // or a fragment shader type (GLES20.GL_FRAGMENT_SHADER)
        int shader = GLES20.glCreateShader(type);

        // Add the source code to the shader and compile it
        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);

        return shader;
    }
}
