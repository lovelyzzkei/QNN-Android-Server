package com.lovelyzzkei.qnnSkeleton.common;

import android.util.Log;

public class LogUtils {
    public static final String TAG = "QNNSKELETON";
    public static void info(String msg) {
        Log.i(TAG, msg);
    }
    public static void debug(String msg) {
        Log.d(TAG, msg);
    }
    public static void error(String msg) {
        Log.e(TAG, msg);
    }

}
