package de.unidue.tagrecognition;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;

public class SettingsActivity extends Activity {

	private Button _btn_accept;
	private Button _btn_cancel;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// Hide the window title.
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setContentView(R.layout.setting);
		
		_btn_accept = (Button) findViewById(R.id.btn_accept);
		_btn_cancel = (Button) findViewById(R.id.btn_cancel);

		// actions for radar button
		_btn_accept.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				//functionSearch();
			}
		});
		
		_btn_cancel.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				SettingsActivity.this.finish();
			}
		});
		
		
	}

}
