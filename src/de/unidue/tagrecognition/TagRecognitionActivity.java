package de.unidue.tagrecognition;

import java.util.ArrayList;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ExecutionException;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.os.Bundle;

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

public class TagRecognitionActivity extends Activity {
	private static final String TAG = "TagRecognizer";

	public Preview _mPreview;
	public Actions _currentAction;
	public NDKWrapper _ndk;
	private Timer _timer;
	private AlertDialog _helpMenu;
	private mCmdReceiver _notification;
	
	private JpegCallBack _jpegCallback;

	private Button _btn_calibrate;
	private Button _btn_radar;
	private Button _btn_help;
	private Button _btn_stop;

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

		// Initialization of internal variables, preview, ndk and command
		// receiver server
		_ndk = new NDKWrapper();
		_notification = null;
		_mPreview = new Preview(this);
		_timer = null;
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
				// enable option buttons
				// _btn_calibrate.setEnabled(true);
				// _btn_radar.setEnabled(true);
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
	}

	
	/**
	 * 
	 */
	private void enableServer() {
		// Create server
		Intent i = new Intent(this, CmdReceiver.class);
		i.putExtra(CmdReceiver.CmdReceiver_IN_MSG, CmdReceiver.PARAM_START);
		startService(i);

		// Prepare to receive notifications from service
		_notification = new mCmdReceiver();
		IntentFilter intentFilter = new IntentFilter(
				CmdReceiver.CmdReceiver_OUT_MSG);
		registerReceiver(_notification, intentFilter);
	}

	/**
	 * 
	 */
	private void disableServer() {
		// Unregister receiver
		if (_notification != null) {
			unregisterReceiver(_notification);
			_notification = null;
		}
		// Stop server
		Intent i = new Intent(this, CmdReceiver.class);
		i.putExtra(CmdReceiver.CmdReceiver_IN_MSG, CmdReceiver.PARAM_STOP);
		startService(i);
	}

	// Create the alert dialog with the help menu
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
				.setIcon(R.raw.tag25)
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

	public void functionCalibrate() {
		if (_currentAction == Actions.NONE) {
			Toast.makeText(TagRecognitionActivity.this,
					"Calibrating params. It can take a while",
					Toast.LENGTH_SHORT).show();
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

	public void functionStopSearch() {
		if (_currentAction == Actions.RECOGNITION) {
			// Adjust buttons and state
			initialState();
			// Make action
			_timerOn = false;
			cleanTimer();
		}
	}

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

	// Create timer
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

	// Delete timer
	private void cleanTimer() {
		if (_timer != null) {
			_timer.cancel();
			_timer = null;
		}
	}

	// Make autofocus and do the picture
	private void takingPicture() {
		// because the mCamera could be not already instantiated after onResume
		if (_mPreview.mCamera != null) {
			synchronized (_working) {
				_working = true;
			}
			_mPreview.mCamera.autoFocus(new Camera.AutoFocusCallback() {
				public synchronized void onAutoFocus(boolean success,
						Camera camera) {
					// take picture
					camera.takePicture(null, null, _jpegCallback);
				}
			});
		}
	}

	protected void processView(byte[] data) {
		ArrayList<byte[]> array = new ArrayList<byte[]>();
		array.add(data);
		// Send situation of calibration
		DataSender sender = new DataSender();
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

	protected void processCalibrationResult(boolean success) {
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
		DataSender sender = new DataSender();
		sender.execute(s);
		try {
			sender.get();
		} catch (InterruptedException e) {
			e.printStackTrace();
		} catch (ExecutionException e) {
			e.printStackTrace();
		}
	}

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
			Log.d(TAG, "N� of tags: " + taginfo.length);
			// Create tags
			for (int i = 0; i < taginfo.length; i++) {
				try {
					String[] decompressTag = taginfo[i].split("/");
					tags.add(new Tag(Integer.parseInt(decompressTag[0]),
							Integer.parseInt(decompressTag[1]),
							decompressTag[2], time));
				} catch (Exception e) {
					Log.e(TAG, "Format error reading a Tag");
				}
			}

		} else {
			tags.add(new Tag(-1, -1, "", time));
			Log.d(TAG, "No tags founded");
		}

		// Send by net
		DataSender sender = new DataSender();
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

	public class mCmdReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			Bundle bundle = intent.getExtras();
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

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// TODO Auto-generated method stub
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
			// startActivity(new Intent(this, Help.class));
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
			enableServer();
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
		disableServer();

		// Reset buttons
		initialState();

		super.onPause();
	}

	@Override
	protected void onDestroy() {
		if (_helpMenu != null) {
			_helpMenu.cancel();
		}
		super.onDestroy();
	}
}