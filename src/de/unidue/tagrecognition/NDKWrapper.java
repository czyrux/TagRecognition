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
	private native void setup(int rows , int cols , float tag_width, float tag_height , float tag_border , String template , boolean debugMode);

	
	public NDKWrapper( int rows , int cols , float tag_width, float tag_height , float tag_border , String template , boolean debugMode ) {
		updateConfParams(rows, cols, tag_width, tag_height, tag_border, template, debugMode);
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

	public void updateConfParams(int rows , int cols , float tag_width, float tag_height , float tag_border , String template , boolean debugMode) {
		setup(rows,cols,tag_width,tag_height,tag_border,template,debugMode);
	}
}
