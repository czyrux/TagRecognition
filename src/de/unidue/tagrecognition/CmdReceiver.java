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

/**
 * @file CmdReceiver.java
 * @brief Extends IntentService. Handles the server work. 
 * @author Antonio Manuel Gutierrez Martinez
 * @version 1.0
 */
public class CmdReceiver extends IntentService {
	private final static String TAG = "CmdReceiver";
	public static final String CmdReceiver_IN_MSG = "imsg";
	public static final String CmdReceiver_PORT_MSG = "port_msg";
	public static final String CmdReceiver_OUT_MSG = "outmsg";
	public static final int PARAM_START = 1;
	public static final int PARAM_STOP = 2;
	private static ServerThread _server = null;

	/**
	 * Constructor
	 */
	public CmdReceiver() {
		super("CmdReceiver");
	}

	/**
	 * Manage Intent message
	 */
	@Override
	protected void onHandleIntent(Intent intent) {
		// Get intent value
		int cmd = intent.getIntExtra(CmdReceiver_IN_MSG, 0);
		int port;
		// Select operation
		switch (cmd) {
		case PARAM_START:
			port = intent.getIntExtra(CmdReceiver_PORT_MSG, 8000);
			if (_server == null) {
				_server = new ServerThread(port);
				Thread sf = new Thread(_server);
				sf.start();
			}
			break;
		case PARAM_STOP:
			if (_server != null) {
				_server.closeServer();
				_server = null;
			}
			break;
		default:
			break;
		}
	}

	/**
	 * Send the command received to the main activity
	 * @param cmd
	 */
	void sendCmd(String cmd) {
		Intent i = new Intent();
		Bundle bundle = new Bundle();
		bundle.putString(CmdReceiver_OUT_MSG, cmd);
		i.putExtras(bundle);
		i.setAction(CmdReceiver.CmdReceiver_OUT_MSG);
		this.sendBroadcast(i);
	}

	/**
	 * @brief Implements runnable server thread. 
	 * @author Antonio Manuel Gutierrez Martinez
 	 * @version 1.0
	 */
	public class ServerThread implements Runnable {
		/** Server port */
		private int SERVERPORT = 8000;
		/** Server socket */
		private ServerSocket _serverSocket;

		/**
		 * Constructor
		 * @param port where create the server
		 */
		public ServerThread(int port) {
			SERVERPORT = port;
			_serverSocket = null;
		}

		/**
		 * Thread process
		 */
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

		/**
		 * Close server
		 */
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