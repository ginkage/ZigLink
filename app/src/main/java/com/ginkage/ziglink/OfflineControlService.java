package com.ginkage.ziglink;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.service.controls.Control;
import android.service.controls.ControlsProviderService;
import android.service.controls.DeviceTypes;
import android.service.controls.actions.BooleanAction;
import android.service.controls.actions.ControlAction;
import android.service.controls.templates.ControlButton;
import android.service.controls.templates.ToggleTemplate;
import androidx.annotation.NonNull;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Flow;
import java.util.function.Consumer;
import org.reactivestreams.FlowAdapters;
import io.reactivex.Flowable;
import io.reactivex.processors.ReplayProcessor;

public class OfflineControlService extends ControlsProviderService {

    private static final String DEVICE_ID = "UsbSwitch";
    private ReplayProcessor<Control> updatePublisher;
    private NotificationServiceConnection connection;

    @Override
    public void onCreate() {
        super.onCreate();
        connection = new NotificationServiceConnection(this, service -> {});
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        connection.unbind();
    }

    @NonNull
    @Override
    public Flow.Publisher<Control> createPublisherForAllAvailable() {
        Context context = getBaseContext();
        Intent i = new Intent();
        PendingIntent pi = PendingIntent.getActivity(context, 1, i, PendingIntent.FLAG_UPDATE_CURRENT);
        List controls = new ArrayList<>();
        Control control = new Control.StatelessBuilder(DEVICE_ID, pi)
                // Required: The name of the control
                .setTitle("All lights")
                // Required: Usually the room where the control is located
                .setSubtitle("Living room")
                // Optional: Structure where the control is located, an example would be a house
                .setStructure("Osborne House")
                // Required: Type of device, i.e., thermostat, light, switch
                .setDeviceType(DeviceTypes.TYPE_LIGHT) // For example, DeviceTypes.TYPE_THERMOSTAT
                .build();
        controls.add(control);
        // Create more controls here if needed and add it to the ArrayList

        // Uses the RxJava 2 library
        return FlowAdapters.toFlowPublisher(Flowable.fromIterable(controls));
    }

    @NonNull
    @Override
    public Flow.Publisher<Control> createPublisherFor(@NonNull List<String> controlIds) {
        Context context = getBaseContext();
        /* Fill in details for the activity related to this device. On long press,
         * this Intent will be launched in a bottomsheet. Please design the activity
         * accordingly to fit a more limited space (about 2/3 screen height).
         */
        Intent i = new Intent();
        PendingIntent pi = PendingIntent.getActivity(context, 1, i, PendingIntent.FLAG_UPDATE_CURRENT);

        updatePublisher = ReplayProcessor.create();
        connection.bind();

        // For each controlId in controlIds
        if (controlIds.contains(DEVICE_ID)) {
            Control control = new Control.StatefulBuilder(DEVICE_ID, pi)
                    // Required: The name of the control
                    .setTitle("All lights")
                    // Required: Usually the room where the control is located
                    .setSubtitle("Living room")
                    // Optional: Structure where the control is located, an example would be a house
                    .setStructure("Osborne House")
                    // Required: Type of device, i.e., thermostat, light, switch
                    .setDeviceType(DeviceTypes.TYPE_LIGHT) // For example, DeviceTypes.TYPE_THERMOSTAT
                    // Required: Current status of the device
                    .setStatus(Control.STATUS_OK) // For example, Control.STATUS_OK
                    .setControlTemplate(
                            new ToggleTemplate("button",
                                    new ControlButton(false, "toggle")))
                    .setStatusText("Off")
                    .build();

            updatePublisher.onNext(control);
        }
        // Uses the Reactive Streams API
        return FlowAdapters.toFlowPublisher(updatePublisher);
    }

    @Override
    public void performControlAction(@NonNull String controlId,
                                     @NonNull ControlAction controlAction,
                                     @NonNull Consumer<Integer> consumer) {
        /* First, locate the control identified by the controlId. Once it is located, you can
         * interpret the action appropriately for that specific device. For instance, the following
         * assumes that the controlId is associated with a light, and the light can be turned on
         * or off.
         */
        if (controlAction instanceof BooleanAction) {
            // Inform SystemUI that the action has been received and is being processed
            consumer.accept(ControlAction.RESPONSE_OK);

            BooleanAction action = (BooleanAction) controlAction;
            boolean state = action.getNewState();
            // In this example, action.getNewState() will have the requested action: true for “On”,
            // false for “Off”.

            if (state) {
                connection.turnOn();
            } else {
                connection.turnOff();
            }

            Context context = getBaseContext();
            Intent i = new Intent();
            PendingIntent pi = PendingIntent.getActivity(context, 1, i, PendingIntent.FLAG_UPDATE_CURRENT);

            /* This is where application logic/network requests would be invoked to update the state of
             * the device.
             * After updating, the application should use the publisher to update SystemUI with the new
             * state.
             */
            Control control = new Control.StatefulBuilder(DEVICE_ID, pi)
                    // Required: The name of the control
                    .setTitle("All lights")
                    // Required: Usually the room where the control is located
                    .setSubtitle("Living room")
                    // Optional: Structure where the control is located, an example would be a house
                    .setStructure("Osborne House")
                    // Required: Type of device, i.e., thermostat, light, switch
                    .setDeviceType(DeviceTypes.TYPE_LIGHT) // For example, DeviceTypes.TYPE_THERMOSTAT
                    // Required: Current status of the device
                    .setStatus(Control.STATUS_OK) // For example, Control.STATUS_OK
                    .setControlTemplate(
                            new ToggleTemplate("button",
                                    new ControlButton(state, "toggle")))
                    .setStatusText(state ? "On" : "Off")
                    .build();

            // This is the publisher the application created during the call to createPublisherFor()
            updatePublisher.onNext(control);
        }
    }
}
