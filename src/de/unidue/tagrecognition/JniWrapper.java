package de.unidue.tagrecognition;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

public class JniWrapper {
	// load the library - name matches jni/Android.mk
	static {
		System.loadLibrary("tagrecognizer-jni");
	}

	// native functions
	//public native byte[] extractFAST(int[] pixels, int width, int height);

	private native byte[] tagRecognizer(int[] pixels, int width, int height);

	private native void calibrate();

	private native void nativeSetup();

	// shared variables
	private int _RED_BOUNDARY ;
	private int _BLUE_BOUNDARY ;
	private int _GREEN_BOUNDARY ;

	public JniWrapper() {
		_RED_BOUNDARY = 0;
		_BLUE_BOUNDARY = -10;
		_GREEN_BOUNDARY = -20;
		
		nativeSetup();
	}

	// llamada para paso de imagen directa
	public Bitmap tagRecognizer(Bitmap bitmap) {
		// prepare bitmap
		int width = bitmap.getWidth();
		int height = bitmap.getHeight();
		int[] pixels = new int[width * height];
		bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
		// call method jni
		// byte[] imageData = this.extractFAST(pixels, width, height);
		byte[] imageData = this.tagRecognizer(pixels, width, height);

		// prepare the data returned
		Bitmap bmp = null;
		if (imageData != null)
			bmp = BitmapFactory.decodeByteArray(imageData, 0, imageData.length);
		return bmp;
	}

	public void calibration () {
		Log.d("OPENCV JAVA", "R:" + _RED_BOUNDARY + " G:" + _GREEN_BOUNDARY + " B:" + _BLUE_BOUNDARY );
	}
}
