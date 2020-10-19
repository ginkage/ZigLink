package com.ginkage.ziglink;


import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;

import androidx.annotation.Nullable;

/** Helper class to manage the Sensor Service binding to an Activity. */
public class NotificationServiceConnection {

    /** Interface for listening to the service binding event. */
    public interface ConnectionListener {
        /**
         * Callback that receives the service instance.
         *
         * @param service Service instance.
         */
        void onServiceConnected(NotificationService service);
    }

    private final ServiceConnection connection =
            new ServiceConnection() {
                @Override
                public void onServiceConnected(ComponentName className, IBinder service) {
                    if (service != null) {
                        NotificationService.LocalBinder binder = (NotificationService.LocalBinder) service;
                        NotificationServiceConnection.this.service = binder.getService();
                        listener.onServiceConnected(NotificationServiceConnection.this.service);
                    }
                }

                @Override
                public void onServiceDisconnected(ComponentName className) {
                    service = null;
                }
            };

    private final ConnectionListener listener;
    private final Context context;

    @Nullable
    private NotificationService service;
    private boolean bound;

    /**
     * @param context Activity that the service is bound to.
     * @param listener Callback to receive the service instance.
     */
    public NotificationServiceConnection(Context context, ConnectionListener listener) {
        this.context = context;
        this.listener = listener;
    }

    /** Connects the activity to the service, starting it if required. */
    public void bind() {
        if (!bound) {
            Intent intent = new Intent(context, NotificationService.class);
            context.bindService(intent, connection, Context.BIND_AUTO_CREATE);
            bound = true;
        }
    }

    /** Unbinds the service from the activity. */
    public void unbind() {
        if (service != null) {
            service = null;
        }
        if (bound) {
            context.unbindService(connection);
            bound = false;
        }
    }

    public void turnOn() {
        if (bound && service != null) {
            service.turnOn();
        }
    }

    public void turnOff() {
        if (bound && service != null) {
            service.turnOff();
        }
    }
}
