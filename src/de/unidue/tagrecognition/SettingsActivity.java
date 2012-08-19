package de.unidue.tagrecognition;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

/**
 * @file SettingsActivity.java
 * @brief Android activity to manage configuration variables.
 * @author Antonio Manuel Gutierrez Martinez
 * @version 1.0
 */
public class SettingsActivity extends Activity {
	public static final String SettingActivitiy_MSG = "setting_msg";
	public static final String SettingActivitiy_DATA = "CHANGES";

	/** Pattern to check IP format. */
	private static final String IPADDRESS_PATTERN = "^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
			+ "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
			+ "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
			+ "([01]?\\d\\d?|2[0-4]\\d|25[0-5])$";

	/** Interface elements */
	private Button _btn_accept;
	private Button _btn_cancel;

	private EditText _txt_template;
	private EditText _txt_rows;
	private EditText _txt_cols;
	private EditText _txt_height;
	private EditText _txt_width;
	private EditText _txt_border;
	private EditText _txt_app_port;
	private EditText _txt_desk_port;
	private EditText _txt_ip;
	private CheckBox _ck_debug;

	/** Number of rows in tags */
	private int _rows;
	/** Number of cols in tags */
	private int _cols;
	/** Tag width, in cm */
	private float _tag_width;
	/** Tag height, in cm */
	private float _tag_height;
	/** Tag border size, in cm */
	private float _tag_border;
	/** Holds debug mode */
	private boolean _debugMode;
	/** Tag template to calibrate colours thresholds */
	private String _template;
	/** Desktop server IP */
	private String _desk_IP;
	/** Desktop server Port */
	private int _desk_Port;
	/** Port for android application server */
	private int _app_Port;

	@Override
	/**
	 *  Actions made on create activity
	 */
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// Hide the window title.
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setContentView(R.layout.setting);

		// Get fields references
		_txt_template = (EditText) findViewById(R.id.editText_template);
		_txt_rows = (EditText) findViewById(R.id.editText_rows);
		_txt_cols = (EditText) findViewById(R.id.editText_cols);
		_txt_height = (EditText) findViewById(R.id.editText_height);
		_txt_width = (EditText) findViewById(R.id.editText_width);
		_txt_border = (EditText) findViewById(R.id.editText_border);
		_txt_app_port = (EditText) findViewById(R.id.editText_app_port);
		_txt_desk_port = (EditText) findViewById(R.id.editText_desk_port);
		_txt_ip = (EditText) findViewById(R.id.editText_ip);
		_ck_debug = (CheckBox) findViewById(R.id.checkBox);
		_btn_accept = (Button) findViewById(R.id.btn_accept);
		_btn_cancel = (Button) findViewById(R.id.btn_cancel);

		// Set fields values
		setFieldsValues();

		// actions for radar button
		_btn_accept.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// get values
				getFieldsValues();

				// check values
				String msg = checkValues();

