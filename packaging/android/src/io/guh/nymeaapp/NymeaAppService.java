package io.guh.nymeaapp;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import org.qtproject.qt5.android.bindings.QtService;

public class NymeaAppService extends QtService
{
    public static final String BROADCAST_STATE_CHANGE = "io.guh.nymeaapp.NymeaAppService.broadcast.stateChanged";

    private static final String TAG = "nymea-app: NymeaAppService";

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "Creating Service");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "Destroying Service");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int ret = super.onStartCommand(intent, flags, startId);

        // Do some work

        Log.d(TAG, "*************** Service started");

        return ret;
    }

    public void sendBroadcast(String thingId, String stateTypeId, String value) {
//        String name = new String(intent.getByteArrayExtra("name"));
        Intent sendToUiIntent = new Intent();
        sendToUiIntent.setAction(BROADCAST_STATE_CHANGE);
        sendToUiIntent.putExtra("name", "io.guh.nymeaapp.NymeaAppService");
        sendToUiIntent.putExtra("thingId", thingId);
        sendToUiIntent.putExtra("stateTypeId", stateTypeId);
        sendToUiIntent.putExtra("value", value);
        Log.d(TAG, "Service sending broadcast");
        sendBroadcast(sendToUiIntent);
    }
}
