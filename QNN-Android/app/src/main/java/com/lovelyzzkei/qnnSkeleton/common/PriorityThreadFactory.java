package com.lovelyzzkei.qnnSkeleton.common;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicInteger;

import android.os.Process;

public class PriorityThreadFactory implements ThreadFactory {
    private final int[] threadPriorities;
    private final AtomicInteger count = new AtomicInteger(0);

    public PriorityThreadFactory(int[] threadPriorities) {
        this.threadPriorities = threadPriorities;
    }

    @Override
    public Thread newThread(Runnable r) {
        int priorityIndex = count.getAndIncrement() % threadPriorities.length;
        int threadPriority = threadPriorities[priorityIndex];
        return new Thread(() -> {
            // Set thread priority
            Process.setThreadPriority(threadPriority);
            r.run();
        });
    }
}