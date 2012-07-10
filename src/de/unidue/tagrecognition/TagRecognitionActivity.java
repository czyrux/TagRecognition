package de.unidue.tagrecognition;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
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
import android.graphics.Matrix;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.os.Environment;
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
	private static final String NAME = "TagRecognizer";

	private Preview _mPreview;
	private Timer _timer;
	private JniWrapper _jni ;
	private AlertDialog _helpMenu;

	private Button _btn_calibrate;
	private Button _btn_radar;
	private Button _btn_help;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// Hide the window title.
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setContentView(R.layout.main);

		_jni = new JniWrapper();
		_timer = null;
		_helpMenu = null;
		
		_mPreview = new Preview(this);
		((FrameLayout) findViewById(R.id.preview)).addView(_mPreview);

		RelativeLayout relativeLayoutControls = (RelativeLayout) findViewById(R.id.controls_layout);
		relativeLayoutControls.bringToFront();

		_btn_help = (Button) findViewById(R.id.help_action);
		_btn_radar = (Button) findViewById(R.id.recognize_action);
		_btn_calibrate = (Button) findViewById(R.id.calibrate_action);

		// recognize button action
		_btn_radar.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				_btn_radar.setEnabled(false);
				_timer = new Timer();
				_timer.schedule(new UpdateTimeTask(), 1000, 2000);
			}
		});
		_btn_radar.setEnabled(false);

		// actions for calibrate button
		_btn_calibrate.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// enable option buttons
				_btn_calibrate.setEnabled(false);
				_btn_radar.setEnabled(true);
				//_jni.calibration();
			}
		});

		_btn_help.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// enable option buttons
				_btn_calibrate.setEnabled(true);
				_btn_radar.setEnabled(false);
				//show alert dialog
				createHelpMenu();
				/*
				 * Dialog dialog = new Dialog(TagRecognitionActivity.this);
				 * dialog.setCancelable(true);
				 * dialog.setContentView(R.layout.help);
				 * dialog.setTitle("info"); dialog.show();
				 */
				
			}
		});
		
	}
	
	//Create the alert dialog
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
					.setPositiveButton("Accept",
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog,
										int which) {
									_helpMenu.cancel();
								}
							});
			// .setIcon(R.drawable.icon);
			// Set builder
			_helpMenu = builder.create();
			// show alertdialog
			_helpMenu.show();
	}

	// for the temporal calling to task
	int i = 0;

	class UpdateTimeTask extends TimerTask {//
		public void run() {
			if (i < 1) {
				Log.d(NAME, "taken foto " + i);
				_mPreview.mCamera.autoFocus(new Camera.AutoFocusCallback() {
					public void onAutoFocus(boolean success, Camera camera) {
						camera.takePicture(null, null, jpegCallback);
						_btn_radar.setEnabled(true);
					}
				});
			} else
				cleanTimer();
			i++;
		}
	}

	// Handles data for jpeg picture
	private PictureCallback jpegCallback = new PictureCallback() {
		@Override
		public void onPictureTaken(byte[] data, Camera camera) {
			Long start, end, elapse;
			start = System.currentTimeMillis();
			BitmapFactory.Options options = new BitmapFactory.Options();
			options.inSampleSize = 4; // make the picture 1/4 of size
			// options.inSampleSize = 1;
			Bitmap bmp = BitmapFactory.decodeByteArray(data, 0, data.length,
					options);

			// Rotate the image to 90
			Matrix mtx = new Matrix();
			mtx.postRotate(90);

			// Rotating Bitmap
			Bitmap bmpRotate = Bitmap.createBitmap(bmp, 0, 0, bmp.getWidth(),
					bmp.getHeight(), mtx, true);

			// Filter image
			// 208 ms jni
			// 1700 ms java

			/**
			 * JNI ALGORITHM2 WORKING (complete program) FAST 840 ms SURF 4000ms
			 */

			Bitmap bmpExtract2 = _jni.tagRecognizer(bmpRotate);
			end = System.currentTimeMillis();
			elapse = end - start;
			Toast.makeText(TagRecognitionActivity.this,
					"" + elapse + " ms is used to extract features.",
					Toast.LENGTH_LONG).show();
			Log.d(NAME, elapse.toString() + " ms");

			// WITH ROTATE OUTSIDE = 800 MS
			// SIN ROTATE 600 MS
			// WITH ROTATE INSIDE = 800ms

			// Store the image
			//storeBitmap(bmpRotate, "IMG_0_");
			if (bmpExtract2 != null)
				storeBitmap(bmpExtract2, "IMG_1_");

			// Continue with the preview
			_mPreview.mCamera.startPreview();
		}
	};

	// Save Bitmap
	private void storeBitmap(Bitmap bmp, String head) {
		int quality = 100;

		File pictureFile = getOutputMediaFile(head);
		if (pictureFile == null) {
			Log.d(NAME, "Error creating media file, check storage permissions: ");
			return;
		}
		// Store the image
		try {
			//Log.d(NAME, pictureFile.toString());
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
				NAME +"App");

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

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		this.cleanTimer();
		if ( _helpMenu != null ) {
			_helpMenu.cancel();
		}
	}

	@Override
	protected void onPause() {
		super.onPause();
		_mPreview.release();
	}

	// Delete timer
	private void cleanTimer() {
		if (_timer != null) {
			_timer.cancel();
			_timer = null;
		}
	}

}