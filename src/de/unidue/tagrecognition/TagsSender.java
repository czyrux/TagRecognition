package de.unidue.tagrecognition;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;

import android.os.AsyncTask;
import android.util.Log;

/**
 * 
 * @author Antonio M. Gutierrez Martinez
 * @reference Based on the code: {@link http
 *            ://stackoverflow.com/questions/5135438
 *            /example-android-bi-directional-network-socket-using-asynctask}
 * 
 */
public class TagsSender extends AsyncTask<ArrayList<Tag>, Void, Boolean> {
	private static final String TAG = "SenderTags";
	private final String HOST = "192.168.137.1";
	private final int PORT = 8080;
	private Socket _socket = null;
	private ObjectOutputStream _out = null;
	private ObjectInputStream _in = null;

	@Override
	protected Boolean doInBackground(ArrayList<Tag>... params) {
		ArrayList<Tag> tags = params[0];
		boolean sent = true;
		Object o = null;
		try {
			// Create socket
			SocketAddress sockaddr = new InetSocketAddress(HOST, PORT);
			_socket = new Socket();
			_socket.connect(sockaddr, 2000); // 2 second connection timeout

			if (_socket.isConnected()) {
				Log.i(TAG, "doInBackground: Client connected");
				// Open streams
				_out = new ObjectOutputStream(_socket.getOutputStream());
				_in = new ObjectInputStream(_socket.getInputStream());
				Log.i(TAG, "doInBackground: stream created");

				// Send array of tags
				for (int i=0 ; i<tags.size() ; i++) {
					Log.i(TAG, "doInBackground: Sending tag: " + tags.get(i).toString());

					// Send tag
					_out.writeObject(tags.get(i));
					_out.flush();

					// Wait confirmation
					try {
						Log.i(TAG, "doInBackground: Waiting ACK");
						o = _in.readObject();
						if (o.equals(CMD.ACK.toString())) {
							Log.i(TAG, "doInBackground: ACK received");
						}
					} catch (ClassNotFoundException e) {
						Log.i(TAG, "doInBackground: ClassNotFoundException");
						e.printStackTrace();
					}
					
					//If it is the last close communication
					if ( i == tags.size()-1 ) {
						Log.i(TAG, "doInBackground: Sending end of communication");
						_out.writeObject(CMD.QUIT.toString());
						_out.flush();

						try {
							o = _in.readObject();
							if (o.equals(CMD.ACK.toString())) {
								Log.i(TAG, "doInBackground: ACK received");
							}
						} catch (ClassNotFoundException e) {
							Log.i(TAG, "doInBackground: ClassNotFoundException");
							e.printStackTrace();
						}
					}
				}//end for
			}

		} catch (UnknownHostException e) {
			Log.i(TAG, "doInBackground: UnknownHostException");
			sent = false;
			e.printStackTrace();
		} catch (IOException e) {
			Log.i(TAG, "doInBackground: IOException");
			sent = false;
			e.printStackTrace();
		} finally {
			//Close sockets and streams
			try {
				if (_in != null)
					_in.close();
				if (_out != null)
					_out.close();
				if (_socket != null)
					_socket.close();
			} catch (IOException e) {
				Log.i(TAG, "doInBackground: IOException");
				e.printStackTrace();
			} catch (Exception e) {
				Log.i(TAG, "doInBackground: Exception");
				e.printStackTrace();
			}
			Log.i(TAG, "doInBackground: Finished");
		}

		return sent;
	}

	@Override
	protected void onCancelled() {
		try {
			_in.close();
			_out.close();
			_socket.close();
		} catch (IOException e) {
			Log.i(TAG, "doInBackground: IOException");
			e.printStackTrace();
		} catch (Exception e) {
			Log.i(TAG, "doInBackground: Exception");
			e.printStackTrace();
		}

		super.onCancelled();
	}

}
