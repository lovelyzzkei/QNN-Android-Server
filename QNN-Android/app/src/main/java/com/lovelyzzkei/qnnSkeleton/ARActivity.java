package com.lovelyzzkei.qnnSkeleton;

import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.media.Image;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.PopupMenu;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import com.lovelyzzkei.qnnSkeleton.common.LogUtils;
import com.lovelyzzkei.qnnSkeleton.common.helpers.CameraPermissionHelper;
import com.lovelyzzkei.qnnSkeleton.common.helpers.DepthSettings;
import com.lovelyzzkei.qnnSkeleton.common.helpers.InstantPlacementSettings;
import com.lovelyzzkei.qnnSkeleton.common.helpers.TapHelper;
import com.lovelyzzkei.qnnSkeleton.samplerender.Framebuffer;
import com.lovelyzzkei.qnnSkeleton.samplerender.GLError;
import com.lovelyzzkei.qnnSkeleton.samplerender.Mesh;
import com.lovelyzzkei.qnnSkeleton.samplerender.Shader;
import com.lovelyzzkei.qnnSkeleton.samplerender.Texture;
import com.lovelyzzkei.qnnSkeleton.samplerender.arcore.BackgroundRenderer;
import com.lovelyzzkei.qnnSkeleton.common.helpers.DisplayRotationHelper;
import com.lovelyzzkei.qnnSkeleton.common.helpers.SnackbarHelper;

import com.lovelyzzkei.qnnSkeleton.samplerender.arcore.SpecularCubemapFilter;
import com.google.ar.core.ArCoreApk;
import com.google.ar.core.Camera;
import com.google.ar.core.Config;
import com.google.ar.core.Frame;
import com.google.ar.core.LightEstimate;
import com.google.ar.core.Session;
import com.google.ar.core.TrackingState;
import com.google.ar.core.exceptions.CameraNotAvailableException;
import com.google.ar.core.exceptions.NotYetAvailableException;
import com.google.ar.core.exceptions.UnavailableApkTooOldException;
import com.google.ar.core.exceptions.UnavailableArcoreNotInstalledException;
import com.google.ar.core.exceptions.UnavailableDeviceNotCompatibleException;
import com.google.ar.core.exceptions.UnavailableSdkTooOldException;
import com.google.ar.core.exceptions.UnavailableUserDeclinedInstallationException;
import com.lovelyzzkei.qnnSkeleton.samplerender.SampleRender;
import com.lovelyzzkei.qnnSkeleton.tasks.DepthEstimationManager;
import com.lovelyzzkei.qnnSkeleton.tasks.ImageClassificationManager;
import com.lovelyzzkei.qnnSkeleton.tasks.ObjectDetectionManager;
import com.lovelyzzkei.qnnSkeleton.tasks.TaskManagerFactory;
import com.lovelyzzkei.qnnSkeleton.tasks.base.BaseManager;
import com.lovelyzzkei.qnnSkeleton.tasks.base.InferenceResult;
import com.lovelyzzkei.qnnSkeleton.tasks.base.TaskType;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;


public class ARActivity extends AppCompatActivity implements SampleRender.Renderer{

    private static final String TAG = "ARctivity";
    private static final float Z_NEAR = 0.1f;
    private static final float Z_FAR = 100f;

    // See the definition of updateSphericalHarmonicsCoefficients for an explanation of these
    // constants.
    private static final float[] sphericalHarmonicFactors = {
            0.282095f,
            -0.325735f,
            0.325735f,
            -0.325735f,
            0.273137f,
            -0.273137f,
            0.078848f,
            -0.273137f,
            0.136569f,
    };

    private static final int CUBEMAP_RESOLUTION = 16;
    private static final int CUBEMAP_NUMBER_OF_IMPORTANCE_SAMPLES = 32;
    private Session session;
    private GLSurfaceView surfaceView;
    private DetectionOverlayView detectionOverlayView;
    private TextView latencyView;
    private TapHelper tapHelper;
    private SampleRender render;

    // Environmental HDR
    private Texture dfgTexture;
    private SpecularCubemapFilter cubemapFilter;

    private BackgroundRenderer backgroundRenderer;
    private Framebuffer virtualSceneFramebuffer;
//    private final List<WrappedAnchor> wrappedAnchors = new ArrayList<>();

