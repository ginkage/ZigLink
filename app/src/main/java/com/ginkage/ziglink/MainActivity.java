package com.ginkage.ziglink;

import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    private NotificationServiceConnection connection;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        startForegroundService(NotificationService.getIntent(this));
        connection = new NotificationServiceConnection(this, service -> {});

        if ("android.hardware.usb.action.USB_DEVICE_ATTACHED".equals(getIntent().getAction())) {
            finish();
        } else {
            setContentView(R.layout.activity_main);
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