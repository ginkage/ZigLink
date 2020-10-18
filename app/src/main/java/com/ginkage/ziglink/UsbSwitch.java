package com.ginkage.ziglink;

import android.content.Context;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.util.Log;
import com.hoho.android.usbserial.driver.CdcAcmSerialDriver;
import com.hoho.android.usbserial.driver.ProbeTable;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import java.io.IOException;
import java.util.List;

public class UsbSwitch {
    private final ProbeTable customTable =
            new ProbeTable().addProduct(0x0451, 0x16A8, CdcAcmSerialDriver.class);
    private final UsbSerialProber prober = new UsbSerialProber(customTable);
    private UsbSerialPort port;

    public void connect(Context context) throws IOException {
        // Find all available drivers from attached devices.
        UsbManager usbManager = context.getSystemService(UsbManager.class);
        List<UsbSerialDriver> availableDrivers = prober.findAllDrivers(usbManager);
        if (availableDrivers.isEmpty()) {
            return;
        }

        // Open a connection to the first available driver.
        UsbSerialDriver driver = availableDrivers.get(0);
        UsbDeviceConnection connection = usbManager.openDevice(driver.getDevice());
        if (connection == null) {
            // add UsbManager.requestPermission(driver.getDevice(), ..) handling here
            return;
        }

        port = driver.getPorts().get(0); // Most devices have just one port (port 0)
        port.open(connection);
        port.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);
    }

    public void turnOn() {
        try {
            port.write(new byte[] { '1' }, 1000);
        } catch (IOException e) {
            Log.e("UsbSwitch", "Failed to send ON command", e);
        }
    }

    public void turnOff() {
        try {
            port.write(new byte[] { '0' }, 1000);
        } catch (IOException e) {
            Log.e("UsbSwitch", "Failed to send OFF command", e);
        }
    }

    public void ddisconnect() {
        try {
            port.close();
        } catch (IOException e) {
            Log.e("UsbSwitch", "Failed to disconnect", e);
        }
    }
}
