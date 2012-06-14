package de.unidue.tagrecognition;

import android.graphics.Bitmap;
import android.graphics.Color;

public class ColorFilter {

	// Apply color to a bitmap
	public static Bitmap extractRed(Bitmap src, double threshold) {
		// image size
		int width = src.getWidth();
		int height = src.getHeight();

		// create output bitmap
		Bitmap bmOut = Bitmap.createBitmap(width, height, src.getConfig());

		// color information
		//int colorR = Color.red(Color.BLUE), colorG = Color.green(Color.BLUE), colorB = Color.blue(Color.BLUE);
		int pixel;

		//TRY TYPE OF FILTER IN RANGE OF COLOR
		//SEE HOW TO OPENCV MANAGE THAT
		
		// scan through all pixels
		for (int x = 0; x < width; ++x) {
			for (int y = 0; y < height; ++y) {
				// get pixel color
				pixel = src.getPixel(x, y);
				/*
				 * int difference = Math.abs(Math.abs(Color.red(pixel) - colorR)
				 * + Math.abs(Color.green(pixel) - colorG) +
				 * Math.abs(Color.blue(pixel) - colorB));
				 */
				int difference = Color.red(pixel)
						- (Color.blue(pixel) + Color.green(pixel)); // to show only red color
				if (difference > threshold) {
				//if ( Color.red(pixel) > 50 && Color.blue(pixel) < 50 &&  Color.green(pixel) < 50) {
					bmOut.setPixel(x, y, Color.BLACK);
				} else {
					bmOut.setPixel(x, y, Color.WHITE);
				}
			}
		}

		// return final image
		return bmOut;
	}

	public static Bitmap blackWhite(Bitmap src) {
		Bitmap bmOut = Bitmap.createBitmap(src.getWidth(), src.getHeight(),
				src.getConfig());

		int A, R, G, B;
		int pixelBW, pixelColor;

		for (int x = 0; x < src.getWidth(); x++) {
			for (int y = 0; y < src.getHeight(); y++) {
				pixelColor = src.getPixel(x, y);
				A = Color.alpha(pixelColor);
				R = Color.red(pixelColor);
				G = Color.green(pixelColor);
				B = Color.blue(pixelColor);

				// value in grey scale
				pixelBW = (R + G + B) / 3;
				// set value
				bmOut.setPixel(x, y, Color.argb(A, pixelBW, pixelBW, pixelBW));
			}
		}

		return bmOut;
	}
}
