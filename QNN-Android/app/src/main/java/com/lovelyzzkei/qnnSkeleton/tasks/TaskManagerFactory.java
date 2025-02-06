package com.lovelyzzkei.qnnSkeleton.tasks;

import com.lovelyzzkei.qnnSkeleton.tasks.base.BaseManager;
import com.lovelyzzkei.qnnSkeleton.tasks.base.TaskType;

public class TaskManagerFactory {
    public static BaseManager createManager(TaskType taskType) {
        switch (taskType) {
            case OBJECT_DETECTION:
                return new ObjectDetectionManager();
            case DEPTH_ESTIMATION:
                return new DepthEstimationManager();
            case SUPER_RESOLUTION:
                // return new SuperResolutionManager();
                // (You'd implement that class similarly)
            default:
                throw new IllegalArgumentException("Unsupported task: " + taskType);
        }
    }
}
