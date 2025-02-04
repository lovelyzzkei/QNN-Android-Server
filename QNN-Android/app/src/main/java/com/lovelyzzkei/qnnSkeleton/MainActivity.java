/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.lovelyzzkei.qnnSkeleton;

//import com.qualcomm.qti.snpe.FloatTensor;
//import com.qualcomm.qti.snpe.SNPE;
//import com.qualcomm.qti.snpe.NeuralNetwork;


import android.content.Intent;
import android.os.Bundle;
//import android.support.annotation.NonNull;

import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.Spinner;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Model Spinner
        Spinner spinnerModel = findViewById(R.id.spinner_model);
        String[] models = {"YOLOv6"};
        setupSpinner(spinnerModel, models);

        // Framework Spinner
        Spinner spinnerFramework = findViewById(R.id.spinner_framework);
        String[] frameworks = {"QNN"};
        setupSpinner(spinnerFramework, frameworks);

        // Backend Spinner
        Spinner spinnerBackend = findViewById(R.id.spinner_backend);
        String[] backends = {"CPU", "NPU", "GPU"};
        setupSpinner(spinnerBackend, backends);

        // Precision Spinner
        Spinner spinnerPrecision = findViewById(R.id.spinner_precision);
        String[] precisions = {"FP32", "FP16"};
        setupSpinner(spinnerPrecision, precisions);


        Button ariaButton = findViewById(R.id.aria);
        ariaButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String selectedModel = spinnerModel.getSelectedItem().toString();
                String selectedFramework = spinnerFramework.getSelectedItem().toString();
                String selectedBackend = spinnerBackend.getSelectedItem().toString();
                String selectedPrecision = spinnerPrecision.getSelectedItem().toString();

                Toast.makeText(MainActivity.this,
                        "Model: " + selectedModel + " Framework: " + selectedFramework +
                                "\nBackend: " + selectedBackend + " Precision: " + selectedPrecision,
                        Toast.LENGTH_LONG).show();

                // Start Activity
                Intent intent = new Intent(MainActivity.this, ARActivity.class);
//                Intent intent = new Intent(MainActivity.this, CameraActivity.class);

                intent.putExtra("SELECTED_MODEL", selectedModel);
                intent.putExtra("SELECTED_FRAMEWORK", selectedFramework);
                intent.putExtra("SELECTED_BACKEND", selectedBackend);
                intent.putExtra("SELECTED_PRECISION", selectedPrecision);
                startActivity(intent);
            }
        });

        hideSystemUI();


//        Button ariaEvalButton = findViewById(R.id.aria_evaluation);
//        ariaEvalButton.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                // Start Activity A
//                Intent intent = new Intent(MainActivity.this, ARIAEvalActivity.class);
//                startActivity(intent);
//            }
//        });

//        Button benchmarkDEButton = findViewById(R.id.benchmark_de);
//        benchmarkDEButton.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                // Start Activity A
//                Intent intent = new Intent(MainActivity.this, BenchmarkDEActivity.class);
//                startActivity(intent);
//            }
//        });
    }

    private void hideSystemUI() {
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN);
    }

    private void setupSpinner(Spinner spinner, String[] items) {
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this,
                android.R.layout.simple_spinner_item, items);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
    }
}
