package com.lovelyzzkei.qnnSkeleton.common;

import android.annotation.SuppressLint;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class Latency {
    public double inputProcessing = 0;
    public double fromBitmap = 0;
    public double tensorProcessor = 0;
    public double outputProcessing = 0;
    public double inference = 0;
    public double totalLatency = 0;
    public float numFrame = 0.0f;

    public List<Double> inputProcessingList = new ArrayList<>();
    public List<Double> fromBitmapList = new ArrayList<>();
    public List<Double> tensorProcessorList = new ArrayList<>();

    public List<Double> outputProcessingList = new ArrayList<>();
    public List<Double> inferenceList = new ArrayList<>();
    public List<Double> totalLatencyList = new ArrayList<>();
    public Latency() {
        clearLatencies();
    }

    public void updateLists() {
        inputProcessingList.add(inputProcessing);
        fromBitmapList.add(fromBitmap);
        tensorProcessorList.add(tensorProcessor);
        inferenceList.add(inference);
        outputProcessingList.add(outputProcessing);
        totalLatencyList.add((inputProcessing + inference + outputProcessing));
        clearLatencies();
    }

    public void clearLatencies() {
        inputProcessing = 0;
        fromBitmap = 0;
        tensorProcessor = 0;
        outputProcessing = 0;
        inference = 0;
        totalLatency = 0;
    }

    @SuppressLint("DefaultLocale")
    public void printLatencies() {
        LogUtils.info(String.format("[LATENCY PROFILING]\n" +
                                    "- Input processing     : %f ms\n" +
                                    "- Inference            : %f ms\n" +
                                    "- Output Processing    : %f ms\n" +
                                    "- Total Latency        : %f ms"
                , inputProcessing, inference, outputProcessing, totalLatency));
    }

    public void saveLatencyData(String filePath) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(filePath))) {
            writer.write("Frame,Input Processing,fromBitmap,tensorProcessor,Output Processing,Inference,Total Latency\n");
            for (int i = 0; i < inputProcessingList.size(); i++) {
                writer.write(i + 1 + "," +
                        inputProcessingList.get(i) + "," +
                        fromBitmapList.get(i) + "," +
                        tensorProcessorList.get(i) + "," +
                        outputProcessingList.get(i) + "," +
                        inferenceList.get(i) + "," +
                        totalLatencyList.get(i) + "\n");
            }
            System.out.println("Latency data saved successfully to " + filePath);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
