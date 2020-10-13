package com.example.practiceopencv;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.ImageView;

import androidx.fragment.app.Fragment;

import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Scalar;

public class SheetFragment extends Fragment {
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

    // ============================================================================================
    // Create Global Variables
    // ============================================================================================
    private TypedArray sheetMusics;
    private int arrayIndex = 0;
    private int temp = 0;

    private Mat matImage;
    private Bitmap bitmapImage;
    private ImageView imageView;

    private int[] staffsInfo;
    private double avgDistance = 0.00;
    private int key;
    private int[][] notesInfo;

    private View view;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.fragment_sheet, container, false);

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
        imageView = view.findViewById(R.id.imageView);
        showImage(imageView, matImage);

        // ============================================================================================
        // Set Click Listener
        // ============================================================================================
        // Previous Image
        ImageButton button01 = view.findViewById(R.id.button01);
        button01.setOnClickListener(new View.OnClickListener() {
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
        ImageButton button02 = view.findViewById(R.id.button02);
        button02.setOnClickListener(new View.OnClickListener() {
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

        // Music Arrangement
        ImageButton button03 = view.findViewById(R.id.button03);
        button03.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ProgressDialog progressDialog = new ProgressDialog(getContext());
                progressDialog.setMessage("Creating...");
                progressDialog.setCancelable(false);
                progressDialog.setProgressStyle(android.R.style.Widget_ProgressBar_Horizontal);
                progressDialog.show();
                preprocess1(matImage.getNativeObjAddr());
                preprocess2(matImage.getNativeObjAddr());
                staffsInfo = preprocess3(matImage.getNativeObjAddr());
                staffsInfo = preprocess4(matImage.getNativeObjAddr(), staffsInfo);
                notesInfo = process(matImage.getNativeObjAddr(), staffsInfo);
                showImage(imageView, matImage);
                int length = notesInfo.length;
                int[] notes = new int[length];
                int[] beats = new int[length];
                for (int i = 0; i < length; i++) {
                    notes[i] = notesInfo[i][0];
                    beats[i] = notesInfo[i][1];
                }
                progressDialog.dismiss();
                Intent intent = new Intent(getActivity(), CreateActivity.class);
                intent.putExtra("key", key);
                intent.putExtra("notes", notes);
                intent.putExtra("beats", beats);
                startActivity(intent);
            }
        });

//        ImageButton button03 = view.findViewById(R.id.button03);
//        button03.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                if(temp == 0){
//                    preprocess1(matImage.getNativeObjAddr());
//                    showImage(imageView, matImage);
//                }
//                else if(temp == 1){
//                    preprocess2(matImage.getNativeObjAddr());
//                    showImage(imageView, matImage);
//                }else if(temp == 2){
//                    staffsInfo = preprocess3(matImage.getNativeObjAddr());
//                    showImage(imageView, matImage);
//                }else if(temp == 3){
//                    staffsInfo = preprocess4(matImage.getNativeObjAddr(), staffsInfo);
//                    showImage(imageView, matImage);
//                }else if(temp == 4){
//                    notesInfo = process(matImage.getNativeObjAddr(), staffsInfo);
//                    showImage(imageView, matImage);
//                }
//                temp++;
//            }
//        });

        return view;
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
