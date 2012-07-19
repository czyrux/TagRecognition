package de.unidue.tagrecognition;

import android.graphics.Bitmap;
import android.util.Log;

public class NDKWrapper {
	// load the library - name matches jni/Android.mk
	static {
		System.loadLibrary("tagrecognizer-jni");
	}

	// native functions
	private native String tagRecognizer(int[] pixels, int width, int height);

	private native boolean calibrate(int[] pixels, int width, int height);

	//private native void nativeSetup();

	// shared variables
	private int _RED_BOUNDARY;
	private int _BLUE_BOUNDARY;
	private int _GREEN_BOUNDARY;

	public NDKWrapper() {
		_RED_BOUNDARY = 0;
		_BLUE_BOUNDARY = -10;
		_GREEN_BOUNDARY = -20;

		//nativeSetup();
	}

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

	public void calibration(Bitmap bitmap ) {
		// prepare bitmap
		int width = bitmap.getWidth();
		int height = bitmap.getHeight();
		int[] pixels = new int[width * height];
		bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
		
		//call jni method
		boolean realize = calibrate(pixels, width, height);
		//Log.d("VALORES AJUSTADOS", "R:" + _RED_BOUNDARY + " G:" + _GREEN_BOUNDARY + " B:" + _BLUE_BOUNDARY);
		Log.d("AJUSTE",""+realize);
				
	}
}
