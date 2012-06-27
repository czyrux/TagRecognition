package de.unidue.tagrecognition;


import android.graphics.Bitmap;
import android.graphics.BitmapFactory;


public class OpenCV {
	// load the library - name matches jni/Android.mk
	static {			
		System.loadLibrary("tagrecognizer-jni");
	}
	
	// native functions
	public native byte[] extractFAST(int[] pixels, int width, int height);
	public native byte[] tagRecognizer(int[] pixels, int width, int height);
	
	
	//llamada para paso de imagen directa
	public Bitmap openCV(Bitmap bitmap) {
		//prepare bitmap
		int width = bitmap.getWidth();
		int height = bitmap.getHeight();
		int[] pixels = new int[width * height];
		bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
		//call method jni
		//byte[] imageData = this.extractFAST(pixels, width, height);
		byte[] imageData = this.tagRecognizer(pixels, width, height);
		Bitmap bmp = null;
		if (imageData != null )
			bmp = BitmapFactory.decodeByteArray(imageData, 0, imageData.length);
		return bmp;
	}
	
}
