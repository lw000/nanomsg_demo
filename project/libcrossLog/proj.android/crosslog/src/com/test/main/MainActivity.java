package com.test.main;

import java.io.File;

import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.appcompat.R;
import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

import com.cross.log.CrossLog;

@SuppressLint("NewApi") public class MainActivity extends AppCompatActivity {
	static {
		System.loadLibrary("hncrosslog");
	}

	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        String path = Environment.getExternalStorageDirectory().getAbsoluteFile() + File.separator;
        path += "hnlog/";
        Log.d("crosslog", path);
        
        CrossLog.hnConfigFastLog(path);
        CrossLog.hnStartFastLog();
        
        for (int i = 0; i < 10; i++)
        {
        	CrossLog.hnPrintLogD(String.format(">>>>>>>>>> onCreate : %s", i));
        	CrossLog.hnPrintLogI(String.format(">>>>>>>>>> onCreate : %s", i));
        	CrossLog.hnPrintLogE(String.format(">>>>>>>>>> onCreate : %s", i));
        	CrossLog.hnPrintLogW(String.format(">>>>>>>>>> onCreate : %s", i));
        }
    }

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