    // Virtual object (ARCore pawn)
    private Mesh virtualObjectMesh;
    private Shader virtualObjectShader;
    private Texture virtualObjectAlbedoTexture;
    private Texture virtualObjectAlbedoInstantPlacementTexture;

    private boolean hasSetTextureNames = false;
    private boolean installRequested;
    private DisplayRotationHelper displayRotationHelper;
    private final SnackbarHelper messageSnackbarHelper = new SnackbarHelper();
    private final DepthSettings depthSettings = new DepthSettings();
    private boolean[] depthSettingsMenuDialogCheckboxes = new boolean[3];
    private final InstantPlacementSettings instantPlacementSettings = new InstantPlacementSettings();

    // Temporary matrix allocated here to reduce number of allocations for each frame.
    private final float[] modelMatrix = new float[16];
    private final float[] viewMatrix = new float[16];
    private final float[] projectionMatrix = new float[16];
    private final float[] modelViewMatrix = new float[16]; // view x model
    private final float[] modelViewProjectionMatrix = new float[16]; // projection x view x model
    private final float[] sphericalHarmonicsCoefficients = new float[9 * 3];
    private final float[] viewInverseMatrix = new float[16];
    private final float[] worldLightDirection = {0.0f, 0.0f, 0.0f, 0.0f};
    private final float[] viewLightDirection = new float[4]; // view x world light direction


    // Task managers
    private BaseManager manager;
    private TaskType selectedTask;
    private boolean turnOnInference = false;
    private boolean isInitialized = false;

    public static final String[] POWER_OPTIONS = {
        "BURST",
        "SUSTAINED_HIGH_PERFORMANCE",
        "HIGH_PERFORMANCE",
        "BALANCED",
        "LOW_BALANCED",
        "HIGH_POWER_SAVER",
        "POWER_SAVER",
        "LOW_POWER_SAVER",
        "EXTREME_POWER_SAVER"
    };
    private int currentPowerModeIndex = 0; // default to "Burst", i.e., index 0

