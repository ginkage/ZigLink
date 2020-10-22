package com.ginkage.ziglink;

import android.content.Intent;
import android.net.Uri;
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
        final String query = Intent.ACTION_VIEW.equals(action) ? intent.getData().getQuery() : null;

        connection = new NotificationServiceConnection(this, service -> {
            if ("on".equals(query)) {
                connection.turnOn();
                finish();
            } else if ("off".equals(query)) {
                connection.turnOff();
                finish();
            }
        });

        if (Intent.ACTION_MAIN.equals(action) || query != null) {
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