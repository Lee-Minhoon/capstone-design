package com.example.practiceopencv;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import androidx.appcompat.app.AppCompatActivity;

import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Scalar;

public class TestActivity extends AppCompatActivity {
    // ============================================================================================
    // Call Libraries & Native Functions
    // ============================================================================================
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("opencv_java4");
    }
    public native void convertRGBtoGray(long matInput, long matOutput);
    public native void convertGraytoBinary(long matInput, long matOutput, long standard, long max);
    public native int[][] staffHistogram(long matInput, long matOutput, long matHisto);
    public native void staffMorphology(long matInput, long matOutput);
    public native void templateHandCraft(long matInput, long matOutput, long matTemplate);
    public native void noteExtaction(long matInput, long matOutput);
    public native int[][] noteLabeling(long matInput, long matOutput);

    // ============================================================================================
    // Create Global Variables
    // ============================================================================================
    private Mat matInput;
    private Mat matTemplate;
    private Mat matOutput;
    private Mat matHisto;
    private Bitmap bitmapInput;
    private Bitmap bitmapTemplate;
    private Bitmap bitmapOutput;
    private Bitmap bitmapCrop;
    private ImageView imageView;
    private int[][] staffsInfo;
    private int[][] notesInfo;
    private int[] pianoSounds;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test);

        // ============================================================================================
        // Variable Initialization
        // ============================================================================================
        // Create Bitmap(Original Image)
        bitmapInput = BitmapFactory.decodeResource(getResources(), R.drawable.nmusic0);
        bitmapTemplate = BitmapFactory.decodeResource(getResources(), R.drawable.template);

        // Create Mat(Input)
        matInput = new Mat(bitmapInput.getHeight(), bitmapInput.getWidth(), CvType.CV_8UC3, new Scalar(255, 255, 255));
        matTemplate = new Mat(bitmapTemplate.getHeight(), bitmapTemplate.getWidth(), CvType.CV_8UC3, new Scalar(255, 255, 255));
        Utils.bitmapToMat(bitmapInput, matInput);
        Utils.bitmapToMat(bitmapTemplate, matTemplate);

        // Create Mat(Output)
        matOutput = new Mat(bitmapInput.getHeight(), bitmapInput.getWidth(), CvType.CV_8UC3, new Scalar(255, 255, 255));
        matHisto = new Mat(bitmapInput.getHeight(), bitmapInput.getWidth(), CvType.CV_8U, new Scalar(255));

        // Create Bitmap(Output on Screen)
        bitmapOutput = Bitmap.createBitmap(matInput.cols(), matInput.rows(), Bitmap.Config.RGB_565);

        // Set ImageView
        imageView = findViewById(R.id.imageView);
        imageView.setImageBitmap(bitmapInput);

        // Call Sounds
        pianoSounds = MusicPlayer.initSound(getApplicationContext());

        // ============================================================================================
        // Set Click Listener
        // ============================================================================================
        // Default(Initialization)
        Button button01 = findViewById(R.id.button01);
        button01.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Utils.bitmapToMat(bitmapInput, matOutput);
                showImage(imageView, matOutput, bitmapOutput);
            }
        });

        // Grayscale
        Button button02 = findViewById(R.id.button02);
        button02.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                convertRGBtoGray(matInput.getNativeObjAddr(), matOutput.getNativeObjAddr());
                showImage(imageView, matOutput, bitmapOutput);
            }
        });

        // Binaryzation
        Button button03 = findViewById(R.id.button03);
        button03.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                convertGraytoBinary(matOutput.getNativeObjAddr(), matOutput.getNativeObjAddr(), 127, 255);
                showImage(imageView, matOutput, bitmapOutput);
            }
        });

        // Histogram
        Button button04 = findViewById(R.id.button04);
        button04.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                staffsInfo = staffHistogram(matOutput.getNativeObjAddr(), matOutput.getNativeObjAddr(), matHisto.getNativeObjAddr());
                showImage(imageView, matHisto, bitmapOutput);
                for (int i = 0; i < staffsInfo.length; i++) {
                    for (int j = 0; j < staffsInfo[i].length; j++) {
                        int value = staffsInfo[i][j];
                        Log.i("staffsInfo", value + "");
                    }
                }
            }
        });

        // Show Output
        Button button05 = findViewById(R.id.button05);
        button05.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                bitmapCrop = Bitmap.createBitmap(matOutput.cols(), matOutput.rows(), Bitmap.Config.RGB_565);
                showImage(imageView, matOutput, bitmapCrop);
            }
        });

        // Morphology
        Button button06 = findViewById(R.id.button06);
        button06.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                staffMorphology(matOutput.getNativeObjAddr(), matOutput.getNativeObjAddr());
                showImage(imageView, matOutput, bitmapCrop);
            }
        });

        // Template Matching
        Button button07 = findViewById(R.id.button07);
        button07.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                templateHandCraft(matOutput.getNativeObjAddr(), matOutput.getNativeObjAddr(), matTemplate.getNativeObjAddr());
                showImage(imageView, matOutput, bitmapCrop);
            }
        });

        // Note Extraction
        Button button08 = findViewById(R.id.button08);
        button08.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                noteExtaction(matOutput.getNativeObjAddr(), matOutput.getNativeObjAddr());
                showImage(imageView, matOutput, bitmapCrop);
            }
        });

        // Labeling
        Button button09 = findViewById(R.id.button09);
        button09.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                notesInfo = noteLabeling(matOutput.getNativeObjAddr(), matOutput.getNativeObjAddr());
                showImage(imageView, matOutput, bitmapCrop);
            }
        });

        // Ready to Play
        Button button10 = findViewById(R.id.button10);
        button10.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                musicReady();
            }
        });

        // Music Play
        Button button11 = findViewById(R.id.button11);
        button11.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                musicPlay();
            }
        });

        // Main Screen
        Button button12 = findViewById(R.id.button12);
        button12.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(TestActivity.this, MainActivity.class);
                startActivity(intent);
                finish();
            }
        });
    }

    // ============================================================================================
    // Create Custom Functions
    // ============================================================================================
    // Show an Image on the Screen
    private void showImage(ImageView imageView, Mat mat, Bitmap bitmap) {
        Utils.matToBitmap(mat, bitmap);
        imageView.setImageBitmap(bitmap);
    }

    // Ready to Play
    private void musicReady() {
        int distance = 20;
        int[] staffsStd = new int[17];
        int startPoint = staffsInfo[4][1] + distance * 2;
        for (int i = 0; i < 17; i++) {
            staffsStd[i] = startPoint - distance / 2 * i;
        }

        for (int i = 0; i < staffsStd.length; i++) {
            Log.i("staffsStd", staffsStd[i] + "");
        }

        for (int i = 0; i < notesInfo.length; i++) {
            for (int j = 0; j < 17; j++) {
                if (notesInfo[i][1] < staffsStd[j] + distance / 3 && notesInfo[i][1] > staffsStd[j] - distance / 3) {
                    notesInfo[i][1] = j;
                    break;
                }
            }
        }

        for (int i = 0; i < notesInfo.length; i++) {
            Log.i("notesInfo", notesInfo[i][1] + "");
        }
    }

    // Music Play
    private void musicPlay() {
        /*
        for(int i = 0; i < pianoSounds.length; i++) {
            Log.i("pianoSounds Resources", pianoSounds[i] + "");
        }
        */
        for (int i = 0; i < notesInfo.length; i++) {
            Log.i("Index", notesInfo[i][1] - 2 + "");
            MusicPlayer.play(pianoSounds[notesInfo[i][1] - 2]);
            try {
                Thread.sleep(600);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}