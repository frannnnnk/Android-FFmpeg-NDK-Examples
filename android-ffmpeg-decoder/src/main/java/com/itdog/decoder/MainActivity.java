package com.itdog.decoder;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Created by tianbei on 2017/3/22.
 */

public class MainActivity extends AppCompatActivity {

    private static final String MP4_FILE = "flower.mp4";

    private static final String YUV_FILE = "flower.yuv";

    private Button convert;

    private static final int MSG_COPY_SUCCESS = 1;
    private static final int MSG_COPY_FAIL = 2;
    private static final int MSG_DECODE_SUCCESS = 3;
    private static final int MSG_DECODE_FAIL = 4;

    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_COPY_SUCCESS:
                    Toast.makeText(MainActivity.this, "文件拷贝成功!", Toast.LENGTH_LONG).show();
                    break;
                case MSG_COPY_FAIL:
                    Toast.makeText(MainActivity.this, "文件拷贝失败!", Toast.LENGTH_LONG).show();
                    break;
                case MSG_DECODE_SUCCESS:
                    long fileSize = new File(getFilesDir(), YUV_FILE).length();
                    Toast.makeText(MainActivity.this, "文件解码成功! size = " + fileSize, Toast.LENGTH_LONG).show();
                    break;
                case MSG_DECODE_FAIL:
                    Toast.makeText(MainActivity.this, "文件解码失败!", Toast.LENGTH_LONG).show();
                    break;
            }
        }
    };

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        initView();

        if (!new File(getFilesDir(), MP4_FILE).exists()) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    try {
                        copyFile();
                        handler.sendEmptyMessage(MSG_COPY_SUCCESS);
                    } catch (IOException e) {
                        e.printStackTrace();
                        handler.sendEmptyMessage(MSG_COPY_FAIL);
                    }
                }
            }).start();
        }

    }

    private void initView() {

        convert = (Button) findViewById(R.id.btn_work);

        convert.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        int ret = decodeVideo();
                        if(ret == 0) {
                            handler.sendEmptyMessage(MSG_DECODE_SUCCESS);
                        } else {
                            handler.sendEmptyMessage(MSG_DECODE_FAIL);
                        }
                    }
                }).start();
            }
        });
    }

    private void copyFile() throws IOException {
        InputStream fin = this.getAssets().open(MP4_FILE);
        FileOutputStream fout = new FileOutputStream(new File(getFilesDir(), MP4_FILE));
        byte[] buf = new byte[1024];
        int readLen = 0;
        while ((readLen = fin.read(buf, 0, buf.length)) > 0) {
            fout.write(buf, 0, readLen);
        }
        fout.flush();
        fout.close();
        fin.close();
    }

    private int decodeVideo() {

        String inputPath = new File(getFilesDir(), MP4_FILE).getPath();
        String outputPath = new File(getFilesDir(), YUV_FILE).getPath();

        return NativeBridge.decode(inputPath, outputPath);

    }

}
