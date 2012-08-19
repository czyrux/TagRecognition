package de.unidue.tagrecognition;

import java.util.ArrayList;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ExecutionException;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.os.Bundle;

import android.graphics.Bitmap;
import android.hardware.Camera;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.RelativeLayout;
import android.widget.Toast;

/**
 * @file TagRecognitionActivity.java
 * @brief Main application activity
 * @author Antonio Manuel Gutierrez Martinez
 * @version 1.0
 */
public class TagRecognitionActivity extends Activity {
	private static final String TAG = "TagRecognizer";
	public static final String PREFS_NAME = "TagRecognizerPrefs";
	
	/** Camera preview */
	public Preview _mPreview;
	/** Hold action currently developed by application. */
	public Actions _currentAction;
	/** NDK instance */
	public NDKWrapper _ndk;
	/** Timer user to make period actions. */
	private Timer _timer;
	/** Help menu */
	private AlertDialog _helpMenu;
	/** Progress Dialog instance */
	private ProgressDialog _progress;
	/** Instance of BroadcastReceiver implementation */
	private mReceiver _notification;
	/** Handles PictureCallback event from camera */
	private JpegCallBack _jpegCallback;
	
	/** Configuration variables */
	private String _desk_IP; //Desktop server IP
	private int _desk_Port; // Desktop server port
	private int _app_Port; // Own port server

	/** Interface buttons */
	private Button _btn_calibrate;
	private Button _btn_radar;
	private Button _btn_help;
	private Button _btn_stop;

	/** Boolean control variables */
	public Boolean _isAlive; // used to check if the program is still alive
	public Boolean _working; // used to check if one thread is making a picture
	private boolean _timerOn; // used to check if the timer was activated
	private boolean _serverOn; // used to check if the server was running

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// Hide the window title.
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setContentView(R.layout.main);

		// Restore preferences
	    SharedPreferences settings = getSharedPreferences(PREFS_NAME, Activity.MODE_PRIVATE);
	    int rows = settings.getInt("rows", 2);
		int cols = settings.getInt("cols", 4);
		float tag_width = settings.getFloat("tag_width", 8.0f);
		float tag_height = settings.getFloat("tag_height", 4.5f);
		float tag_border = settings.getFloat("tag_border", 1.0f);
		boolean debugMode = settings.getBoolean("debugMode", false);
		String template = settings.getString("templateTag", "12321113");
		_desk_IP = settings.getString("desk_IP", "192.168.137.1");
		_desk_Port = settings.getInt("desk_Port", 8080);
		_app_Port = settings.getInt("app_Port", 8000);
		
		// Initialization of internal variables, preview, ndk and command
		// receiver server
		_ndk = new NDKWrapper(rows,cols,tag_width,tag_height,tag_border,template,debugMode);
		_notification = null;
		_mPreview = new Preview(this);
		_timer = null;
		_progress = null;
		_helpMenu = null;
		_jpegCallback = new JpegCallBack(this);

		// Control variables
		_currentAction = Actions.NONE;
		_isAlive = true;
		_timerOn = false;
		_working = false;
		_serverOn = true;

		// Set preview and layout
		((FrameLayout) findViewById(R.id.preview)).addView(_mPreview);
		RelativeLayout relativeLayoutControls = (RelativeLayout) findViewById(R.id.controls_layout);
		relativeLayoutControls.bringToFront();

		// Initialization of buttons
		_btn_help = (Button) findViewById(R.id.help_action);
		_btn_radar = (Button) findViewById(R.id.recognize_action);
		_btn_calibrate = (Button) findViewById(R.id.calibrate_action);
		_btn_stop = (Button) findViewById(R.id.stop_action);

