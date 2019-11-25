package com.yyzhu.reinforceres;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;



public class MainActivity extends AppCompatActivity {

    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE" };

    // Used to load the 'native-lib' library on application startup.


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        verifyStoragePermissions(this);

        setContentView(R.layout.activity_main);
        Button btn = findViewById(R.id.button);
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                clickOpenAsset();
            }
        });

        Button btn2 = findViewById(R.id.button2);
        btn2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    clickCopyFile();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });


        Button btn3 = findViewById(R.id.button3);
        btn3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                openAsset();
            }
        });


    }


    public static void verifyStoragePermissions(Activity activity) {

        try {
            //检测是否有写的权限
            int permission = ActivityCompat.checkSelfPermission(activity,
                    "android.permission.WRITE_EXTERNAL_STORAGE");
            if (permission != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE,REQUEST_EXTERNAL_STORAGE);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */

    public void openAsset(){
        AssetManager am = getAssets();
        try {
            Log.d("reinforceRes", "invoke java openAsset");
            Log.d("reinforceRes", "AssetManager::open");
            InputStream input = am.open("reinforceRes.txt");
            byte[] b = new byte[12];
            input.read(b);
            input.close();
            Toast.makeText(this, new String(b,"utf-8"),Toast.LENGTH_SHORT).show();
            Log.d("reinforceRes", new String(b,"utf-8"));

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void clickCopyFile() throws IOException {
        Log.d("reinforceRes", "invoke clickCopyFile");
        String inputfilepath = getApplicationContext().getPackageResourcePath();
        String outputfilepath = "mybase.apk";
        boolean ret = copyFile(inputfilepath, outputfilepath);
        if(ret){
            Toast.makeText(this, "complete",Toast.LENGTH_SHORT).show();

            if(!isload){
                System.loadLibrary("native-lib");
                isload = true;
            }


        }

    }

    public native void clickOpenAsset();


    public boolean copyFile(String oldPath$Name, String newPath$Name) {
        try {
            //"/data/data/com.yyzhu.reinforceres/files/mybase.apk"
            //"/sdcard/mybase.apk"
            File newFile = new File("/data/data/com.yyzhu.reinforceres/files/mybase.apk");
            if(newFile.exists()){
                return true;
            }

            File oldFile = new File(oldPath$Name);
            if (!oldFile.exists()) {
                Log.e("--Method--", "copyFile:  oldFile not exist.");
                return false;
            } else if (!oldFile.isFile()) {
                Log.e("--Method--", "copyFile:  oldFile not file.");
                return false;
            } else if (!oldFile.canRead()) {
                Log.e("--Method--", "copyFile:  oldFile cannot read.");
                return false;
            }


            FileInputStream fileInputStream = new FileInputStream(oldPath$Name);    //读入原文件

            //采用这种方式在files文件夹下写入的数据在jni层竟然无权限访问，生成的文件的所有者也很奇怪为u0_a57
            FileOutputStream fileOutputStream = this.openFileOutput(newPath$Name, Context.MODE_PRIVATE);
            //FileOutputStream fileOutputStream = new FileOutputStream("/sdcard/mybase.apk");

            byte[] buffer = new byte[1024];
            int byteRead;
            while ((byteRead = fileInputStream.read(buffer)) != -1) {
                fileOutputStream.write(buffer, 0, byteRead);
            }
            fileInputStream.close();
            fileOutputStream.flush();
            fileOutputStream.close();

            File newFile2 = new File("/data/data/com.yyzhu.reinforceres/files/mybase.apk");
            newFile2.setReadable(true);

            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    private boolean isload = false;

}
