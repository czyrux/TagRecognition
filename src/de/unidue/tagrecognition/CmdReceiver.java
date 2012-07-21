package de.unidue.tagrecognition;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;

import android.app.IntentService;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

public class CmdReceiver extends IntentService {
	private final static String TAG = "CmdReceiver";
	public static final String CmdReceiver_IN_MSG = "imsg";
	public static final String CmdReceiver_OUT_MSG = "outmsg";
	public static final int PARAM_START = 1;
	public static final int PARAM_STOP = 2;
	private static ServerThread _server = null;

	public CmdReceiver() {
		super("CmdReceiver");
	}

	@Override
	protected void onHandleIntent(Intent intent) {
		// Get intent value
		int cmd = intent.getIntExtra(CmdReceiver_IN_MSG, 0);
		// Select operation
		switch (cmd) {
		case PARAM_START:
			runServer();
			break;
		case PARAM_STOP:
			stopServer();
			break;
		default:
			break;
		}
	}

	void runServer() {
		if (_server == null) {
			_server = new ServerThread();
			Thread sf = new Thread(_server);
			sf.start();
		}
	}

	void stopServer() {
		if (_server != null) {
			_server.closeServer();
			_server = null;
		}
	}

	void sendCmd(String cmd) {
		Intent i = new Intent();
		Bundle bundle = new Bundle();
		bundle.putString(CmdReceiver_OUT_MSG, cmd);
		i.putExtras(bundle);
		i.setAction(CmdReceiver.CmdReceiver_OUT_MSG);
		this.sendBroadcast(i);
	}

	public class ServerThread implements Runnable {
		private final int SERVERPORT = 8000;
		private ServerSocket _serverSocket;

		public ServerThread() {
			_serverSocket = null;
		}

		public void run() {
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
							// Send cmd to main activity
							sendCmd((String) o);
						}

						// send confirmation
						Log.i(TAG, "Sending ACK");
						out.writeObject(Message.ACK.toString());
						out.flush();

						// close streams
						in.close();
						out.close();

					} catch (IOException e) {
						Log.i(TAG, "IOException. Connect wait aborted.");
						end = true;
						// e.printStackTrace();
					} catch (Exception e) {
						Log.i(TAG, "Exception. Connection interrupted.");
						e.printStackTrace();
					}
				}// while
			}// if (_serverSocket != null)

		}

		public void closeServer() {
			if (_serverSocket != null) {
				try {
					_serverSocket.close();
					Log.i(TAG, "ServerThread: Server closed.");
				} catch (IOException e) {
					Log.i(TAG, "ServerThread: Could not close socket");
				}
			}
		}
	}

}