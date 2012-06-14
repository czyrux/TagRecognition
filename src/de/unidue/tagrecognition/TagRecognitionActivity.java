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
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.Toast;

public class TagRecognitionActivity extends Activity {
	private static final String TAG = "CVJNI_Activity";
	public static final int MEDIA_TYPE_IMAGE = 1;

	private Preview mPreview;
	// private Camera mCamera;
	private Timer timer , timer2;
	private OpenCV opencv = new OpenCV();

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// Hide the window title.
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setContentView(R.layout.main);

		mPreview = new Preview(this); // <3>
		((FrameLayout) findViewById(R.id.preview)).addView(mPreview); // <4>

		// this is where we call the native code
		// String hello = opencv.hello();
		// Toast.makeText(TagRecognitionActivity.this, hello,
		// Toast.LENGTH_SHORT).show();
		// new AlertDialog.Builder(this).setMessage(hello).show();

		timer = new Timer();
		timer.schedule(new UpdateTimeTask(), 1000, 2000);

	}

	int i = 0;

	class UpdateTimeTask extends TimerTask {//
		public void run() {
			if (i < 1) {
				Log.d(TAG, "foto " + i);
				mPreview.mCamera.autoFocus(new Camera.AutoFocusCallback() {
					public void onAutoFocus(boolean success, Camera camera) {
							camera.takePicture(null, null, jpegCallback);
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
			long start , end , elapse;
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
			//208 ms
			//Bitmap bmpFilter = opencv.opencvFilter(bmpRotate);
			
			
			//1700 ms
			/*
			start = System.currentTimeMillis();
			Bitmap bmpFilter2 = ColorFilter.extractRed(bmpRotate, 0);
			end = System.currentTimeMillis();
			elapse = end - start;
			Toast.makeText(TagRecognitionActivity.this, "" + elapse + " ms is used to extract features.",
					Toast.LENGTH_LONG).show();
			*/
			
			/**
			 * JNI ALGORITHM1 WORKING SLOW 320 ms
			 */
			
			/*
			start = System.currentTimeMillis();
			Bitmap bmpExtract1 = opencv.opencvImage(bmpFilter);
			end = System.currentTimeMillis();
			elapse = end - start;
			Toast.makeText(TagRecognitionActivity.this, "" + elapse + " ms is used to extract features.",
					Toast.LENGTH_LONG).show();
			*/
			/**
			 * JNI ALGORITHM2 WORKING (complete program)
			 * FAST 840 ms
			 * SURF 4000ms
			 */
			
			Bitmap bmpExtract2 = opencv.openCV(bmpRotate);
			end = System.currentTimeMillis();
			elapse = end - start;
			Toast.makeText(TagRecognitionActivity.this, "" + elapse + " ms is used to extract features.",
					Toast.LENGTH_LONG).show();
			
			//WITH ROTATE OUTSIDE = 800 MS
			//SIN ROTATE 600 MS
			//WITH ROTATE INSIDE = 800ms
			
			// Store the image
			storeBitmap(bmpRotate, "IMG_0_");
			storeBitmap(bmpExtract2, "IMG_1_");

			// Continue with the preview
			mPreview.mCamera.startPreview();
		}
	};

	// Save Bitmap
	private void storeBitmap (Bitmap bmp , String head ) {
		int quality = 100;

		File pictureFile = getOutputMediaFile(MEDIA_TYPE_IMAGE, head);
		if (pictureFile == null) {
			Log.d(TAG,
					"Error creating media file, check storage permissions: ");
			return;
		}
		// Store the image
		try {
			Log.d(TAG, pictureFile.toString());
			FileOutputStream fos = new FileOutputStream(pictureFile);
			bmp.compress(CompressFormat.JPEG, quality, fos);
			fos.flush();
			fos.close();
		} catch (FileNotFoundException e) {
			Log.d(TAG, "File not found: " + e.getMessage());
		} catch (IOException e) {
			Log.d(TAG, "Error accessing file: " + e.getMessage());
		} catch (Exception e) {
			Log.d(TAG, "Unkown error: " + e.getMessage());
		}
	}

	// Create a File for saving an image
	private File getOutputMediaFile(int type, String head) {
		// To be safe, you should check that the SDCard is mounted
		// using Environment.getExternalStorageState() before doing this.

		// Make the path to a directory with name MyCameraApp in the directory
		// of pictures
		File mediaStorageDir = new File(
				Environment
						.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES),
				"MyCameraApp");

		// Create the storage directory if it does not exist
		if (!mediaStorageDir.exists()) {
			if (!mediaStorageDir.mkdirs()) {
				Log.d("MyCameraApp", "failed to create directory");
				return null;
			}
		}

		// Create a media file name
		String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss")
				.format(new Date());
		File mediaFile;
		if (type == MEDIA_TYPE_IMAGE) {
			mediaFile = new File(mediaStorageDir.getPath() + File.separator
					+ head + timeStamp + ".jpeg");
		} else {
			return null;
		}

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
	}

	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		super.onPause();
	}

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
	}

	private void cleanTimer() {
		if (timer != null) {
			this.timer.cancel();
			timer = null;
		}
	}

}