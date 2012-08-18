package de.unidue.tagrecognition;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;

import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.util.Log;

/**
 * @file DataSender.java
 * @brief Asynctask that send data to a remote server via socket.
 * @author Antonio M. Gutierrez Martinez
 * @reference Based on the code: {@link http
 *            ://stackoverflow.com/questions/5135438
 *            /example-android-bi-directional-network-socket-using-asynctask}
 * @version 1.0
 */
@SuppressWarnings("rawtypes")
public class DataSender extends AsyncTask<ArrayList, Void, Boolean> {
	private static final String TAG = "SenderTags";
	/** Server Host */
	private String HOST;
	/** Server port */
	private int PORT;
	/** Socket object */
	private Socket _socket = null;
	/** Output stream to socket communication. */
	private ObjectOutputStream _out = null;
	/** Input stream to socket communication. */
	private ObjectInputStream _in = null;

	/**
	 * Constructor
	 * @param host Server host
	 * @param port Server port
	 */
	public DataSender (String host , int port ) {
		HOST = host;
		PORT = port;
	}
	
	/**
	 * Send the data to the server
	 * Process made in an indepent thread.
	 * @return Return true if the information could be sent, false otherwise.
	 */
	@Override
	protected Boolean doInBackground(ArrayList... params) {
		ArrayList objects = params[0];
		boolean sent = true;
		Object o = null;
		try {
			// Create socket
			SocketAddress sockaddr = new InetSocketAddress(HOST, PORT);
			_socket = new Socket();
			_socket.connect(sockaddr, 2000); // 2 second connection timeout

			if (_socket.isConnected()) {
				Log.i(TAG, "Client connected");
				// Open streams
				_out = new ObjectOutputStream(_socket.getOutputStream());
				_in = new ObjectInputStream(_socket.getInputStream());
				Log.i(TAG, "Streams created");

				// Send objects
				for (int i = 0; i < objects.size(); i++) {
					if (objects.get(i) instanceof Bitmap) {
						sendImage(((Bitmap)objects.get(i)));
					}else {
						if (objects.get(i) instanceof Tag)
							Log.i(TAG, "Sending tag: " + objects.get(i).toString());

						// Send object
						_out.writeObject(objects.get(i));
						_out.flush();
					}

					// Wait confirmation
					try {
						Log.i(TAG, "Waiting ACK");
						o = _in.readObject();
						if (o.equals(Message.ACK.toString())) {
							Log.i(TAG, "ACK received");
						}
					} catch (ClassNotFoundException e) {
						Log.i(TAG, "ClassNotFoundException");
						e.printStackTrace();
					}

					// If it is the last close communication
					if (i == objects.size() - 1) {
						Log.i(TAG, "Sending end of communication");
						_out.writeObject(Message.QUIT.toString());
						_out.flush();

						try {
							Log.i(TAG, "Waiting ACK");
							o = _in.readObject();
							if (o.equals(Message.ACK.toString())) {
								Log.i(TAG, "ACK received");
							}
						} catch (ClassNotFoundException e) {
							Log.i(TAG, "ClassNotFoundException");
							e.printStackTrace();
						}
					}
				}// end for
			}

		} catch (UnknownHostException e) {
			Log.i(TAG, "UnknownHostException");
			sent = false;
			e.printStackTrace();
		} catch (IOException e) {
			Log.i(TAG, "IOException");
			sent = false;
			e.printStackTrace();
		} finally {
			// Close sockets and streams
			try {
				if (_in != null)
					_in.close();
				if (_out != null)
					_out.close();
				if (_socket != null)
					_socket.close();
			} catch (IOException e) {
				Log.i(TAG, "IOException closing");
				e.printStackTrace();
			} catch (Exception e) {
				Log.i(TAG, "Exception");
				e.printStackTrace();
			}
			Log.i(TAG, "Finished");
		}

		return sent;
	}

	/**
	 * Send a Bitmap over the socket stream
	 * @param bmp Image
	 * @throws IOException
	 */
	private void sendImage( Bitmap bmp ) throws IOException {
		ByteArrayOutputStream stream = new ByteArrayOutputStream();
		bmp.compress(Bitmap.CompressFormat.JPEG, 100, stream);
		byte[] byteArray = stream.toByteArray();
		_out.writeObject(byteArray);
		_out.flush();
	}

	/**
	 * Close the socket stream if the asynctask is cancelled.
	 */
	@Override
	protected void onCancelled() {
		try {
			_in.close();
			_out.close();
			_socket.close();
		} catch (IOException e) {
			Log.i(TAG, "onCancelled: IOException");
			e.printStackTrace();
		} catch (Exception e) {
			Log.i(TAG, "onCancelled: Exception");
			e.printStackTrace();
		}

		super.onCancelled();
	}

}
