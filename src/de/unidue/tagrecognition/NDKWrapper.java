package de.unidue.tagrecognition;

import android.graphics.Bitmap;
import android.util.Log;

/**
 * @file NDKWrapper.java
 * @brief Wrapper to call the C++ methods with JNI (Java Native Interface)
 * @author Antonio Manuel Gutierrez Martinez
 * @version 1.0
 */
public class NDKWrapper {

	/**
	 *  load the library - name matches jni/Android.mk
	 */
	static {
		System.loadLibrary("tagrecognizer-jni");
	}

	/** Native methods */
	private native String tagRecognizer(int[] pixels, int width, int height);
	private native boolean calibrate(int[] pixels, int width, int height);
	private native void setup(int rows , int cols , float tag_width, float tag_height , float tag_border , String template , boolean debugMode);

	/**
	 * Constructor
	 */
	public NDKWrapper( int rows , int cols , float tag_width, float tag_height , float tag_border , String template , boolean debugMode ) {
		updateConfParams(rows, cols, tag_width, tag_height, tag_border, template, debugMode);
	}

	/**
	 * Wrapper to call the native method tagRecognizer
	 * @param bitmap Image to process natively
	 * @return String with the tag located in the image
	 */
	public String tagRecognizer(Bitmap bitmap) {
		// prepare bitmap
		int width = bitmap.getWidth();
		int height = bitmap.getHeight();
		int[] pixels = new int[width * height];
		bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
		
		// call method jni
		//byte[] imageData = 
		String tags = this.tagRecognizer(pixels, width, height);

		// prepare the data returned
		/*
		Bitmap bmp = null;
		if (imageData != null)
			bmp = BitmapFactory.decodeByteArray(imageData, 0, imageData.length);
		return bmp;*/
		return tags;
	}

	/**
	 * Wrapper to call the native method calibrate
	 * @param bitmap Image used to calibrate thresholds values.
	 * @return success of calibration process
	 */
	public boolean calibration(Bitmap bitmap ) {
		// prepare bitmap
		int width = bitmap.getWidth();
		int height = bitmap.getHeight();
		int[] pixels = new int[width * height];
		bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
		
		//call jni method
		boolean success = calibrate(pixels, width, height);
		//Log.d("VALORES AJUSTADOS", "R:" + _RED_BOUNDARY + " G:" + _GREEN_BOUNDARY + " B:" + _BLUE_BOUNDARY);
		Log.d("AJUSTE",""+success);
		
		return success;
	}

	/**
	 * Setup of native configuration parameters
	 * @param rows Number of rows on tags.
	 * @param cols Number of cols on tags.
	 * @param tag_width Width of tags.
	 * @param tag_height Height of tags.
	 * @param tag_border Border width of tags.
	 * @param template Tag template code to calibration process.
	 * @param debugMode Enable/Disable debug mode
	 */
	public void updateConfParams(int rows , int cols , float tag_width, float tag_height , float tag_border , String template , boolean debugMode) {
		setup(rows,cols,tag_width,tag_height,tag_border,template,debugMode);
	}
}
