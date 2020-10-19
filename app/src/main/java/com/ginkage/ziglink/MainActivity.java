package com.ginkage.ziglink;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    private static final String ACTION_TOGGLE = "com.ginkage.ziglink.TOGGLE_LIGHT";
    private NotificationServiceConnection connection;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        startForegroundService(NotificationService.getIntent(this));

        final Intent intent = getIntent();
        final String action = intent.getAction();

        connection = new NotificationServiceConnection(this, service -> {
            if (ACTION_TOGGLE.equals(action)) {
                String state = intent.getStringExtra("state");
                if ("LIGHTS_ON".equals(state)) {
                    connection.turnOn();
                } else if ("LIGHTS_OFF".equals(state)) {
                    connection.turnOff();
                }
            }
        });

        if (Intent.ACTION_MAIN.equals(action) || ACTION_TOGGLE.equals(action)) {
            setContentView(R.layout.activity_main);
        } else {
            finish();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        connection.bind();

        Button btnOn = findViewById(R.id.btnOn);
        Button btnOff = findViewById(R.id.btnOff);
        btnOn.setOnClickListener(v -> connection.turnOn());
        btnOff.setOnClickListener(v -> connection.turnOff());
    }

    @Override
    protected void onPause() {
        super.onPause();
        connection.unbind();
    }
}