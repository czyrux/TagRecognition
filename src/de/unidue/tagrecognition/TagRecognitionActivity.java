package de.unidue.tagrecognition;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.res.Configuration;
import android.os.Bundle;

import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.util.Log;
import android.view.LayoutInflater;
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

	private Preview _mPreview;
	private Timer _timer;
	private JniWrapper _jni;
	private AlertDialog _helpMenu;

	private Button _btn_calibrate;
	private Button _btn_radar;
	private Button _btn_help;
	private Button _btn_stop;

	private boolean _recognizerFunction;
	private Boolean _isAlive; //used to check if the program is still alive 
	private Boolean _working; //used to check if one thread is making a picture
	private boolean _timerOn; //used to check if the timer was activated

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// Hide the window title.
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setContentView(R.layout.main);

		// Initialization of internal variables, preview and ndk
		_jni = new JniWrapper();
		_mPreview = null;
		_timer = null;
		_helpMenu = null;

		// Boolean control variables
		_isAlive = true;
		_timerOn = false;
		_working = false;

		// Set preview and layout
		_mPreview = new Preview(this);
		((FrameLayout) findViewById(R.id.preview)).addView(_mPreview);
		RelativeLayout relativeLayoutControls = (RelativeLayout) findViewById(R.id.controls_layout);
		relativeLayoutControls.bringToFront();

		// Initialization of buttons
		_btn_help = (Button) findViewById(R.id.help_action);
		_btn_radar = (Button) findViewById(R.id.recognize_action);
		_btn_calibrate = (Button) findViewById(R.id.calibrate_action);
		_btn_stop = (Button) findViewById(R.id.stop_action);
		_btn_stop.setVisibility(View.GONE);

		// recognize button action
		_btn_radar.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				_recognizerFunction = true;
				_btn_stop.setEnabled(true);
				_btn_stop.setVisibility(0);
				_btn_radar.setEnabled(false);

				// change icon

				// Activate timer
				createTimer();
			}
		});
		_btn_radar.setEnabled(true);

		// actions for calibrate button
		_btn_calibrate.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// Call picture
				_recognizerFunction = false;
				_btn_calibrate.setEnabled(false);
				takingPicture();
			}
		});

		// actions for help button
		_btn_help.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// enable option buttons
				_btn_calibrate.setEnabled(true);
				_btn_radar.setEnabled(false);
				// show alert dialog
				createHelpMenu();
			}
		});

		// actions for help button
		_btn_stop.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				_timerOn = false;
				_btn_stop.setVisibility(View.GONE);
				_btn_stop.setEnabled(false);
				_btn_radar.setEnabled(true);
				cleanTimer();
			}
		});
		_btn_stop.setEnabled(false);

	}

	// Create the alert dialog with the help menu
	private void createHelpMenu() 
	{
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
				.setPositiveButton("Accept",
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

	// for the temporal calling to task
	int i = 0;

	class UpdateTimeTask extends TimerTask {
		public void run() {
			//if there are another avoid to put this one in the queue
			synchronized (_working) {
				if (_working) {
					return;
				}
			}
			takingPicture();
		}
	}

	// Create timer
	private void createTimer() {
		_timerOn = true;
		i = 0;
		_timer = new Timer();
		_timer.schedule(new UpdateTimeTask(), 0, 2000);
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
		//because the mCamera could be not already instantiated
		if (_mPreview.mCamera != null) 
		{
			synchronized (_working) {
				_working = true;
			}
			
			_mPreview.mCamera.autoFocus(new Camera.AutoFocusCallback() {
				public synchronized void onAutoFocus(boolean success,
						Camera camera) {
					// take picture
					camera.takePicture(null, null, jpegCallback);
				}
			});
		}
	}

	// Handles data for jpeg picture
	private PictureCallback jpegCallback = new PictureCallback() {
		@Override
		public void onPictureTaken(byte[] data, Camera camera) {
			// checked if the activity is still alive
			synchronized (camera) {
				if (!_isAlive) {
					return;
				}
			}
			
			Long start, end, elapse;
			start = System.currentTimeMillis();

			// Read the stream of data
			BitmapFactory.Options options = new BitmapFactory.Options();
			options.inSampleSize = 4; // make the picture 1/4 of size
			//options.inSampleSize = 2;
			Bitmap bmp = BitmapFactory.decodeByteArray(data, 0, data.length,
					options);

			/* Rotate the image to 90
			 * Matrix mtx = new Matrix(); mtx.postRotate(90);
			 * Rotating Bitmap Bitmap bmpRotate = Bitmap.createBitmap(bmp, 0,0, bmp.getWidth(), bmp.getHeight(), mtx, true);
			 */

			// Select operation
			String tags;
			if (_recognizerFunction == true) {
				tags = _jni.tagRecognizer(bmp);
				processTags(tags);
				_btn_calibrate.setEnabled(true);
			} else {
				_jni.calibration(bmp);
				_btn_radar.setEnabled(true);
			}

			// Get time
			end = System.currentTimeMillis();
			elapse = end - start;
			Toast.makeText(TagRecognitionActivity.this,
					"" + elapse + " ms is used to do the operation.",
					Toast.LENGTH_LONG).show();
			Log.d(TAG, elapse.toString() + " ms");

			// WITH ROTATE OUTSIDE = 800 MS
			// SIN ROTATE 600 MS
			// WITH ROTATE INSIDE = 800ms
			// Filter image
			// 208 ms jni
			// 1700 ms java

			// Continue with the preview
			_mPreview.mCamera.startPreview();
			
			//release
			synchronized (_working) {
				_working = false;
			}
		}
	};
	
	private void processTags( String tagsinfo ){
		//Store the tags founded
		ArrayList<Tag> tags = new ArrayList<Tag>();
		//Get current time
		Date time = new Date();
		
		//Prepare tags
		if ( tagsinfo.length() > 0 ) 
		{
			Log.d(TAG, "Stream received:" + tagsinfo +".");
			//Split by token &
			String [] taginfo = tagsinfo.split("&");
			Log.d(TAG, "N¼ of tags: " + taginfo.length);
			//Create tags
			for ( int i=0 ; i<taginfo.length ; i++ ){
				try {
					String[] decompressTag = taginfo[i].split("/");
					tags.add(new Tag(Integer.parseInt(decompressTag[0]),
							Integer.parseInt(decompressTag[1]),
							decompressTag[2],time));
					
					//two times the same tag
					tags.add(new Tag(Integer.parseInt(decompressTag[0]),
							Integer.parseInt(decompressTag[1]),
							decompressTag[2],time));
				}
				catch ( Exception e){
					Log.e(TAG, "Format error reading a Tag");
				}
			}

		}else{
			tags.add(new Tag(-1,-1,"",time));
			Log.d(TAG, "No tags founded");
		}
		
		//Send by net
		SenderData net = null;
		try {
			net = new SenderData();
		} catch (UnknownHostException e) {
			Log.d(TAG, "Server not found");
			e.printStackTrace();
		} catch (IOException e) {
			Log.d(TAG, "Problems open streams");
			e.printStackTrace();
		} catch ( Exception e){
			Log.d(TAG,"Unkowns exception opening socket");
			e.printStackTrace();
		}
		
		if ( net != null ) {
			try {
				net.sendData(tags);
			} catch (IOException e) {
				Log.d(TAG, "Error sending tags");
				e.printStackTrace();
			}
			net.closeConnection();	
		}
				
		
	}

	// Save Bitmap
	/*
	private void storeBitmap(Bitmap bmp, String head) {
		int quality = 100;

		File pictureFile = getOutputMediaFile(head);
		if (pictureFile == null) {
			Log.d(NAME,
					"Error creating media file, check storage permissions: ");
			return;
		}
		// Store the image
		try {
			// Log.d(NAME, pictureFile.toString());
			FileOutputStream fos = new FileOutputStream(pictureFile);
			bmp.compress(CompressFormat.JPEG, quality, fos);
			fos.flush();
			fos.close();
		} catch (FileNotFoundException e) {
			Log.d(NAME, "File not found: " + e.getMessage());
		} catch (IOException e) {
			Log.d(NAME, "Error accessing file: " + e.getMessage());
		} catch (Exception e) {
			Log.d(NAME, "Unkown error: " + e.getMessage());
		}
	}

	// Create a File for saving an image
	private File getOutputMediaFile(String head) {
		// To be safe, you should check that the SDCard is mounted
		// using Environment.getExternalStorageState() before doing this.

		// Make the path to a directory with name MyCameraApp in the directory
		// of pictures
		File mediaStorageDir = new File(
				Environment
						.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES),
				NAME + "App");

		// Create the storage directory if it does not exist
		if (!mediaStorageDir.exists()) {
			if (!mediaStorageDir.mkdirs()) {
				Log.d(NAME, "failed to create directory");
				return null;
			}
		}

		// Create a media file name
		String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss")
				.format(new Date());
		File mediaFile = new File(mediaStorageDir.getPath() + File.separator
				+ head + timeStamp + ".jpeg");

		return mediaFile;
	}
*/

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
	}

	@Override
	protected void onResume() {
		//If timer was working, create a new one
		if (_timerOn) {
			createTimer();
		}
		_isAlive = true;
		_working = false;
		
		super.onResume();
	}

	@Override
	protected void onPause() {
		synchronized (_isAlive) {
			_isAlive = false;
		}
		_mPreview.release();
		cleanTimer();

		super.onPause();
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		if (_helpMenu != null) {
			_helpMenu.cancel();
		}
	}

}