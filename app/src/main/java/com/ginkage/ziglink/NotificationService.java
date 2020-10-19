package com.ginkage.ziglink;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;

import androidx.annotation.Nullable;

/**
 * Service that shows an ongoing notification if there is a device connected. Android P requires
 * that for HID profile app to remain registered, it should be "visible", i.e. have a running
 * activity or a foreground service. We don't want to become unregistered with an active connection,
 * so we'll display a notification to stay "visible".
 */
public class NotificationService extends Service {
    private static final int ONGOING_NOTIFICATION_ID = 0x1111;
    private static final String NOTIFICATION_CHANNEL_ID = "ZigLinkNotify";
    private static final String NOTIFICATION_CHANNEL_NAME = "All notifications";
    private static final String ACTION_START = "com.ginkage.ziglink.START_NOTIFY";
    private static final String ACTION_STOP = "com.ginkage.ziglink.STOP_NOTIFY";

    private boolean isForeground;
    private NotificationManager notificationManager;
    private UsbSwitch usbSwitch;

    private final NotificationChannel notificationChannel =
            new NotificationChannel(
                    NOTIFICATION_CHANNEL_ID,
                    NOTIFICATION_CHANNEL_NAME,
                    NotificationManager.IMPORTANCE_LOW);

    /** Interface for binding the service to an activity. */
    class LocalBinder extends Binder {
        /**
         * Get the service instance from the Binder proxy.
         *
         * @return Service instance.
         */
        NotificationService getService() {
            return NotificationService.this;
        }
    }

    private final IBinder binder = new LocalBinder();

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (ACTION_START.equals(intent.getAction())) {
            updateNotification();
        } else if (ACTION_STOP.equals(intent.getAction())) {
            stopNotification();
            return START_NOT_STICKY;
        }

        return START_STICKY;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        usbSwitch = new UsbSwitch();

        notificationManager = getSystemService(NotificationManager.class);
        if (notificationManager != null) {
            notificationManager.createNotificationChannel(notificationChannel);
        }
    }

    @Override
    public void onDestroy() {
        stopNotification();
        super.onDestroy();
    }

    /**
     * Returns an intent that should be passed to startService() to create or update the ongoing
     * notification.
     */
    public static Intent getIntent(Context context) {
        return new Intent(ACTION_START).setClass(context, NotificationService.class);
    }

    private void updateNotification() {
        Notification notification = buildNotification("USB dongle connected");

        if (isForeground) {
            if (notificationManager != null) {
                notificationManager.notify(ONGOING_NOTIFICATION_ID, notification);
            }
        } else {
            startForeground(ONGOING_NOTIFICATION_ID, notification);
            connect();
            isForeground = true;
        }
    }

    private void stopNotification() {
        if (isForeground) {
            stopForeground(true);
            disconnect();
            isForeground = false;
        }
    }

    private Notification buildNotification(String text) {
        Intent intent =
                new Intent(ACTION_STOP).setClass(this, NotificationService.class);

        PendingIntent pendingIntent = PendingIntent.getService(this, 0, intent, 0);

        return new Notification.Builder(this, NOTIFICATION_CHANNEL_ID)
                .setLocalOnly(true)
                .setOngoing(true)
                .setSmallIcon(getApplicationInfo().icon)
                .setContentTitle(getString(R.string.app_name))
                .setContentText(text)
                .setContentIntent(pendingIntent)
                .build();
    }

    public void connect() {
        usbSwitch.connect(this);
    }

    public void disconnect() {
        usbSwitch.disconnect();
    }

    public void turnOn() {
        usbSwitch.turnOn();
    }

    public void turnOff() {
        usbSwitch.turnOff();
    }
}