		// actions for radar button
		_btn_radar.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				functionSearch();
			}
		});

		// actions for calibrate button
		_btn_calibrate.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				functionCalibrate();
			}
		});

		// actions for help button
		_btn_help.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// show alert dialog
				createHelpMenu();
			}
		});

		// actions for STOP button
		_btn_stop.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				functionStopSearch();
			}
		});

		// initial state
		initialState();
		
		// Prepare to receive notifications from activity and service
		_notification = new mReceiver();
		IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(SettingsActivity.SettingActivitiy_MSG);
		intentFilter.addAction(CmdReceiver.CmdReceiver_OUT_MSG);
		registerReceiver(_notification, intentFilter);
	}

	/**
	 *  Create the alert dialog with the help menu
	 */
	private void createHelpMenu() {
		// Get layout
		LayoutInflater inflater = getLayoutInflater();
		View dialoglayout = inflater.inflate(R.layout.help,
				(ViewGroup) getCurrentFocus());

		// Create builder and set options
		AlertDialog.Builder builder = new AlertDialog.Builder(
				TagRecognitionActivity.this);
		builder.setTitle("TagRecognizer")
				.setCancelable(true)
				.setView(dialoglayout)
				.setIcon(R.drawable.tag25)
				.setPositiveButton("close",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,
									int which) {
								_helpMenu.cancel();
							}
						});

		// Set builder
		_helpMenu = builder.create();
		// show alertdialog
		_helpMenu.show();
	}

	/**
	 * Handles the actions related with the search function
	 */
	public void functionSearch() {
		if (_currentAction == Actions.NONE) {
			// Adjust buttons
			_btn_help.setEnabled(false);
			_btn_calibrate.setEnabled(false);
			_btn_stop.setEnabled(true);
			_btn_stop.setVisibility(0);
			_btn_radar.setEnabled(false);
			_btn_radar.setVisibility(View.GONE);
			// Set action
			_currentAction = Actions.RECOGNITION;
			// Make action
			_timerOn = true;
			createTimer();
		}

	}

	/**
	 * Handles the actions related with the calibration function
	 */
	public void functionCalibrate() {
		if (_currentAction == Actions.NONE) {
			_progress = ProgressDialog.show(TagRecognitionActivity.this,"", 
					"Calibrating. It can take a while...Please don't close the application. ",
					true);
			// Adjust buttons
			_btn_calibrate.setEnabled(false);
			_btn_radar.setEnabled(false);
			_btn_help.setEnabled(false);
			// Set action
			_currentAction = Actions.CALIBRATION;
			// Make action
			takingPicture();
		}
	}

	/**
	 * Handles the actions related with the stop search function
	 */
	public void functionStopSearch() {
		if (_currentAction == Actions.RECOGNITION) {
			// Adjust buttons and state
			initialState();
			// Make action
			_timerOn = false;
			cleanTimer();
		}
	}

	/**
	 * Handles the actions related with send the device view
	 */
	public void functionSendView() {
		if (_currentAction == Actions.NONE) {
			Toast.makeText(TagRecognitionActivity.this,
					"Sending view to server. It can take a while.",
					Toast.LENGTH_LONG).show();
			// Adjust buttons
			_btn_calibrate.setEnabled(false);
			_btn_radar.setEnabled(false);
			_btn_help.setEnabled(false);
			// Set action
			_currentAction = Actions.SENDING_VIEW;
			// Make action
			takingPicture();
		}
	}

	/**
	 * Reset interface state to initial values
	 */
	protected void initialState() {
		// Adjust buttons
		_btn_stop.setVisibility(View.GONE);
		_btn_stop.setEnabled(false);
		_btn_help.setEnabled(true);
		_btn_calibrate.setEnabled(true);
		_btn_radar.setEnabled(true);
		_btn_radar.setVisibility(0);
		// Set action
		_currentAction = Actions.NONE;
	}

	/**
	 *  Create timer to take picture every 2s and run it
	 */
	private void createTimer() {
		_timer = new Timer();
		_timer.schedule(new TimerTask() {
			public void run() {
				// if there are another avoid to put this one in the queue
				synchronized (_working) {
					if (_working) {
						return;
					}
				}
				// take picture
				takingPicture();
			}
		}, 0, 2000);
	}

	/**
	 *  Stop timer
	 */
	private void cleanTimer() {
		if (_timer != null) {
			_timer.cancel();
			_timer = null;
		}
	}

	/**
	 *  Take picture using autofocus
	 */
	private void takingPicture() {
		// because the mCamera could be not already instantiated after onResume
		if (_mPreview.mCamera != null) {
			synchronized (_working) {
				_working = true;
			}
			_mPreview.mCamera.autoFocus(new Camera.AutoFocusCallback() {
				public void onAutoFocus(boolean success,
						Camera camera) {
					synchronized (_isAlive) {
						if (!_isAlive)
							return;
					}
					// take picture
					camera.takePicture(null, null, _jpegCallback);
				}
			});
		}
	}

	/**
	 * Process image get it by the camera a send it to the server
	 * @param data Bitmap image
	 */
	protected void processView(Bitmap data) {
		ArrayList<Bitmap> array = new ArrayList<Bitmap>();
		array.add(data);
		// Send situation of calibration
		DataSender sender = new DataSender(_desk_IP,_desk_Port);
		sender.execute(array);
		// Wait end of operation
		try {
			sender.get();
		} catch (InterruptedException e) {
			e.printStackTrace();
		} catch (ExecutionException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Actions associated to the result of calibration.
	 * Write a toast message and send result to server
	 * @param success
	 */
	protected void processCalibrationResult(boolean success) {
		_progress.dismiss();
		ArrayList<String> s = new ArrayList<String>();
		if (success) {
			s.add(Message.CALIBRATION_OK.toString());
			Toast.makeText(TagRecognitionActivity.this, "Calibration done.",
					Toast.LENGTH_SHORT).show();
		} else {
			s.add(Message.CALIBRATION_FAIL.toString());
			Toast.makeText(TagRecognitionActivity.this,
					"Calibration couldn't been done.", Toast.LENGTH_SHORT)
					.show();
		}

		// Send situation of calibration
		DataSender sender = new DataSender(_desk_IP,_desk_Port);
		sender.execute(s);
		try {
			sender.get();
		} catch (InterruptedException e) {
			e.printStackTrace();
		} catch (ExecutionException e) {
			e.printStackTrace();
		}
	}

	/**
	 * * Actions associated to the result of a tag serach.
	 * Send tags found to the server
	 * @param tagsinfo
	 */
	protected void processTags(String tagsinfo) {
		// Hold the tags founded
		ArrayList<Tag> tags = new ArrayList<Tag>();
		// Get current time
		Date time = new Date();

		// Prepare tags
		if (tagsinfo.length() > 0) {
			Log.d(TAG, "Stream received:" + tagsinfo + ".");
			// Split by token &
			String[] taginfo = tagsinfo.split("&");
			Log.d(TAG, "N¼ of tags: " + taginfo.length);
			// Create tags
			for (int i = 0; i < taginfo.length; i++) {
				try {
					String[] decompressTag = taginfo[i].split("/");
					tags.add(new Tag(Float.parseFloat(decompressTag[0]),
							Float.parseFloat(decompressTag[1]),
							decompressTag[2], time));
				} catch (Exception e) {
					Log.e(TAG, "Format error reading a Tag");
				}
			}

		} else {
			tags.add(new Tag(-1, -1,"", time));
			Log.d(TAG, "No tags founded");
		}

		// Send by net
		DataSender sender = new DataSender(_desk_IP,_desk_Port);
		sender.execute(tags);
		try {
			// wait the end the process
			boolean success = sender.get();
			if (success) {
				Toast.makeText(TagRecognitionActivity.this,
						"Tags sendt to server.", Toast.LENGTH_SHORT).show();
			} else {
				Toast.makeText(TagRecognitionActivity.this,
						"Tags couldn't been sent to server.",
						Toast.LENGTH_SHORT).show();
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		} catch (ExecutionException e) {
			e.printStackTrace();
		}
	}

	/**
	 * BroadcastReceiver implementation
	 * Handles Intent from another activity or intentservice
	 */
	public class mReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			Bundle bundle = intent.getExtras();
			if (intent.getAction().equals(CmdReceiver.CmdReceiver_OUT_MSG)) {
				String cmd = bundle.getString(CmdReceiver.CmdReceiver_OUT_MSG);
				if (cmd.equals(Message.CALIBRATE.toString())) {
					functionCalibrate();
				} else if (cmd.equals(Message.START_SEARCH.toString())) {
					functionSearch();
				} else if (cmd.equals(Message.STOP_SEARCH.toString())) {
					functionStopSearch();
				} else if (cmd.equals(Message.SEND_VIEW.toString())) {
					functionSendView();
				}
			}
			else if (intent.getAction().equals(SettingsActivity.SettingActivitiy_MSG)) {
				Boolean changes = bundle.getBoolean(SettingsActivity.SettingActivitiy_DATA);
				if (changes == true ) {
					// Restore preferences
				    SharedPreferences settings = getSharedPreferences(PREFS_NAME, Activity.MODE_PRIVATE);
				    int rows = settings.getInt("rows", 2);
					int cols = settings.getInt("cols", 4);
					float tag_width = settings.getFloat("tag_width", 8.0f);
					float tag_height = settings.getFloat("tag_height", 4.5f);
					float tag_border = settings.getFloat("tag_border", 1.0f);
					boolean debugMode = settings.getBoolean("debugMode", true);
					String template = settings.getString("templateTag", "12321113");
					_desk_IP = settings.getString("desk_IP", "192.168.137.1");
					_desk_Port = settings.getInt("desk_Port", 8080);
					_app_Port = settings.getInt("app_Port", 8000);
					
					//update NDK
					_ndk.updateConfParams(rows, cols, tag_width, tag_height, tag_border, template, debugMode);
					
				}
			}
			
		}

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.layout.menu, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// TODO Auto-generated method stub
		switch (item.getItemId()) {
		case R.id.menu_about:
			// startActivity(new Intent(this, About.class));
			return true;
		case R.id.menu_settings:
			startActivity(new Intent(this,SettingsActivity.class));
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
	}

	@Override
	protected void onResume() {
		// If timer was working begin again with recognition
		if (_timerOn) {
			functionSearch();
		}
		_isAlive = true;
		_working = false;
		
		// Open server
		if (_serverOn) {
			Intent i = new Intent(this, CmdReceiver.class);
			i.putExtra(CmdReceiver.CmdReceiver_IN_MSG, CmdReceiver.PARAM_START);
			i.putExtra(CmdReceiver.CmdReceiver_PORT_MSG, _app_Port);
			startService(i);
		}
		
		super.onResume();
	}

	@Override
	protected void onPause() {
		synchronized (_isAlive) {
			_isAlive = false;
		}
		_mPreview.release();
		cleanTimer();
				
		// Close server
		Intent i = new Intent(this, CmdReceiver.class);
		i.putExtra(CmdReceiver.CmdReceiver_IN_MSG, CmdReceiver.PARAM_STOP);
		startService(i);

		// Reset buttons
		initialState();

		super.onPause();
	}

	@Override
	protected void onDestroy() {
		// Unregister receiver
		if (_notification != null) {
			unregisterReceiver(_notification);
			_notification = null;
		}
				
		if (_helpMenu != null) {
			_helpMenu.cancel();
		}
		super.onDestroy();
	}
}