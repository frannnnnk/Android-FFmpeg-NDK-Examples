package com.itdog.recordvideo;

import android.hardware.Camera;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.IOException;

/**
 * Created by tianbei on 2017/3/27.
 */

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback {

    private static final String TAG = "(>_<)";

    SurfaceView surfaceView;
    SurfaceHolder surfaceHolder;
    Camera camera;

    TextView tip;
    Button stop;
    boolean recording = true;
    String fileName = null;

    // 音频部分
    private int frequence = 44100;

    // CHANNEL_IN_MONO 单声道
    private int channelConfig = AudioFormat.CHANNEL_IN_STEREO;
    private int audioEncoding = AudioFormat.ENCODING_PCM_16BIT;


    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        tip = (TextView) findViewById(R.id.tip);
        stop = (Button) findViewById(R.id.stop);

        fileName = Environment.getExternalStorageDirectory().getAbsolutePath() + "/b/" + "itdog.flv";
        Log.d(TAG, "onCreate: fileName = " + fileName);

        if (NativeBridge.init(fileName.getBytes()) == -1) {
            tip.setText("无法连接网络");
            return;
        }

        stop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                recording = false;
                NativeBridge.flush();
                NativeBridge.close();
            }
        });

        surfaceView = (SurfaceView) findViewById(R.id.view);
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(this);
        surfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        // 开始录制
        RecordTask task = new RecordTask();
        task.execute();
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD) {
            camera = Camera.open(0);
        } else {
            camera = Camera.open();
        }

        try {
            camera.setPreviewDisplay(holder);
            surfaceHolder = holder;
        } catch (IOException ex) {
            if (camera != null) {
                camera.release();
                camera = null;
            }
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Camera.Parameters p = camera.getParameters();
        p.setPreviewSize(600, 800);

        camera.setPreviewCallback(new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
                Log.d(TAG, "onPreviewFrame: onPreviewFrame");
                if (recording) {
                    NativeBridge.start(data);
                }
            }
        });

        camera.setParameters(p);
        try {
            camera.setPreviewDisplay(surfaceHolder);
        } catch (Exception ex) {

        }
        camera.startPreview();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (camera != null) {
            camera.setPreviewCallback(null);
            camera.stopPreview();
            camera.release();
            camera = null;
            surfaceView = null;
            surfaceHolder = null;
        }
    }

    class RecordTask extends AsyncTask<Void, Integer, Void> {

        @Override
        protected Void doInBackground(Void... params) {
            int bufferSize = AudioRecord.getMinBufferSize(frequence, channelConfig, audioEncoding);
            AudioRecord record = new AudioRecord(MediaRecorder.AudioSource.MIC, frequence, channelConfig, audioEncoding, bufferSize);

            // 开始录制
            record.startRecording();

            byte[] bb = new byte[bufferSize];
            while (recording) {
                int size = record.read(bb, 0, bufferSize);
                if (size > 0) {
                    NativeBridge.startAudio(bb, size);
                }
            }

            if (record.getRecordingState() == AudioRecord.RECORDSTATE_RECORDING) {
                record.stop();
            }

            record.release();
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);

            tip.setText("已经停止了");
        }
    }

}
