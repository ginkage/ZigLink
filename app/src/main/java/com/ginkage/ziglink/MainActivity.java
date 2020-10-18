package com.ginkage.ziglink;

import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    private final UsbSwitch usbSwitch = new UsbSwitch();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    @Override
    protected void onResume() {
        super.onResume();

        try {
            usbSwitch.connect(this);
            TextView txtState = findViewById(R.id.txtState);
            txtState.setText("Connected");
        } catch (IOException e) {
            e.printStackTrace();
        }

        Button btnOn = findViewById(R.id.btnOn);
        Button btnOff = findViewById(R.id.btnOff);
        btnOn.setOnClickListener(v -> usbSwitch.turnOn());
        btnOff.setOnClickListener(v -> usbSwitch.turnOff());
    }

    @Override
    protected void onPause() {
        super.onPause();

        usbSwitch.ddisconnect();
    }
}