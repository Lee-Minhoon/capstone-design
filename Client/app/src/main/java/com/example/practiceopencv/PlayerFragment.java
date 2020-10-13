package com.example.practiceopencv;

import android.media.MediaPlayer;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.fragment.app.Fragment;

import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.List;

public class PlayerFragment extends Fragment {
    private View view;
    private LinearLayout linearLayout;
    private TextView textView;
    private TextView playTime;
    private TextView totalTime;
    private SeekBar seekBar;
    private File file;
    private File[] files;
    private List<String> fileNameList;
    private ListView listView;
    private ArrayAdapter<String> arrayAdapter;
    private MediaPlayer mediaPlayer;
    private int pausePosition;
    private Thread thread;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.fragment_player, container, false);
        textView = (TextView) view.findViewById(R.id.textView);
        playTime = (TextView) view.findViewById(R.id.playTime);
        totalTime = (TextView) view.findViewById(R.id.totalTime);
        seekBar = (SeekBar) view.findViewById(R.id.seekBar);
        linearLayout = (LinearLayout) view.findViewById(R.id.player);
        linearLayout.setVisibility(View.GONE);

        file = getContext().getFilesDir();
        files = file.listFiles();
        fileNameList = new ArrayList<>();
        for(int i = 0; i < files.length; i++) {
            fileNameList.add(files[i].getName());
        }
        arrayAdapter = new ArrayAdapter<String>(
                getActivity(),
                android.R.layout.simple_list_item_1,
                fileNameList
        );

        listView = (ListView) view.findViewById(R.id.list);
        listView.setAdapter(arrayAdapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Object object = (Object) listView.getAdapter().getItem(position);
                Toast.makeText(getActivity().getApplicationContext(), "" + object, Toast.LENGTH_SHORT).show();
                try {
                    if (mediaPlayer != null) {
                        mediaPlayer.stop();
                        mediaPlayer = null;
                    }
                    textView.setText("" + object);
                    FileInputStream fileInputStream = new FileInputStream(getContext().getFilesDir() + "/" + object);
                    mediaPlayer = new MediaPlayer();
                    mediaPlayer.setDataSource(fileInputStream.getFD());
                    fileInputStream.close();
                    mediaPlayer.prepare();
                    mediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
                        @Override
                        public void onCompletion(MediaPlayer mp) {
                            seekBar.setProgress(0);
                            pausePosition = 0;
                            thread.interrupt();
                        }
                    });
                    playTime.setText("0:00");
                    String minute = "" + mediaPlayer.getDuration() / 1000 / 60;
                    String second = String.format("%02d", (mediaPlayer.getDuration() / 1000 % 60));
                    String time = minute + ":" + second;
                    totalTime.setText(time);
                    seekBar.setProgress(0);
                    seekBar.setMax(mediaPlayer.getDuration());
                    seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                        @Override
                        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                            if(fromUser) {
                                mediaPlayer.seekTo(progress);
                                if(seekBar.getProgress() == mediaPlayer.getDuration()) {
                                    seekBar.setProgress(0);
                                    pausePosition = 0;
                                    thread.interrupt();
                                }
                            }
                        }
                        @Override
                        public void onStartTrackingTouch(SeekBar seekBar) {

                        }
                        @Override
                        public void onStopTrackingTouch(SeekBar seekBar) {

                        }
                    });
                    linearLayout.setVisibility(View.VISIBLE);
                    pausePosition = 0;
                    thread = new Thread(new Runnable() {
                        @Override
                        public void run() {
                            while(!Thread.currentThread().isInterrupted()) {
                                try {
                                    Thread.sleep(1000);
                                } catch (InterruptedException e) {
                                    Thread.currentThread().interrupt();
                                }
                                if (!Thread.currentThread().isInterrupted()) {
                                    if (mediaPlayer != null) {
                                        Log.i("thread", "running");
                                        String minute = "" + mediaPlayer.getCurrentPosition() / 1000 / 60;
                                        String second = String.format("%02d", (mediaPlayer.getCurrentPosition() / 1000 % 60));
                                        String time = minute + ":" + second;
                                        playTime.setText(time);
                                        seekBar.setProgress(mediaPlayer.getCurrentPosition());
                                    }
                                }
                            }
                        }
                    });
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });

        ImageButton button01 = view.findViewById(R.id.button01);
        button01.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!mediaPlayer.isPlaying()) {
                    mediaPlayer.seekTo(pausePosition);
                    mediaPlayer.start();
                    if (thread.getState() == Thread.State.NEW){
                        thread.start();
                    } else {
                        thread = new Thread(new Runnable() {
                            @Override
                            public void run() {
                                while(!Thread.currentThread().isInterrupted()) {
                                    try {
                                        Thread.sleep(1000);
                                    } catch (InterruptedException e) {
                                        Thread.currentThread().interrupt();
                                    }
                                    if (!Thread.currentThread().isInterrupted()) {
                                        if (mediaPlayer != null) {
                                            Log.i("thread", "running");
                                            String minute = "" + mediaPlayer.getCurrentPosition() / 1000 / 60;
                                            String second = String.format("%02d", (mediaPlayer.getCurrentPosition() / 1000 % 60));
                                            String time = minute + ":" + second;
                                            playTime.setText(time);
                                            seekBar.setProgress(mediaPlayer.getCurrentPosition());
                                        }
                                    }
                                }
                            }
                        });
                        thread.start();
                    }
                }
            }
        });

        ImageButton button02 = view.findViewById(R.id.button02);
        button02.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mediaPlayer.isPlaying()) {
                    mediaPlayer.pause();
                    pausePosition = mediaPlayer.getCurrentPosition();
                }
            }
        });

        ImageButton button03 = view.findViewById(R.id.button03);
        button03.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mediaPlayer != null) {
                    mediaPlayer.stop();
                    mediaPlayer = null;
                    thread.interrupt();
                    linearLayout.setVisibility(View.GONE);
                }
            }
        });

        return view;
    }

    @Override
    public void onStop() {
        super.onStop();
        if (mediaPlayer != null) {
            mediaPlayer.stop();
            thread.interrupt();
            mediaPlayer = null;
        }
    }
}