    private int frameIdx = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_ar);

        hideSystemUI();
        FrameLayout loadingOverlay = findViewById(R.id.loadingOverlay);
        detectionOverlayView = findViewById(R.id.detectionOverlay);
        latencyView = findViewById(R.id.latency);

        loadingOverlay.bringToFront();

        // Get Intent and retrieve data
        Intent intent = getIntent();
        String taskString = intent.getStringExtra("SELECTED_TASK");
        TaskType selectedTask = TaskType.valueOf(taskString);
        String selectedModel = intent.getStringExtra("SELECTED_MODEL");
        String selectedBackend = intent.getStringExtra("SELECTED_BACKEND");
        String selectedPrecision = intent.getStringExtra("SELECTED_PRECISION");
        String selectedFramework = intent.getStringExtra("SELECTED_FRAMEWORK");

        Switch taskSwitch = findViewById(R.id.task_switch);
        taskSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            turnOnInference = isChecked;
        });

        initializeARCoreSession();

        loadingOverlay.setVisibility(View.VISIBLE);
        new Thread(() -> {
            try {
                String device = getDeviceMapping(android.os.Build.MODEL);
                String nativeLibDir = this.getApplicationInfo().nativeLibraryDir;

                // Create the manager for the chosen task
                int vtcmSizeInMB = 8;
                manager = TaskManagerFactory.createManager(selectedTask);
                manager.initialize(device,
                        nativeLibDir,
                        selectedModel,
                        selectedBackend,
                        selectedPrecision,
                        selectedFramework,
                        vtcmSizeInMB,
                        0);

                if (manager instanceof ImageClassificationManager) {
                    ImageClassificationManager.loadLabels(this, "imagenet_labels.txt");
                }

                isInitialized = true;

                ImageButton settingsButton = findViewById(R.id.settings_button);
                settingsButton.setOnClickListener(
                        v -> {
                            PopupMenu popup = new PopupMenu(ARActivity.this, v);
                            popup.setOnMenuItemClickListener(ARActivity.this::settingsMenuClick);
                            popup.inflate(R.menu.settings_menu);
                            popup.show();
                        }
                );
            }
            finally {
                runOnUiThread(() -> loadingOverlay.setVisibility(View.GONE));
            }
        }).start();
    }


    private void initializeARCoreSession() {
        surfaceView = findViewById(R.id.surfaceview);
        displayRotationHelper = new DisplayRotationHelper(/* context= */ this);

        // Set up touch listener.
        tapHelper = new TapHelper(/* context= */ this);
        surfaceView.setOnTouchListener(tapHelper);

        // Set up renderer.
        render = new SampleRender(surfaceView, this, getAssets());
        instantPlacementSettings.onCreate(this);
    }


    private String getDeviceMapping(String model) {
        // or a map from your example
        if ("SM-S9210".equals(model)) return "s24";
        if ("SM-S911N".equals(model))  return "s23";
        if ("SM-S901N".equals(model))  return "s22";
        return "unknown";
    }


    @Override
    protected void onDestroy() {
        if (session != null) {
            session.close();
            session = null;
        }
        super.onDestroy();
    }


    @Override
    protected void onResume() {
        super.onResume();

        if (session == null) {
            Exception exception = null;
            String message = null;
            try {
                // Always check the latest availability.
                ArCoreApk.Availability availability = ArCoreApk.getInstance().checkAvailability(this);

                // In all other cases, try to install ARCore and handle installation failures.
                if (availability != ArCoreApk.Availability.SUPPORTED_INSTALLED) {
                    switch (ArCoreApk.getInstance().requestInstall(this, !installRequested)) {
                        case INSTALL_REQUESTED:
                            installRequested = true;
                            return;
                        case INSTALLED:
                            break;
                    }
                }

                // ARCore requires camera permissions to operate. If we did not yet obtain runtime
                // permission on Android M and above, now is a good time to ask the user for it.
                if (!CameraPermissionHelper.hasCameraPermission(this)) {
                    CameraPermissionHelper.requestCameraPermission(this);
                    return;
                }

                // Create the session.
                session = new Session(/* context= */ this);
            } catch (UnavailableArcoreNotInstalledException
                     | UnavailableUserDeclinedInstallationException e) {
                message = "Please install ARCore";
                exception = e;
            } catch (UnavailableApkTooOldException e) {
                message = "Please update ARCore";
                exception = e;
            } catch (UnavailableSdkTooOldException e) {
                message = "Please update this app";
                exception = e;
            } catch (UnavailableDeviceNotCompatibleException e) {
                message = "This device does not support AR";
                exception = e;
            } catch (Exception e) {
                message = "Failed to create AR session";
                exception = e;
            }

            if (message != null) {
                messageSnackbarHelper.showError(this, message);
                Log.e(TAG, "Exception creating session", exception);
                return;
            }
        }

        // Note that order matters - see the note in onPause(), the reverse applies here.
        try {
            configureSession();   // For ARCore
            session.resume();
        } catch (CameraNotAvailableException e) {
            messageSnackbarHelper.showError(this, "Camera not available. Try restarting the app.");
            session = null;
            return;
        }

        surfaceView.onResume();
        displayRotationHelper.onResume();

    }


    @Override
    public void onSurfaceCreated(SampleRender render) {
        try {
            backgroundRenderer = new BackgroundRenderer(render);
            virtualSceneFramebuffer = new Framebuffer(render, /* width= */ 1, /* height= */ 1);

            cubemapFilter =
                    new SpecularCubemapFilter(
                            render, CUBEMAP_RESOLUTION, CUBEMAP_NUMBER_OF_IMPORTANCE_SAMPLES);
            // Load DFG lookup table for environmental lighting
            dfgTexture =
                    new Texture(
                            render,
                            Texture.Target.TEXTURE_2D,
                            Texture.WrapMode.CLAMP_TO_EDGE,
                            /* useMipmaps= */ false);
            // The dfg.raw file is a raw half-float texture with two channels.
            final int dfgResolution = 64;
            final int dfgChannels = 2;
            final int halfFloatSize = 2;

            ByteBuffer buffer =
                    ByteBuffer.allocateDirect(dfgResolution * dfgResolution * dfgChannels * halfFloatSize);
            try (InputStream is = getAssets().open("models/dfg.raw")) {
                is.read(buffer.array());
            }
            // SampleRender abstraction leaks here.
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, dfgTexture.getTextureId());
            GLError.maybeThrowGLException("Failed to bind DFG texture", "glBindTexture");
            GLES30.glTexImage2D(
                    GLES30.GL_TEXTURE_2D,
                    /* level= */ 0,
                    GLES30.GL_RG16F,
                    /* width= */ dfgResolution,
                    /* height= */ dfgResolution,
                    /* border= */ 0,
                    GLES30.GL_RG,
                    GLES30.GL_HALF_FLOAT,
                    buffer);
            GLError.maybeThrowGLException("Failed to populate DFG texture", "glTexImage2D");

            // Virtual object to render (ARCore pawn)
            virtualObjectAlbedoTexture =
                    Texture.createFromAsset(
                            render,
                            "models/pawn_albedo.png",
                            Texture.WrapMode.CLAMP_TO_EDGE,
                            Texture.ColorFormat.SRGB);
            virtualObjectAlbedoInstantPlacementTexture =
                    Texture.createFromAsset(
                            render,
                            "models/pawn_albedo_instant_placement.png",
                            Texture.WrapMode.CLAMP_TO_EDGE,
                            Texture.ColorFormat.SRGB);
            Texture virtualObjectPbrTexture =
                    Texture.createFromAsset(
                            render,
                            "models/pawn_roughness_metallic_ao.png",
                            Texture.WrapMode.CLAMP_TO_EDGE,
                            Texture.ColorFormat.LINEAR);

            virtualObjectMesh = Mesh.createFromAsset(render, "models/pawn.obj");
            virtualObjectShader =
                    Shader.createFromAssets(
                                    render,
                                    "shaders/environmental_hdr.vert",
                                    "shaders/environmental_hdr.frag",
                                    /* defines= */ new HashMap<String, String>() {
                                        {
                                            put(
                                                    "NUMBER_OF_MIPMAP_LEVELS",
                                                    Integer.toString(cubemapFilter.getNumberOfMipmapLevels()));
                                        }
                                    })
                            .setTexture("u_AlbedoTexture", virtualObjectAlbedoTexture)
                            .setTexture("u_RoughnessMetallicAmbientOcclusionTexture", virtualObjectPbrTexture)
                            .setTexture("u_Cubemap", cubemapFilter.getFilteredCubemapTexture())
                            .setTexture("u_DfgTexture", dfgTexture);
        } catch (IOException e) {
            Log.e(TAG, "Failed to read a required asset file", e);
            messageSnackbarHelper.showError(this, "Failed to read a required asset file: " + e);
        }
    }

    @Override
    public void onSurfaceChanged(SampleRender render, int width, int height) {
        displayRotationHelper.onSurfaceChanged(width, height);
        virtualSceneFramebuffer.resize(width, height);
    }

    @Override
    public void onDrawFrame(SampleRender render) {
        if (session == null) {
            return;
        }

        // Texture names should only be set once on a GL thread unless they change. This is done during
        // onDrawFrame rather than onSurfaceCreated since the session is not guaranteed to have been
        // initialized during the execution of onSurfaceCreated.
        if (!hasSetTextureNames) {
            session.setCameraTextureNames(
                    new int[] {backgroundRenderer.getCameraColorTexture().getTextureId()});
            hasSetTextureNames = true;
        }
        displayRotationHelper.updateSessionIfNeeded(session);


        Frame frame;
        try {
            frame = session.update();
        } catch (CameraNotAvailableException e) {
            Log.e(TAG, "Camera not available during onDrawFrame", e);
            messageSnackbarHelper.showError(this, "Camera not available. Try restarting the app.");
            return;
        }
        Camera camera = frame.getCamera();

        Image cameraImage = null;
        if (camera.getTrackingState() == TrackingState.TRACKING && isInitialized) {
            try {
                cameraImage = frame.acquireCameraImage();

            } catch (NotYetAvailableException e) {
                Log.e(TAG, "Camera image not available during onDrawFrame", e);
                messageSnackbarHelper.showError(this, "Camera image not available. Try restarting the app.");
            }
        }

        // Update BackgroundRenderer state to match the depth settings.
        try {
            backgroundRenderer.setUseDepthVisualization(
                    render,
                    (manager.getTaskType().name().equals("DEPTH_ESTIMATION") && turnOnInference));
            backgroundRenderer.setUseOcclusion(render, depthSettings.useDepthForOcclusion());
        } catch (IOException e) {
            Log.e(TAG, "Failed to read a required asset file", e);
            messageSnackbarHelper.showError(this, "Failed to read a required asset file: " + e);
            return;
        }
        // BackgroundRenderer.updateDisplayGeometry must be called every frame to update the coordinates
        // used to draw the background camera image.
        backgroundRenderer.updateDisplayGeometry(frame);


        if (camera.getTrackingState() == TrackingState.TRACKING) {
            if (isInitialized && cameraImage != null && turnOnInference) {
                // Convert YUV image to RGB and pass it to JNI
                byte[] cameraImageData = convertImage2YUVBuffer(cameraImage);
                int width = cameraImage.getWidth();
                int height = cameraImage.getHeight();

                manager.setPowerMode(currentPowerModeIndex);
                InferenceResult result = manager.runInference(cameraImageData, width, height);

                if (result instanceof ObjectDetectionManager.DetectionResult) {
                    ObjectDetectionManager.DetectionResult detectionResult = (ObjectDetectionManager.DetectionResult) result;
                    ObjectDetectionManager.YoloDetection[] detections = detectionResult.detections;
                    detectionOverlayView.setDetections(Arrays.asList(detections));
                }
                else if (result instanceof DepthEstimationManager.DepthResult) {
                    DepthEstimationManager.DepthResult depthResult = (DepthEstimationManager.DepthResult) result;
                    float[] depthMap = depthResult.depthMap;
                    renderDepthMap(depthMap);
                }
                else if (result instanceof ImageClassificationManager.ImageClassificationResult) {
                    ImageClassificationManager.ImageClassificationResult classificationResult = (ImageClassificationManager.ImageClassificationResult) result;
                    List<String> topLabels = classificationResult.getTopKLabels();
                    LogUtils.debug("Top labels: " + topLabels.toString());
                }



                updateLatency(manager.getInferenceTime());

                frameIdx++;
                if (frameIdx == 100) {
                    finish();
                }

            }
        }


        if (cameraImage != null) {
            cameraImage.close();
        }



        // -- Draw background
        if (frame.getTimestamp() != 0) {
            // Suppress rendering if the camera did not produce the first frame yet. This is to avoid
            // drawing possible leftover data from previous sessions if the texture is reused.
            backgroundRenderer.drawBackground(render);
        }

        // If not tracking, don't draw 3D objects.
        if (camera.getTrackingState() == TrackingState.PAUSED) {
            return;
        }

        // -- Draw non-occluded virtual objects (planes, point cloud)
        // Get projection matrix.
        camera.getProjectionMatrix(projectionMatrix, 0, Z_NEAR, Z_FAR);

        // Get camera matrix and draw.
        camera.getViewMatrix(viewMatrix, 0);


        // -- Draw occluded virtual objects

        // Update lighting parameters in the shader
        updateLightEstimation(frame.getLightEstimate(), viewMatrix);

        // Visualize anchors created by touch.
        render.clear(virtualSceneFramebuffer, 0f, 0f, 0f, 0f);

        // Compose the virtual scene with the background.
        backgroundRenderer.drawVirtualScene(render, virtualSceneFramebuffer, Z_NEAR, Z_FAR);
    }


    @Override
    public void onPause() {
        super.onPause();
        if (session != null) {
            // Note that the order matters - GLSurfaceView
            // is paused first so that it does not try
            // to query the session. If Session is paused before GLSurfaceView, GLSurfaceView may
            // still call session.update() and get a SessionPausedException.
            displayRotationHelper.onPause();
            surfaceView.onPause();
            session.pause();
        }
    }

    /** Configures the session with feature settings. */
    private void configureSession() {
        Config config = session.getConfig();
        config.setLightEstimationMode(Config.LightEstimationMode.ENVIRONMENTAL_HDR);
        if (session.isDepthModeSupported(Config.DepthMode.AUTOMATIC)) {
            config.setDepthMode(Config.DepthMode.AUTOMATIC);
        } else {
            config.setDepthMode(Config.DepthMode.DISABLED);
        }
        session.configure(config);
    }

    /** Update state based on the current frame's light estimation. */
    private void updateLightEstimation(LightEstimate lightEstimate, float[] viewMatrix) {
        if (lightEstimate.getState() != LightEstimate.State.VALID) {
            virtualObjectShader.setBool("u_LightEstimateIsValid", false);
            return;
        }
        virtualObjectShader.setBool("u_LightEstimateIsValid", true);

        Matrix.invertM(viewInverseMatrix, 0, viewMatrix, 0);
        virtualObjectShader.setMat4("u_ViewInverse", viewInverseMatrix);

        updateMainLight(
                lightEstimate.getEnvironmentalHdrMainLightDirection(),
                lightEstimate.getEnvironmentalHdrMainLightIntensity(),
                viewMatrix);
        updateSphericalHarmonicsCoefficients(
                lightEstimate.getEnvironmentalHdrAmbientSphericalHarmonics());
        cubemapFilter.update(lightEstimate.acquireEnvironmentalHdrCubeMap());
    }

    private void updateMainLight(float[] direction, float[] intensity, float[] viewMatrix) {
        // We need the direction in a vec4 with 0.0 as the final component to transform it to view space
        worldLightDirection[0] = direction[0];
        worldLightDirection[1] = direction[1];
        worldLightDirection[2] = direction[2];
        Matrix.multiplyMV(viewLightDirection, 0, viewMatrix, 0, worldLightDirection, 0);
        virtualObjectShader.setVec4("u_ViewLightDirection", viewLightDirection);
        virtualObjectShader.setVec3("u_LightIntensity", intensity);
    }

    private void updateSphericalHarmonicsCoefficients(float[] coefficients) {
        // Pre-multiply the spherical harmonics coefficients before passing them to the shader. The
        // constants in sphericalHarmonicFactors were derived from three terms:
        //
        // 1. The normalized spherical harmonics basis functions (y_lm)
        //
        // 2. The lambertian diffuse BRDF factor (1/pi)
        //
        // 3. A <cos> convolution. This is done to so that the resulting function outputs the irradiance
        // of all incoming light over a hemisphere for a given surface normal, which is what the shader
        // (environmental_hdr.frag) expects.
        //
        // You can read more details about the math here:
        // https://google.github.io/filament/Filament.html#annex/sphericalharmonics

        if (coefficients.length != 9 * 3) {
            throw new IllegalArgumentException(
                    "The given coefficients array must be of length 27 (3 components per 9 coefficients");
        }

        // Apply each factor to every component of each coefficient
        for (int i = 0; i < 9 * 3; ++i) {
            sphericalHarmonicsCoefficients[i] = coefficients[i] * sphericalHarmonicFactors[i / 3];
        }
        virtualObjectShader.setVec3Array(
                "u_SphericalHarmonicsCoefficients", sphericalHarmonicsCoefficients);
    }

    private void renderDepthMap(float[] depthMap) {
        // Postprocessing for rendering
        ByteBuffer depthBuffer = ((DepthEstimationManager) manager).convertFloatArrayTo16BitByteBuffer(depthMap);
        // TODO: Adjust to model output
        int depthWidth = 322;
        int depthHeight = 238;
        backgroundRenderer.updateCameraCustomDepthTexture(depthBuffer, depthWidth, depthHeight);
    }


    // Image is same as Camera2 API
    // Convert YUV image to RGB and pass it to JNI
    // Reference: https://answers.opencv.org/question/61628/android-camera2-yuv-to-rgb-conversion-turns-out-green/
    private byte[] convertImage2YUVBuffer(Image image) {
        Image.Plane Y = image.getPlanes()[0];
        Image.Plane U = image.getPlanes()[1];
        Image.Plane V = image.getPlanes()[2];

        int Yb = Y.getBuffer().remaining();
        int Ub = U.getBuffer().remaining();
        int Vb = V.getBuffer().remaining();

        byte[] data = new byte[Yb + Ub + Vb];

        Y.getBuffer().get(data, 0, Yb);
        U.getBuffer().get(data, Yb, Ub);
        V.getBuffer().get(data, Yb+ Ub, Vb);
        return data;
    }


    public ObjectDetectionManager.YoloDetection[] getObjectBoxes(Image cameraImage) {
        byte[] cameraImageData = convertImage2YUVBuffer(cameraImage);
        int width = cameraImage.getWidth();
        int height = cameraImage.getHeight();
        return NativeInterface.getObjectBoxesJNI(cameraImageData, width, height);
    }


    private void applySettingsMenuDialogCheckboxes() {
        depthSettings.setUseDepthForOcclusion(depthSettingsMenuDialogCheckboxes[0]);
        depthSettings.setARcoreDepthColorVisualizationEnabled(depthSettingsMenuDialogCheckboxes[1]);
        depthSettings.setOdEnabled(depthSettingsMenuDialogCheckboxes[2]);
        configureSession();
    }

    private void resetSettingsMenuDialogCheckboxes() {
        depthSettingsMenuDialogCheckboxes[0] = depthSettings.useDepthForOcclusion();
        depthSettingsMenuDialogCheckboxes[1] = depthSettings.arCoreDepthColorVisualizationEnabled();
        depthSettingsMenuDialogCheckboxes[2] = depthSettings.odEnabled();
    }

    private void updateLatency(final double latency) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // Format the text with the measured latency in milliseconds.
                latencyView.setText("Inference time: " + latency + " ms\n Power mode: " + POWER_OPTIONS[currentPowerModeIndex]);
            }
        });
    }


    /** Shows checkboxes to the user to facilitate toggling of depth-based effects. */
    private void launchDepthSettingsMenuDialog() {
        // Retrieves the current settings to show in the checkboxes.
        resetSettingsMenuDialogCheckboxes();

        // Shows the dialog to the user.
        Resources resources = getResources();
        if (session.isDepthModeSupported(Config.DepthMode.AUTOMATIC)) {
            // With depth support, the user can select visualization options.
            new AlertDialog.Builder(this)
                    .setTitle(R.string.options_title_with_depth)
                    .setMultiChoiceItems(
                            resources.getStringArray(R.array.depth_options_array),
                            depthSettingsMenuDialogCheckboxes,
                            (DialogInterface dialog, int which, boolean isChecked) ->
                                    depthSettingsMenuDialogCheckboxes[which] = isChecked)
                    .setPositiveButton(
                            R.string.done,
                            (DialogInterface dialogInterface, int which) -> applySettingsMenuDialogCheckboxes())
                    .setNegativeButton(
                            android.R.string.cancel,
                            (DialogInterface dialog, int which) -> resetSettingsMenuDialogCheckboxes())
                    .show();
        } else {
            // Without depth support, no settings are available.
            new AlertDialog.Builder(this)
                    .setTitle(R.string.options_title_without_depth)
                    .setPositiveButton(
                            R.string.done,
                            (DialogInterface dialogInterface, int which) -> applySettingsMenuDialogCheckboxes())
                    .show();
        }
    }

    private void showPowerConfigDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Select Power Mode")
                .setSingleChoiceItems(POWER_OPTIONS, currentPowerModeIndex,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                currentPowerModeIndex = which; // update selected index
                            }
                        })
                .setPositiveButton("Apply", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Toast.makeText(getApplicationContext(), "Power mode updated to: " +
                                        POWER_OPTIONS[currentPowerModeIndex],
                                Toast.LENGTH_SHORT).show();
                    }
                })
                .setNegativeButton("Cancel", null);
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /** Menu button to launch feature specific settings. */
    protected boolean settingsMenuClick(MenuItem item) {
        if (item.getItemId() == R.id.settings) {
            showPowerConfigDialog();
            return true;
        }
        return false;
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

//    public native String setAdspLibraryPathJNI(String nativeLibDir);
//    public native void initializeODManagerJNI(String device, String nativeLibDir, String model, String backend, String precision, String framework);
//    public native YoloDetection[] getObjectBoxesJNI(byte[] cameraImageBuffer, int width, int height);
}

/**
 * Associates an Anchor with the trackable it was attached to. This is used to be able to check
 * whether or not an Anchor originally was attached to an {@link InstantPlacementPoint}.
 */
//class WrappedAnchor {
//    private Anchor anchor;
////    private Trackable trackable;
//
//    public WrappedAnchor(Anchor anchor) {
//        this.anchor = anchor;
////        this.trackable = trackable;
//    }
//
//    public Anchor getAnchor() {
//        return anchor;
//    }
//
////    public Trackable getTrackable() {
////        return trackable;
////    }
//}
//
