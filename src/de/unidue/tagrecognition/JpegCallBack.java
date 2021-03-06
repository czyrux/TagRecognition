package de.unidue.tagrecognition;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

/**
 * @file JpegCallBack.java
 * @brief Implements of PictureCallBack interface. Handles the jpeg image captured by the mobile camera
 * @author Antonio Manuel Gutierrez Martinez
 * @version 1.0
 */
public class JpegCallBack implements PictureCallback {
	/** Label string to log proposes. */
	private final String TAG = "JpegCallBack";

	/** Reference to called activity */
	private TagRecognitionActivity _activity;

	/**
	 * Constructor
	 * @param activity: reference to called activity
	 */
	public JpegCallBack(TagRecognitionActivity activity) {
		_activity = activity;
	}

	@Override
	/**
	 * Called when image data is available after a picture is taken. The format
	 * of the data depends on the context of the callback and Camera.Parameters
	 * settings.
	 * @param data: a byte array of the picture data
	 * @param camera: the Camera service object
	 */
	public void onPictureTaken(byte[] data, Camera camera) {
		// checked if the activity is still alive
		synchronized (camera) {
			if (!_activity._isAlive) {
				return;
			}
		}

		Long start, end, elapse;
		start = System.currentTimeMillis();

		// Read the stream of data
		BitmapFactory.Options options = new BitmapFactory.Options();
		if (_activity._currentAction == Actions.SENDING_VIEW)
			options.inSampleSize = 2; // make the picture 1/2 of size
		else
			options.inSampleSize = 4; // make the picture 1/4 of size

		Bitmap bmp = BitmapFactory.decodeByteArray(data, 0, data.length,
				options);

		// Select operation
		String tags;
		if (_activity._currentAction == Actions.RECOGNITION) {
			tags = _activity._ndk.tagRecognizer(bmp);
			_activity.processTags(tags);
		} else if (_activity._currentAction == Actions.CALIBRATION) {
			boolean success = _activity._ndk.calibration(bmp);
			_activity.processCalibrationResult(success);
			// return to initial state
			_activity.initialState();
		}
		if (_activity._currentAction == Actions.SENDING_VIEW) {
			_activity.processView(bmp);
			// return to initial state
			_activity.initialState();
		}

		// Get time
		end = System.currentTimeMillis();
		elapse = end - start;
		Toast.makeText(_activity,
				"" + elapse + " ms is used to do the operation.",
				Toast.LENGTH_LONG).show();
		Log.d(TAG, elapse.toString() + " ms");

		// Continue with the preview
		_activity._mPreview.mCamera.startPreview();

		// release block
		synchronized (_activity._working) {
			_activity._working = false;
		}
	}

	/**
	 * Store the Bitmap object image 
	 * @param bmp Bitmap object
	 */
	@SuppressWarnings("unused")
	private void storeBitmap(Bitmap bmp, String head) {
		int quality = 100;

		File pictureFile = getOutputMediaFile(head);
		if (pictureFile == null) {
			Log.d(TAG, "Error creating media file, check storage permissions: ");
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
			Log.d(TAG, "File not found: " + e.getMessage());
		} catch (IOException e) {
			Log.d(TAG, "Error accessing file: " + e.getMessage());
		} catch (Exception e) {
			Log.d(TAG, "Unkown error: " + e.getMessage());
		}
	}

	/**
	 * Create a File for saving an image
	 * @param head Head added to file name
	 */
	private File getOutputMediaFile(String head) {
		// To be safe, you should check that the SDCard is mounted
		// using Environment.getExternalStorageState() before doing this.

		// Make the path to a directory with name MyCameraApp in the directory
		// of pictures
		File mediaStorageDir = new File(
				Environment
						.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES),
				"TagRecognizerApp");

		// Create the storage directory if it does not exist
		if (!mediaStorageDir.exists()) {
			if (!mediaStorageDir.mkdirs()) {
				Log.d(TAG, "failed to create directory");
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

}
