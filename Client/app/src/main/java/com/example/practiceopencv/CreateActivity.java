package com.example.practiceopencv;

import android.app.ProgressDialog;
import android.content.Intent;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class CreateActivity extends AppCompatActivity {

    private int key;
    private int[] notes;
    private int[] beats;
    private int bpmValue;
    private int instrumentValue;
    private int beatValue;
    private int melody;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_create);

        Intent intent = getIntent();
        key = intent.getExtras().getInt("key");
        notes = intent.getExtras().getIntArray("notes");
        beats = intent.getExtras().getIntArray("beats");
        int[][] notesInfo = new int [notes.length][2];
        for(int i = 0; i < notes.length; i ++){
            notesInfo[i][0] = notes[i];
            notesInfo[i][1] = beats[i];
        }

        Spinner bpm = findViewById(R.id.spinner_bpm);
        ArrayAdapter bpms = ArrayAdapter.createFromResource(this, R.array.bpm, android.R.layout.simple_spinner_item);
        bpms.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        bpm.setAdapter(bpms);
        String[] bpmArray = getResources().getStringArray(R.array.bpm);
        bpm.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                bpmValue = Integer.parseInt(bpmArray[position]);
            }
            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        Spinner instrument = findViewById(R.id.spinner_instrument);
        ArrayAdapter instruments = ArrayAdapter.createFromResource(this, R.array.instrument, android.R.layout.simple_spinner_item);
        instruments.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        instrument.setAdapter(instruments);
        String[] instrumentArray = getResources().getStringArray(R.array.instrument);
        instrument.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String string = instrumentArray[position];
                if (string == "piano") {
                    instrumentValue = 1;
                } else if (string == "electric piano") {
                    instrumentValue = 5;
                } else if (string == "violin") {
                    instrumentValue = 41;
                } else if (string == "flute") {
                    instrumentValue = 74;
                } else {
                    instrumentValue = 23;
                }
            }
            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        Spinner beat = findViewById(R.id.spinner_beat);
        ArrayAdapter beats = ArrayAdapter.createFromResource(this, R.array.beat, android.R.layout.simple_spinner_item);
        beats.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        beat.setAdapter(beats);
        String[] beatArray = getResources().getStringArray(R.array.beat);
        beat.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                beatValue = Integer.parseInt(beatArray[position]);
            }
            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        RadioGroup radioGroup = findViewById(R.id.group_onoff);
        radioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                String string = ((RadioButton)findViewById(radioGroup.getCheckedRadioButtonId())).getText().toString();
                if (string == "on") {
                    melody = 0;
                } else {
                    melody = 1;
                }
            }
        });
        RadioButton radioButton = findViewById(R.id.radio_off);
        radioGroup.check(radioButton.getId());

        Button button01 = findViewById(R.id.button01);
        button01.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ProgressDialog progressDialog = new ProgressDialog(CreateActivity.this);
                progressDialog.setMessage("Creating...");
                // progressDialog.setCancelable(false);
                progressDialog.setProgressStyle(android.R.style.Widget_ProgressBar_Horizontal);
                progressDialog.show();
                Thread thread = new socketThread(CreateActivity.this, progressDialog, key, notesInfo, bpmValue, instrumentValue, beatValue, melody, 0);
                thread.start();
            }
        });

        Button button02 = findViewById(R.id.button02);
        button02.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ProgressDialog progressDialog = new ProgressDialog(CreateActivity.this);
                progressDialog.setMessage("Creating...");
                // progressDialog.setCancelable(false);
                progressDialog.setProgressStyle(android.R.style.Widget_ProgressBar_Horizontal);
                progressDialog.show();
                Thread thread = new socketThread(CreateActivity.this, progressDialog, key, notesInfo, bpmValue, instrumentValue, beatValue, melody, 1);
                thread.start();
            }
        });
    }
}
