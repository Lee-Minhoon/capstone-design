package com.example.practiceopencv;

import android.app.ProgressDialog;
import android.content.Context;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.FileOutputStream;
import java.net.Socket;
import java.util.Calendar;
import java.util.TimeZone;

public class socketThread extends Thread {
    private int select;
    private Context context;
    private ProgressDialog progressDialog;
    private int key;
    private int[][] notesInfo;
    private int bpm;
    private int instrument;
    private int beat;
    private int melody;
    private int value;
    private Socket socket;
    private String ip = "192.168.219.103";
    // private String ip = "10.0.2.2";
    private int port = 3000;
    private DataInputStream dis;
    private DataOutputStream dos;
    private BufferedInputStream bis;
    private BufferedOutputStream bos;
    private FileOutputStream fos;

    public socketThread(int getKey, int[][] arrayParam) {
        key = getKey;
        notesInfo = arrayParam;
    }

    public socketThread(Context getContext, ProgressDialog getDialog, int getKey, int[][] arrayParam,
                        int getBpm, int getInstrument, int getBeat, int getMelody, int getValue) {
        context = getContext;
        progressDialog = getDialog;
        key = getKey;
        notesInfo = arrayParam;
        bpm = getBpm;
        instrument = getInstrument;
        beat = getBeat;
        melody = getMelody;
        value = getValue;
    }

    public void run() {
        try {
            socket = new Socket(ip, port);
            Log.i("Socket", "Socket Server Connected");
        } catch (Exception e) {
            progressDialog.dismiss();
            e.printStackTrace();
        }

        try {
            String string = "/";
            dos = new DataOutputStream(socket.getOutputStream());
            for (int i = 0; i < notesInfo.length; i++) {
                string += notesInfo[i][0] + "," + notesInfo[i][1] + "/";
            }
            string += key;
            string += "/" + bpm;
            string += "/" + instrument;
            string += "/" + beat;
            string += "/" + melody;
            string += "/" + value;
            dos.writeUTF(string);
            dos.flush();
        } catch (Exception e) {
            progressDialog.dismiss();
            e.printStackTrace();
        }

        try {
            TimeZone timeZone = TimeZone.getTimeZone("JST");
            Calendar calendar = Calendar.getInstance(timeZone);
            String now =
                    calendar.get(Calendar.YEAR) + "년 " +
                            (calendar.get(Calendar.MONTH) + 1) + "월 " +
                            calendar.get(Calendar.DATE) + "일 " +
                            calendar.get(Calendar.HOUR_OF_DAY) + "시 " +
                            calendar.get(Calendar.MINUTE) + "분 " +
                            calendar.get(Calendar.SECOND) + "초";
            bis = new BufferedInputStream(socket.getInputStream());
            fos = context.openFileOutput("" + now + ".wav", context.MODE_PRIVATE);
            bos = new BufferedOutputStream(fos);

            byte[] buffer = new byte[1024];

            int i = 0;
            while ((i = bis.read(buffer)) != - 1) {
                Log.i("Socket", i + "");
                bos.write(buffer, 0, i);
            }
            bos.flush();
            socket.close();
            progressDialog.dismiss();
        } catch (Exception e) {
            progressDialog.dismiss();
            e.printStackTrace();
        } finally {
            try {
                if (dos != null) try { dos.close(); } catch (Exception e) { e.printStackTrace(); }
                if (bis != null) try { bis.close(); } catch (Exception e) { e.printStackTrace(); }
                if (fos != null) try { fos.close(); } catch (Exception e) { e.printStackTrace(); }
                if (bos != null) try { bos.close(); } catch (Exception e) { e.printStackTrace(); }
            } catch (Exception e) {
                progressDialog.dismiss();
                e.printStackTrace();
            }
        }
    }
}
