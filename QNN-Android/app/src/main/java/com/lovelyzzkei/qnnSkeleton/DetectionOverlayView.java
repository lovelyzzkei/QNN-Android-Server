package com.lovelyzzkei.qnnSkeleton;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

import java.util.List;
import com.lovelyzzkei.qnnSkeleton.ARActivity.YoloDetection;
import com.lovelyzzkei.qnnSkeleton.common.LogUtils;

public class DetectionOverlayView extends View{
    private List<YoloDetection> detections;
    private Paint boxPaint;
    private Paint textPaint;

    private float scaleX = 640;
    private float scaleY = 480;


    public DetectionOverlayView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    private void init() {
        boxPaint = new Paint();
        boxPaint.setColor(Color.RED);
        boxPaint.setStyle(Paint.Style.STROKE);
        boxPaint.setStrokeWidth(5);

        textPaint = new Paint();
        textPaint.setColor(Color.RED);
        textPaint.setTextSize(40);
        textPaint.setStyle(Paint.Style.FILL);
    }

    public void setDetections(List<YoloDetection> detections) {
        this.detections = detections;
        invalidate(); // View 갱신
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        int inputWidth = 640;
        int inputHeight = 480;
        int screenWidth = getWidth();
        int screenHeight = getHeight();

        scaleX = (float) screenWidth / inputWidth;
        scaleY = (float) screenHeight / inputHeight;

        if (detections != null) {
            for (YoloDetection det : detections) {
                float x1 = det.x1 * scaleX;
                float y1 = det.y1 * scaleY;
                float x2 = det.x2 * scaleX;
                float y2 = det.y2 * scaleY;

                canvas.drawRect(x1, y1, x2, y2, boxPaint);
                canvas.drawText(det.cls + " (" + det.score + ")", x1, y1 + 50, textPaint);
            }
        }
    }
}
