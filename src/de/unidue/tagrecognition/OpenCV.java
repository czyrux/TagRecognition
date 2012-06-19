package de.unidue.tagrecognition;


import android.graphics.Bitmap;
import android.graphics.BitmapFactory;


public class OpenCV {
	// load the library - name matches jni/Android.mk
	static {			
		System.loadLibrary("opencv");
	}
	
	// native functions
	//public native void extractSURFFeature();
	public native void extract();
	public native boolean setSourceImage(int[] pixels, int width, int height);
	public native byte[] getSourceImage();
	public native void releaseSourceImage();
	
	public native byte[] extractFAST(int[] pixels, int width, int height);
	public native byte[] square(int[] pixels, int width, int height);
	
	//SLOW
	public Bitmap opencvImage(Bitmap bitmap) {
		//prepare bitmap
		int width = bitmap.getWidth();
		int height = bitmap.getHeight();
		int[] pixels = new int[width * height];
		bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
		
		//set bitmap
		this.setSourceImage(pixels, width, height);
		//call method jni
		this.extract();
		//get bitmap
		byte[] imageData = this.getSourceImage();
		this.releaseSourceImage();
		Bitmap bmp = BitmapFactory.decodeByteArray(imageData, 0, imageData.length);

		return bmp;
	}
	
	//llamada para paso de imagen directa
	public Bitmap openCV(Bitmap bitmap) {
		//prepare bitmap
		int width = bitmap.getWidth();
		int height = bitmap.getHeight();
		int[] pixels = new int[width * height];
		bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
		//call method jni
		//byte[] imageData = this.extractFAST(pixels, width, height);
		byte[] imageData = this.square(pixels, width, height);
		Bitmap bmp = null;
		if (imageData != null )
			bmp = BitmapFactory.decodeByteArray(imageData, 0, imageData.length);
		return bmp;
	}
	
}
