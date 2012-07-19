package de.unidue.tagrecognition;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;

import android.os.AsyncTask;
import android.os.Handler;
import android.util.Log;

public class CmdReceiver extends AsyncTask<Void, String, Void> {
	private final static String TAG = "CmdReceiver";
	private final int SERVERPORT = 8000;
	private ServerSocket _serverSocket;
	private TagRecognitionActivity _activity;
	private Handler _handler;

	public CmdReceiver(TagRecognitionActivity a, Handler handler) {
		_activity = a;
		_serverSocket = null;
		_handler = handler;
	}

	@Override
	protected Void doInBackground(Void... arg0) {
		try {
			_serverSocket = new ServerSocket(SERVERPORT);
			_serverSocket.setReuseAddress(true);
			Log.i(TAG, "Server created");
		} catch (IOException e) {
			Log.i(TAG, "IOException. Unable to create server");
			e.printStackTrace();
			_serverSocket = null;
		}

		if (_serverSocket != null) {
			Object o;
			boolean end = false;
			while (!end) {
				try {
					// listen for incoming clients
					Log.d(TAG, "Server listening: " + SERVERPORT);
					Socket _client = _serverSocket.accept();

					// Process the client
					Log.i(TAG, "Client connected");

					// Create stream for communication
					ObjectInputStream in = new ObjectInputStream(
							_client.getInputStream());
					ObjectOutputStream out = new ObjectOutputStream(
							_client.getOutputStream());

					// read command
					Log.i(TAG, "Waiting command...");
					o = in.readObject();
					if (o instanceof String) {
						Log.i(TAG, "Received: " + o.toString());
						// Send to main activity
						onProgressUpdate((String) o);
					}

					// send confirmation
					Log.i(TAG, "Sending ACK");
					out.writeObject(CMD.ACK.toString());
					out.flush();

					// close streams
					in.close();
					out.close();

				} catch (IOException e) {
					Log.i(TAG, "IOException. Connect wait aborted.");
					end = true;
					//e.printStackTrace();
				} catch (Exception e) {
					Log.i(TAG, "Exception. Connection interrupted.");
					e.printStackTrace();
				}
			}// while
		}

		return null;
	}

	@Override
	protected void onProgressUpdate(String... values) {
		String cmd = values[0];
		if (cmd.equals(CMD.CALIBRATE.toString())) {
			_handler.post(new Runnable() {
				@Override
				public void run() {
					_activity.functionCalibrate();
				}

			});

		} else if (cmd.equals(CMD.START_SEARCH.toString())) {
			_handler.post(new Runnable() {
				@Override
				public void run() {
					_activity.functionSearch();
				}

			});
		} else if (cmd.equals(CMD.STOP_SEARCH.toString())) {
			_handler.post(new Runnable() {
				@Override
				public void run() {
					_activity.functionStopSearch();
				}

			});
		}
	}

	public void closeServer() {
		try {
			if (_serverSocket != null) {
				_serverSocket.close();
				_serverSocket = null;
			}
			Log.i(TAG, "Server closed.");
		} catch (IOException e) {
			Log.i(TAG, "Could not close server.");
			e.printStackTrace();
		}
	}

	@Override
	protected void onCancelled() {
		closeServer();
		super.onCancelled();
	}
}
