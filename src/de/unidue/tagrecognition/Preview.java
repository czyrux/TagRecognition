package de.unidue.tagrecognition;

import java.io.IOException;
import java.util.List;

import android.content.Context;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

class Preview extends SurfaceView implements SurfaceHolder.Callback {
	private static final String TAG = "Preview";
	SurfaceHolder mHolder;
	public Camera mCamera;

	Preview(Context context) {
		super(context);
		// Install a SurfaceHolder.Callback so we get notified when the
		// underlying surface is created and destroyed.
		mHolder = getHolder();
		mHolder.addCallback(this);
		mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
	}

	// Called once the holder is ready
	public void surfaceCreated(SurfaceHolder holder) {
		// The Surface has been created, acquire the camera and tell it where to
		// draw.
		mCamera = Camera.open();
		Camera.Parameters p = mCamera.getParameters();
		List<String> focusModes = p.getSupportedFocusModes();
		if (focusModes.contains(Camera.Parameters.FOCUS_MODE_AUTO)) {
			p.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
		}

		p.setPictureFormat(PixelFormat.JPEG);
		// best quality
		p.setJpegQuality(100);
		
		//max resolution
		int pos = (p.getSupportedPictureSizes().size() - 1);
		p.setPictureSize(p.getSupportedPictureSizes().get(pos).width, p
				.getSupportedPictureSizes().get(pos).height);

		Log.d(TAG, "Focus mode: " + p.getFocusMode());
		Log.d(TAG, "Picture size choosen: " + p.getPictureSize().height + "x"
				+ p.getPictureSize().width);
		mCamera.setParameters(p);

		/*
		 * for (Size i : p.getSupportedPictureSizes()) Log.d("debug", "size: " +
		 * i.height + "x" + i.width);
		 */

		try {
			mCamera.setPreviewDisplay(holder);
			mCamera.startPreview();
		} catch (IOException exception) {
			mCamera.release();
			mCamera = null;
			Log.d(TAG, "Error setting preview display: ");
			exception.printStackTrace();
		}

	}

	// Called when the holder is destroyed
	public void surfaceDestroyed(SurfaceHolder holder) {
		if (mCamera != null) {
			mCamera.stopPreview();
			mCamera.release();
			mCamera = null;
		}
	}

	// Get optimal size for preview
	private Size getOptimalPreviewSize(List<Size> sizes, int w, int h) {
		final double ASPECT_TOLERANCE = 0.05;
		double targetRatio = (double) w / h;
		if (sizes == null)
			return null;

		Size optimalSize = null;
		double minDiff = Double.MAX_VALUE;

		int targetHeight = h;

		// Try to find an size match aspect ratio and size
		for (Size size : sizes) {
			double ratio = (double) size.width / size.height;
			if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE)
				continue;
			if (Math.abs(size.height - targetHeight) < minDiff) {
				optimalSize = size;
				minDiff = Math.abs(size.height - targetHeight);
			}
		}

		// Cannot find the one match the aspect ratio, ignore the requirement
		if (optimalSize == null) {
			minDiff = Double.MAX_VALUE;
			for (Size size : sizes) {
				if (Math.abs(size.height - targetHeight) < minDiff) {
					optimalSize = size;
					minDiff = Math.abs(size.height - targetHeight);
				}
			}
		}
		return optimalSize;
	}

	// Called when holder has changed
	public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
		// stop preview before making changes-- Disable for problems with
		// Autofocus
		/*
		 * try { mCamera.stopPreview(); } catch (Exception e) { Log.d(TAG,
		 * "Error stopping camera preview: " + e.getMessage()); }
		 */

		Camera.Parameters parameters = mCamera.getParameters();
		// set preview size and make any resize, rotate or
		// reformatting changes here
		List<Size> sizes = parameters.getSupportedPreviewSizes();
		Size optimalSize = getOptimalPreviewSize(sizes, w, h);
		parameters.setPreviewSize(optimalSize.width, optimalSize.height);

		// start preview with new settings
		mCamera.setParameters(parameters);
		mCamera.startPreview();
	}

	public void release() {
		if (mCamera != null) {
			mCamera.release();
			mCamera = null;
		}
	}
}