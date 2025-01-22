package com.lovelyzzkei.qnnSkeleton.GL;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLUtils;

import com.lovelyzzkei.qnnSkeleton.R;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

public class ModelObject {
    private FloatBuffer vertexBuffer;
    private FloatBuffer textureBuffer;
    private FloatBuffer colorBuffer;
    private FloatBuffer normalBuffer;
    private ShortBuffer indexBuffer;
    private int mProgram;
    private int textureId;
    private int positionHandle;
    private int mvpMatrixHandle;
    private int textureCoordHandle, textureUniformHandle;
    private int colorHandle;
    private int normalHandle;
    private ObjParser objParser;



    private static final int COORDS_PER_VERTEX = 3;
    private static final int COLORS_PER_VERTEX = 4;
    private static final int vertexStride = COORDS_PER_VERTEX * 4; // 4 bytes per vertex
    private final int colorStride = COLORS_PER_VERTEX * 4; // bytes per color
    private int vertexCount;
    private int indexCount;
    private Context context;

    public ModelObject(Context context, String fileName) throws IOException {
        this.context = context;
        this.objParser = new ObjParser(context, fileName);

        // Load and compile shaders
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, readShaderFromAssets("vertex_shader.glsl"));
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, readShaderFromAssets("fragment_shader.glsl"));


        // Initialize model texture
        int[] textures = new int[1];
        GLES20.glGenTextures(1, textures, 0);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        textureId = textures[0];

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), R.drawable.wood_color);
        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);

        mProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(mProgram, vertexShader);
        GLES20.glAttachShader(mProgram, fragmentShader);
        GLES20.glLinkProgram(mProgram);

        // vertices
        ByteBuffer bb = ByteBuffer.allocateDirect(objParser.objData.vertices.length * 4);
        bb.order(ByteOrder.nativeOrder());
        vertexBuffer = bb.asFloatBuffer();
        vertexBuffer.put(objParser.objData.vertices);
        vertexBuffer.position(0);

        // normals
        ByteBuffer nb = ByteBuffer.allocateDirect(objParser.objData.normals.length * 4);
        nb.order(ByteOrder.nativeOrder());
        normalBuffer = nb.asFloatBuffer();
        normalBuffer.put(objParser.objData.normals);
        normalBuffer.position(0);

        // indices
        ByteBuffer ib = ByteBuffer.allocateDirect(objParser.objData.vertexIndices.length * 2);
        ib.order(ByteOrder.nativeOrder());
        indexBuffer = ib.asShortBuffer();
        indexBuffer.put(objParser.objData.vertexIndices);
        indexBuffer.position(0);

        // texture coords
        textureBuffer = ByteBuffer.allocateDirect(objParser.objData.textureCoords.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        textureBuffer.put(objParser.objData.textureCoords).position(0);

        vertexCount = objParser.objData.vertices.length / 3;
        indexCount = objParser.objData.vertexIndices.length;

        // Get handle to vertex shader's vPosition member
        // Get handle to shape's transformation matrix
        positionHandle = GLES20.glGetAttribLocation(mProgram, "aPosition");
        normalHandle = GLES20.glGetAttribLocation(mProgram, "aNormal");
        mvpMatrixHandle = GLES20.glGetUniformLocation(mProgram, "uMVPMatrix");
        textureCoordHandle = GLES20.glGetAttribLocation(mProgram, "aTexCoord");
        textureUniformHandle = GLES20.glGetAttribLocation(mProgram, "sTexture");
    }

    public void draw(float[] mvpMatrix) {
        GLES20.glUseProgram(mProgram);

        // Prepare the triangle coordinate data
        GLES20.glEnableVertexAttribArray(positionHandle);
        GLES20.glEnableVertexAttribArray(normalHandle);
        GLES20.glEnableVertexAttribArray(textureCoordHandle);

        GLES20.glVertexAttribPointer(positionHandle, COORDS_PER_VERTEX,
                GLES20.GL_FLOAT, false,
                vertexStride, vertexBuffer);
        GLES20.glVertexAttribPointer(normalHandle, COORDS_PER_VERTEX,
                GLES20.GL_FLOAT, false,
                vertexStride, normalBuffer);
        GLES20.glVertexAttribPointer(textureCoordHandle, 2,
                GLES20.GL_FLOAT, false,
                0, textureBuffer);


        // Apply the projection and view transformation
        GLES20.glUniformMatrix4fv(mvpMatrixHandle, 1, false, mvpMatrix, 0);

        // Draw the object
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
        GLES20.glDrawElements(GLES20.GL_TRIANGLES, indexCount,
                GLES20.GL_UNSIGNED_SHORT, indexBuffer);


        GLES20.glDisableVertexAttribArray(positionHandle);
        GLES20.glDisableVertexAttribArray(colorHandle);
        GLES20.glDisableVertexAttribArray(normalHandle);

    }

    private int loadShader(int type, String shaderCode) {
        int shader = GLES20.glCreateShader(type);
        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);
        return shader;
    }

    private String readShaderFromAssets(String fileName) {
        try {
            InputStream inputStream = context.getAssets().open(fileName);
            byte[] buffer = new byte[inputStream.available()];
            inputStream.read(buffer);
            inputStream.close();
            return new String(buffer);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }


}
