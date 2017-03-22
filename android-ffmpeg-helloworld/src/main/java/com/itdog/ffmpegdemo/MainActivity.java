package com.itdog.ffmpegdemo;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        final TextView tv = (TextView) findViewById(R.id.info);

        Button urlprotocolinfo = (Button) findViewById(R.id.urlprotocolinfo);
        urlprotocolinfo.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                tv.setText(NativeBridge.urlprotocolinfo());
            }
        });

        Button avformatinfo = (Button) findViewById(R.id.avformatinfo);
        avformatinfo.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                tv.setText(NativeBridge.avformatinfo());
            }
        });

        Button avcodecinfo = (Button) findViewById(R.id.avcodecinfo);
        avcodecinfo.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                tv.setText(NativeBridge.avcodecinfo());
            }
        });

        Button avfilterinfo = (Button) findViewById(R.id.avfilterinfo);
        avfilterinfo.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                tv.setText(NativeBridge.avfilterinfo());
            }
        });

        Button configurationinfo = (Button) findViewById(R.id.configurationinfo);
        configurationinfo.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                tv.setText(NativeBridge.configurationinfo());
            }
        });
    }



}
