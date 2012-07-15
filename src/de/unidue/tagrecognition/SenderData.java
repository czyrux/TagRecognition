package de.unidue.tagrecognition;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;

import android.util.Log;

public class SenderData {
	private final String HOST = "192.168.137.1";
	private final int PORT = 8080;
	Socket _socket = null;
	ObjectOutputStream _out = null;
	BufferedReader _in = null;

	public SenderData() throws UnknownHostException, IOException , SocketException {
		// Create socket
		_socket = new Socket(HOST, PORT);
		_out = new ObjectOutputStream(_socket.getOutputStream());
		_in = new BufferedReader(new InputStreamReader(_socket.getInputStream()));
	}

	public void sendData(ArrayList<Tag> data) throws IOException {
		Log.d("SENDER","Send data method");
		// Send array of tags
		for ( Tag t : data ) {
			Log.d("SENDER","Sending: " + t.toString() );
			_out.writeObject(t);
			_out.flush();
		}
		
		// Wait for confirmation
	}

	public void closeConnection() {
		try {
			_out.close();
			_in.close();
			_socket.close();
		} catch (IOException e) {
			e.printStackTrace();
			Log.e("SENDER", "Error closing streams");
		}
	}
}
