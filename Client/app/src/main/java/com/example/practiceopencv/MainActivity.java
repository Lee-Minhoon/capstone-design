package com.example.practiceopencv;

import android.os.Bundle;
import android.view.MenuItem;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentTransaction;

import com.google.android.material.bottomnavigation.BottomNavigationView;

public class MainActivity extends AppCompatActivity {
    private BottomNavigationView bottomNavigationView;
    private FragmentManager fragmentManager;
    private FragmentTransaction fragmentTransaction;
    private SheetFragment sheetFragment;
    private PlayerFragment playerFragment;
    private InfoFragment infoFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        sheetFragment = new SheetFragment();
        playerFragment = new PlayerFragment();
        infoFragment = new InfoFragment();

        bottomNavigationView = findViewById(R.id.navigation);
        bottomNavigationView.setOnNavigationItemSelectedListener(new BottomNavigationView.OnNavigationItemSelectedListener() {
            @Override
            public boolean onNavigationItemSelected(@NonNull MenuItem menuItem) {
                switch (menuItem.getItemId()) {
                    case R.id.item01:
                        setFragment(1);
                        break;
                    case R.id.item02:
                        setFragment(2);
                        break;
                    case R.id.item03:
                        setFragment(3);
                        break;
                }
                return true;
            }
        });

        setFragment(1);
    }

    private void setFragment(int i) {
        fragmentManager = getSupportFragmentManager();
        fragmentTransaction = fragmentManager.beginTransaction();

        switch (i) {
            case 1:
                fragmentTransaction.replace(R.id.frame, sheetFragment);
                fragmentTransaction.commit();
                break;
            case 2:
                fragmentTransaction.replace(R.id.frame, playerFragment);
                fragmentTransaction.commit();
                break;
            case 3:
                fragmentTransaction.replace(R.id.frame, infoFragment);
                fragmentTransaction.commit();
                break;
        }
    }
}