package com.lovelyzzkei.qnnSkeleton;

import android.util.Log;

import com.lovelyzzkei.qnnSkeleton.common.Constants;
import com.google.ar.core.Camera;
import com.google.ar.core.Frame;
import com.google.ar.core.Pose;
import com.google.ar.core.TrackingFailureReason;
import com.google.ar.core.TrackingState;
import com.google.ar.sceneform.FrameTime;
import com.google.ar.sceneform.ux.ArFragment;

import java.util.Locale;

public class ARCoreSession {
    private MainActivity mContext;
    private ArFragment mArFragment;

    public ARCoreSession(MainActivity context) {
        mContext = context;

//        mArFragment = (ArFragment) mContext.getSupportFragmentManager().findFragmentById(R.id.ux_fragment);
//        mArFragment.getArSceneView().getPlaneRenderer().setVisible(false);
//        mArFragment.getArSceneView().getScene().addOnUpdateListener(this::onUpdateFrame);
    }

    private void onUpdateFrame(FrameTime frameTime) {

        // obtain current ARCore information
        mArFragment.onUpdate(frameTime);
        Frame frame = mArFragment.getArSceneView().getArFrame();
        Camera camera = frame.getCamera();

        // update ARCore measurements
        long timestamp = frame.getTimestamp();
        long start = System.currentTimeMillis();
        TrackingState trackingState = camera.getTrackingState();
        TrackingFailureReason trackingFailureReason = camera.getTrackingFailureReason();
        Pose T_gc = frame.getAndroidSensorPose();
        Pose T_cp = camera.getPose();

        float qx = T_gc.qx();
        float qy = T_gc.qy();
        float qz = T_gc.qz();
        float qw = T_gc.qw();

        float tx = T_gc.tx();
        float ty = T_gc.ty();
        float tz = T_gc.tz();
        Log.d(Constants.TAG, String.format("===== Tracking State : %b", camera.getTrackingState() == TrackingState.TRACKING));
        Log.d(Constants.TAG, String.format(Locale.US, "getAndroidSensorPose(): %.6f %.6f %.6f %.6f %.6f %.6f %.6f", qx, qy, qz, qw, tx, ty, tz));

        qx = T_cp.qx();
        qy = T_cp.qy();
        qz = T_cp.qz();
        qw = T_cp.qw();

        tx = T_cp.tx();
        ty = T_cp.ty();
        tz = T_cp.tz();
        long end = System.currentTimeMillis();
        Log.d(Constants.TAG, String.format(Locale.US, "Camera.getPose(): %.6f %.6f %.6f %.6f %.6f %.6f %.6f", qx, qy, qz, qw, tx, ty, tz));
        Log.d(Constants.TAG, String.format("Latency: %d ms", end-start));
    }
}
