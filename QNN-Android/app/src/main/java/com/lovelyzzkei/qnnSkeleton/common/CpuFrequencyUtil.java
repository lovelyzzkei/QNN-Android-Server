package com.lovelyzzkei.qnnSkeleton.common;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.util.Log;
import android.os.Process;

public class CpuFrequencyUtil {
    public List<Integer> littleFreqList;
    public List<Integer> mediumFreqList;
    public List<Integer> bigFreqList;
    public List<Integer> coreList;

    private static final String TAG = "CpuFrequencyUtil";

    public CpuFrequencyUtil() {
        littleFreqList = new ArrayList<>();
        mediumFreqList = new ArrayList<>();
        bigFreqList = new ArrayList<>();
        coreList = new ArrayList<>();
    }

    /**
     * Get the current CPU frequency for a specific core.
     * @param coreIndex The index of the CPU core (e.g., 0 for cpu0).
     * @return The current frequency in kHz, or -1 if an error occurred.
     */
    public static int getCurrentCpuFreq(int coreIndex) {
        String cpuFreqPath = "/sys/devices/system/cpu/cpu" + coreIndex + "/cpufreq/scaling_cur_freq";
        try {
            BufferedReader br = new BufferedReader(new FileReader(cpuFreqPath));
            String line = br.readLine();
            br.close();
            return Integer.parseInt(line);
        } catch (IOException | NumberFormatException e) {
            Log.e(TAG, "Error reading CPU frequency for core " + coreIndex, e);
            return -1;
        }
    }

    public static int getCurrentCpuCore() {
        int tid = Process.myTid();  // Get the thread ID
        int pid = Process.myPid();  // Get the process ID
        String statPath = "/proc/" + pid + "/task/" + tid + "/stat";
        LogUtils.info(statPath);

        try (BufferedReader reader = new BufferedReader(new FileReader(statPath))) {
            String statLine = reader.readLine();
            if (statLine != null) {
                LogUtils.info(statLine);
                String[] stats = statLine.split("\\s+");
                if (stats.length >= 39) {
                    return Integer.parseInt(stats[38]);  // The CPU core ID is the 39th field
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        return -1;  // Return -1 if the CPU core could not be determined
    }

    public int getTid() {
        return Process.myTid();  // Get the thread ID
    }

    public void addFreqData() {
        littleFreqList.add(getCurrentCpuFreq(0));
        mediumFreqList.add(getCurrentCpuFreq(4));
        bigFreqList.add(getCurrentCpuFreq(7));
        coreList.add(getCurrentCpuCore());
    }

    @SuppressLint("DefaultLocale")
    public void printFreqData() {
        LogUtils.info(String.format("[Frequency information]\n" +
                                    "- Little   : %d\n" +
                                    "- Medium   : %d\n" +
                                    "- Big      : %d",
                littleFreqList.get(littleFreqList.size()-1),
                mediumFreqList.get(mediumFreqList.size()-1),
                bigFreqList.get(bigFreqList.size()-1)));
    }

    public void saveCpuData(String filePath) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(filePath))) {
            writer.write("Frame,Little,Medium,Big,Curr Core\n");
            for (int i = 0; i < bigFreqList.size(); i++) {
                writer.write(i + 1 + "," +
                    littleFreqList.get(i) + "," +
                    mediumFreqList.get(i) + "," +
                    bigFreqList.get(i) + "," +
                    coreList.get(i) + "\n");
            }
            System.out.println("Cpu frequency data saved successfully to " + filePath);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