				if (msg.equals("")) {
					// Set new preferences
					setPrefrencesValues();

					// Communicate change
					Intent i = new Intent();
					Bundle bundle = new Bundle();
					bundle.putBoolean(SettingActivitiy_DATA, true);
					i.putExtras(bundle);
					i.setAction(SettingsActivity.SettingActivitiy_MSG);
					SettingsActivity.this.sendBroadcast(i);
					SettingsActivity.this.finish();

				} else
					// Show error
					Toast.makeText(SettingsActivity.this,
							"Found following errors: " + msg, Toast.LENGTH_LONG)
							.show();

			}
		});

		_btn_cancel.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				Intent i = new Intent();
				Bundle bundle = new Bundle();
				bundle.putBoolean(SettingActivitiy_DATA, false);
				i.putExtras(bundle);
				i.setAction(SettingsActivity.SettingActivitiy_MSG);
				SettingsActivity.this.sendBroadcast(i);
				SettingsActivity.this.finish();
			}
		});

	}

	/**
	 * Fill the elements of interface with values from SharedPreferences
	 */
	private void setFieldsValues() {
		// Restore preferences
		SharedPreferences settings = getSharedPreferences(
				TagRecognitionActivity.PREFS_NAME, Activity.MODE_PRIVATE);
		_txt_template.setText(settings.getString("templateTag", "12321113"));
		_txt_rows.setText(((Integer) settings.getInt("rows", 2)).toString());
		_txt_cols.setText(((Integer) settings.getInt("cols", 4)).toString());
		_txt_height.setText(((Float) settings.getFloat("tag_height", 4.5f))
				.toString());
		_txt_width.setText(((Float) settings.getFloat("tag_width", 8.0f))
				.toString());
		_txt_border.setText(((Float) settings.getFloat("tag_border", 1.0f))
				.toString());

		_txt_app_port.setText(((Integer) settings.getInt("app_Port", 8000))
				.toString());
		_txt_desk_port.setText(((Integer) settings.getInt("desk_Port", 8080))
				.toString());
		_txt_ip.setText(settings.getString("desk_IP", "192.168.137.1"));

		if (settings.getBoolean("debugMode", false))
			_ck_debug.setChecked(true);
		else
			_ck_debug.setChecked(false);
	}

	/**
	 * Fill the internal variable values with the values from the interface
	 * elements.
	 */
	private void getFieldsValues() {
		_template = _txt_template.getText().toString();
		_rows = Integer.parseInt(_txt_rows.getText().toString());
		_cols = Integer.parseInt(_txt_cols.getText().toString());
		_tag_height = Float.parseFloat(_txt_height.getText().toString());
		_tag_width = Float.parseFloat(_txt_width.getText().toString());
		_tag_border = Float.parseFloat(_txt_border.getText().toString());

		_app_Port = Integer.parseInt(_txt_app_port.getText().toString());
		_desk_Port = Integer.parseInt(_txt_desk_port.getText().toString());
		_desk_IP = _txt_ip.getText().toString();

		_debugMode = _ck_debug.isChecked();
	}

	/**
	 * Check if the values of variables are correct.
	 * 
	 * @return a string indicating the fail found or an empty string otherwise.
	 */
	private String checkValues() {
		String errors = "";
		if (_rows <= 0) {
			errors = "Rows must be > 0";
		} else if (_cols <= 0) {
			errors = "Cols must be > 0";
		} else if (_tag_height <= 0) {
			errors = "Height must be > 0";
		} else if (_tag_width <= 0) {
			errors = "Width must be > 0";
		} else if (_tag_border <= 0) {
			errors = "Border size must be > 0";
		} else if (_app_Port < 0 || _app_Port >= 65535) {
			errors = "App Port invalid";
		} else if (_desk_Port < 0 || _desk_Port >= 65535) {
			errors = "Desktop Port invalid";
		} else if (_app_Port < 0 || _app_Port >= 65535) {
			errors = "App Port invalid";
		} else if (!_desk_IP.matches(IPADDRESS_PATTERN)) {
			errors = "Ip format invalid";
		} else if (_debugMode) {
			if (!createDebugDirectory())
				errors = "Debug directory cannot be created";
		}

		return errors;
	}

	/**
	 * Store the values of internal variables in SharedPreferences system of
	 * Android.
	 */
	private void setPrefrencesValues() {
		// Get editor
		SharedPreferences settings = getSharedPreferences(
				TagRecognitionActivity.PREFS_NAME, Activity.MODE_PRIVATE);
		SharedPreferences.Editor editor = settings.edit();

		editor.putInt("rows", _rows);
		editor.putInt("cols", _cols);
		editor.putFloat("tag_width", _tag_width);
		editor.putFloat("tag_height", _tag_height);
		editor.putFloat("tag_border", _tag_border);
		editor.putBoolean("debugMode", _debugMode);
		editor.putString("templateTag", _template);
		editor.putString("desk_IP", _desk_IP);
		editor.putInt("desk_Port", _desk_Port);
		editor.putInt("app_Port", _app_Port);

		// Commit the edits!
		editor.commit();
	}

	/**
	 * Create a debug directory where the image will be placed if it is not
	 * already created.
	 * 
	 * @return Success of operation
	 */
	private boolean createDebugDirectory() {
		boolean success = true;
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state)) {
			// Make the path to a directory with name MyCameraApp in the
			// directory
			// of pictures
			File mediaStorageDir = new File(
					Environment
							.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES),
					"TagRecognizerApp");

			// Create the storage directory if it does not exist
			if (!mediaStorageDir.exists()) {
				if (!mediaStorageDir.mkdirs()) {
					success = false;
				}
			}
		} else {
			success = false;
		}

		return success;
	}
}
