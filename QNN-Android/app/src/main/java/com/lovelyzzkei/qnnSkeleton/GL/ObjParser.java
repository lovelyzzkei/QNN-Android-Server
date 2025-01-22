package com.lovelyzzkei.qnnSkeleton.GL;

import android.content.Context;

import com.lovelyzzkei.qnnSkeleton.common.LogUtils;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


public class ObjParser {
    public static class ObjData {
        public float[] vertices;
        public float[] normals;
        public float[] textureCoords;
        public short[] vertexIndices;
        public short[] normalIndices;
        public short[] textureIndices;
    }

    private List<Float> verticesList = new ArrayList<>();
    private List<Float> normalsList = new ArrayList<>();
    private List<Float> textureCoordsList = new ArrayList<>();
    private List<Short> facesList = new ArrayList<>();
    private List<Short> normalIndicesList = new ArrayList<>();
    private List<Short> textureIndicesList = new ArrayList<>();

    public static Map<String, Material> materialMap = new HashMap<>();
    public List<String> materials = new ArrayList<>();
    public List<Integer> materialFaceIndices = new ArrayList<>();
    public ObjData objData;

    public ObjParser(Context context, String fileName) {
        objData = new ObjData();
        parseObj(context, fileName);
    }

    private void parseObj(Context context, String fileName) {
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new InputStreamReader(context.getAssets().open(fileName)));
            LogUtils.info(String.format("Loading %s success!", fileName));
            String line;
            String currentMaterial = null;
            /*
            f a/b/c a'/b'/c' a''/b''/c'' => 정점의 위치/텍스쳐 좌표의 위치/normal 좌표의 위치
             */
            while ((line = reader.readLine()) != null) {
                String[] parts = line.split("\\s+");
                switch (parts[0]) {
                    case "v":
                        verticesList.add(Float.parseFloat(parts[1]));
                        verticesList.add(Float.parseFloat(parts[2]));
                        verticesList.add(Float.parseFloat(parts[3]));
                        break;
                    case "vn":
                        normalsList.add(Float.parseFloat(parts[1]));
                        normalsList.add(Float.parseFloat(parts[2]));
                        normalsList.add(Float.parseFloat(parts[3]));
                        break;
                    case "vt":
                        textureCoordsList.add(Float.parseFloat(parts[1]));
                        textureCoordsList.add(Float.parseFloat(parts[2]));
                        break;
                    case "f":
                        for (int i = 1; i <= 3; i++) {
                            String[] vertexData = parts[i].split("/");
                            facesList.add((short) (Short.parseShort(vertexData[0]) - 1));
                            textureIndicesList.add((short) (Short.parseShort(vertexData[1]) - 1));
                            normalIndicesList.add((short) (Short.parseShort(vertexData[2]) - 1));
                        }
                        break;
                    case "mtlib":
                        String mtlFileName = parts[1];
                        parseMTL(context, mtlFileName);
                        break;
                }
            }

            objData.vertices = toFloatArray(verticesList);
            objData.normals = toFloatArray(normalsList);
            objData.textureCoords = toFloatArray(textureCoordsList);

            objData.vertexIndices = toShortArray(facesList);
            objData.normalIndices = toShortArray(normalIndicesList);
            objData.textureIndices = toShortArray(textureIndicesList);

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (reader != null)
                    reader.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void parseMTL(Context context, String mtlFileName) throws IOException {
        BufferedReader reader = new BufferedReader(new InputStreamReader(context.getAssets().open(mtlFileName)));
        String line;
        Material currentMaterial = null;

        while ((line = reader.readLine()) != null) {
            String[] parts = line.split("\\s+");
            switch (parts[0]) {
                case "newmtl":
                    if (currentMaterial != null) {
                        materialMap.put(currentMaterial.name, currentMaterial);
                    }
                    currentMaterial = new Material();
                    currentMaterial.name = parts[1];
                    break;
                case "Ka":
                    currentMaterial.ambient = new float[]{
                            Float.parseFloat(parts[1]),
                            Float.parseFloat(parts[2]),
                            Float.parseFloat(parts[3]),
                            1.0f
                    };
                    break;
                case "Kd":
                    currentMaterial.diffuse = new float[]{
                            Float.parseFloat(parts[1]),
                            Float.parseFloat(parts[2]),
                            Float.parseFloat(parts[3]),
                            1.0f
                    };
                    break;
                case "Ks":
                    currentMaterial.specular = new float[]{
                            Float.parseFloat(parts[1]),
                            Float.parseFloat(parts[2]),
                            Float.parseFloat(parts[3]),
                            1.0f
                    };
                    break;
                case "Ns":
                    currentMaterial.shininess = Float.parseFloat(parts[1]);
                    break;
                case "map_Kd":
                    currentMaterial.textureFileName = parts[1];
                    break;
            }
        }

        if (currentMaterial != null) {
            materialMap.put(currentMaterial.name, currentMaterial);
        }

        reader.close();
    }

    public static float[] toFloatArray(List<Float> list) {
        float[] array = new float[list.size()];
        for (int i = 0; i < list.size(); i++) {
            array[i] = list.get(i);
        }
        return array;
    }

    public static short[] toShortArray(List<Short> list) {
        short[] array = new short[list.size()];
        for (int i = 0; i < list.size(); i++) {
            array[i] = list.get(i);
        }
        return array;
    }


}
