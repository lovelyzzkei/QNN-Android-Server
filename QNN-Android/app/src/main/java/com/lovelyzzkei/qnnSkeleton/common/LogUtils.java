package com.lovelyzzkei.qnnSkeleton.common;

import android.util.Log;

public class LogUtils {
    public static void info(String msg) {
        Log.i(Constants.TAG, msg);
    }

    public static void debug(String msg) {
        Log.d(Constants.TAG, msg);
    }
}
