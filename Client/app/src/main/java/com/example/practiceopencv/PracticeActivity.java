package com.example.practiceopencv;

import android.content.Intent;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

import androidx.appcompat.app.AppCompatActivity;

import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Scalar;

public class PracticeActivity extends AppCompatActivity {
    // ============================================================================================
    // Call Libraries & Native Functions
    // ============================================================================================
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("opencv_java4");
    }
    public native void preprocess1(long sheetMusic);
    public native void preprocess2(long sheetMusic);
    public native int[] preprocess3(long sheetMusic);
    public native int[] preprocess4(long sheetMusic, int staffsInfo[]);
    public native int[][] process(long sheetMusic, int staffsInfo[]);
    // public native int[] getBeatsInfo(long sheetMusic, long note02C, int staffsInfoParam[]);
    //public native int[] getNotesInfo(long sheetMusic);

    // ============================================================================================
    // Create Global Variables
    // ============================================================================================
    private TypedArray sheetMusics;
    private int arrayIndex = 0;

    private Mat matImage;
    private Bitmap bitmapImage;
    private ImageView imageView;

    private int[] staffsInfo;
    private double avgDistance = 0.00;
    private int key;
    private int[][] notesInfo;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_practice);

        // ============================================================================================
        // Variable Initialization
        // ============================================================================================
        // Call Image & Create Bitmap(Original Image)
        sheetMusics = getResources().obtainTypedArray(R.array.sheetMusics);
        bitmapImage = BitmapFactory.decodeResource(getResources(), sheetMusics.getResourceId(arrayIndex, -1));

        // Create Mat(Input)
        matImage = new Mat(bitmapImage.getHeight(), bitmapImage.getWidth(), CvType.CV_8UC3, new Scalar(255, 255, 255));
        Utils.bitmapToMat(bitmapImage, matImage);

        // Set ImageView
        imageView = findViewById(R.id.imageView);
        showImage(imageView, matImage);

        // ============================================================================================
        // Set Click Listener
        // ============================================================================================
        // Music Play
        Button button01 = findViewById(R.id.button01);
        button01.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                preprocess1(matImage.getNativeObjAddr());
                preprocess2(matImage.getNativeObjAddr());
                staffsInfo = preprocess3(matImage.getNativeObjAddr());
                staffsInfo = preprocess4(matImage.getNativeObjAddr(), staffsInfo);
                notesInfo = process(matImage.getNativeObjAddr(), staffsInfo);
                showImage(imageView, matImage);
                Thread thread = new socketThread(key, notesInfo);
                thread.start();
            }
        });


        // Default
        Button button02 = findViewById(R.id.button02);
        button02.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                matImage = new Mat(bitmapImage.getHeight(), bitmapImage.getWidth(), CvType.CV_8UC3, new Scalar(255, 255, 255));
                Utils.bitmapToMat(bitmapImage, matImage);
                showImage(imageView, matImage);
            }
        });

        // Previous Image
        Button button04 = findViewById(R.id.button04);
        button04.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (arrayIndex <= 0) {
                    arrayIndex = 0;
                } else {
                    arrayIndex--;
                }
                bitmapImage = BitmapFactory.decodeResource(getResources(), sheetMusics.getResourceId(arrayIndex, -1));
                matImage = new Mat(bitmapImage.getHeight(), bitmapImage.getWidth(), CvType.CV_8UC3, new Scalar(255, 255, 255));
                Utils.bitmapToMat(bitmapImage, matImage);
                showImage(imageView, matImage);
            }
        });

        // Next Image
        Button button05 = findViewById(R.id.button05);
        button05.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (arrayIndex >= 26) {
                    arrayIndex = 26;
                } else {
                    arrayIndex++;
                }
                bitmapImage = BitmapFactory.decodeResource(getResources(), sheetMusics.getResourceId(arrayIndex, -1));
                matImage = new Mat(bitmapImage.getHeight(), bitmapImage.getWidth(), CvType.CV_8UC3, new Scalar(255, 255, 255));
                Utils.bitmapToMat(bitmapImage, matImage);
                showImage(imageView, matImage);
            }
        });

        // Practice Screen
        Button button06 = findViewById(R.id.button06);
        button06.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(PracticeActivity.this, TestActivity.class);
                startActivity(intent);
                finish();
            }
        });
    }

    // ============================================================================================
    // Create Custom Functions
    // ============================================================================================
    // Show an Image on the Screen
    private void showImage(ImageView imageView, Mat mat) {
        Bitmap bitmapOutput = Bitmap.createBitmap(mat.cols(), mat.rows(), Bitmap.Config.RGB_565);
        Utils.matToBitmap(mat, bitmapOutput);
        imageView.setImageBitmap(bitmapOutput);
    }
}