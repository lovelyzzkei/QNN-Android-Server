package com.lovelyzzkei.qnnSkeleton;

import android.content.Intent;
import android.os.Bundle;
//import android.support.annotation.NonNull;

import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.lovelyzzkei.qnnSkeleton.common.LogUtils;
import com.lovelyzzkei.qnnSkeleton.tasks.base.TaskType;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // OD Model Spinner
        Spinner odModelSpinner = findViewById(R.id.spinner_model);
        Spinner deModelSpinner = findViewById(R.id.spinner_de_model);
        // TODO: Retrieve models from jniLibs directory?
        String[] odModels = {"YOLOv6"};
        String[] deModels = {"Depth-Anything-V2"};
        setupSpinner(odModelSpinner, odModels);
        setupSpinner(deModelSpinner, deModels);


        // OD Framework Spinner
        Spinner odFrameworkSpinner = findViewById(R.id.spinner_framework);
        Spinner deFrameworkSpinner = findViewById(R.id.spinner_de_framework);
        String[] frameworks = {"QNN"};
        setupSpinner(odFrameworkSpinner, frameworks);
        setupSpinner(deFrameworkSpinner, frameworks);


        // OD Backend Spinner
        Spinner odBackendSpinner = findViewById(R.id.spinner_backend);
        Spinner deBackendSpinner = findViewById(R.id.spinner_de_backend);
        String[] backends = {"NPU", "CPU"};
        setupSpinner(odBackendSpinner, backends);
        setupSpinner(deBackendSpinner, backends);


        // OD Precision Spinner
        Spinner odPrecisionSpinner = findViewById(R.id.spinner_precision);
        Spinner dePrecisionSpinner = findViewById(R.id.spinner_de_precision);
        String[] precisions = {"FP32", "FP16"};
        setupSpinner(odPrecisionSpinner, precisions);
        setupSpinner(dePrecisionSpinner, precisions);


        Button odButton = findViewById(R.id.od_button);
        odButton.setOnClickListener(v -> {
            String selectedModel = odModelSpinner.getSelectedItem().toString();
            String selectedFramework = odFrameworkSpinner.getSelectedItem().toString();
            String selectedBackend = odBackendSpinner.getSelectedItem().toString();
            String selectedPrecision = odPrecisionSpinner.getSelectedItem().toString();

            Toast.makeText(MainActivity.this,
                    "Model: " + selectedModel + " Framework: " + selectedFramework +
                            "\nBackend: " + selectedBackend + " Precision: " + selectedPrecision,
                    Toast.LENGTH_LONG).show();

            // Start Activity
            Intent intent = new Intent(MainActivity.this, ARActivity.class);
            intent.putExtra("SELECTED_TASK", TaskType.OBJECT_DETECTION.name());
            intent.putExtra("SELECTED_MODEL", selectedModel);
            intent.putExtra("SELECTED_FRAMEWORK", selectedFramework);
            intent.putExtra("SELECTED_BACKEND", selectedBackend);
            intent.putExtra("SELECTED_PRECISION", selectedPrecision);
            startActivity(intent);
        });


        Button deButton = findViewById(R.id.de_button);
        deButton.setOnClickListener(v -> {
            String selectedModel = deModelSpinner.getSelectedItem().toString();
            String selectedFramework = deFrameworkSpinner.getSelectedItem().toString();
            String selectedBackend = deBackendSpinner.getSelectedItem().toString();
            String selectedPrecision = dePrecisionSpinner.getSelectedItem().toString();

            Toast.makeText(MainActivity.this,
                    "Model: " + selectedModel + " Framework: " + selectedFramework +
                            "\nBackend: " + selectedBackend + " Precision: " + selectedPrecision,
                    Toast.LENGTH_LONG).show();

            // Start Activity
            Intent intent = new Intent(MainActivity.this, ARActivity.class);
            intent.putExtra("SELECTED_TASK", TaskType.DEPTH_ESTIMATION.name());
            intent.putExtra("SELECTED_MODEL", selectedModel);
            intent.putExtra("SELECTED_FRAMEWORK", selectedFramework);
            intent.putExtra("SELECTED_BACKEND", selectedBackend);
            intent.putExtra("SELECTED_PRECISION", selectedPrecision);
            startActivity(intent);
        });

        hideSystemUI();
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
