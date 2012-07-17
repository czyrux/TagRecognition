package de.unidue.tagrecognition;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;

import android.os.AsyncTask;
import android.util.Log;

public class NetworkTask extends AsyncTask<Void, Void, Void> {
	private static final String TAG = "NetworkTask";
	public static final int SERVERPORT = 8079;
	private ServerSocket _serverSocket;
	
	@Override
	protected Void doInBackground(Void... arg0) {
		Log.d(TAG, "Server created");
		
		try {
			_serverSocket = new ServerSocket(SERVERPORT);
			Log.d(TAG, "Server created");
			while (true) {
				Log.d(TAG, "Server listening");
                // listen for incoming clients
                Socket _client = _serverSocket.accept();
                Log.d(TAG, "Client connected");

                //Process the client
                try {
                	//Create stream for communication
                	ObjectInputStream in = new ObjectInputStream(_client.getInputStream());
                	ObjectOutputStream out = new ObjectOutputStream(_client.getOutputStream());
                    
                	//read command
                	String cmd = (String)in.readObject();
                	Log.d(TAG, "Received: " + cmd);
                	
                	//send confirmation
                	out.writeObject(new String("ACK"));
                	out.flush();
                	
                	//close streams
                	in.close();
                	out.close();

                } catch (Exception e) {
                	Log.d(TAG, "Oops. Connection interrupted. Please reconnect your phones.");
                    e.printStackTrace();
                }
            }
			
		} catch (IOException e) {
			Log.d(TAG, "Server aborted");
			try {
				Log.d(TAG, "Lo cerramos");
				_serverSocket.close();
			} catch (IOException e1) {
				Log.d(TAG, "Fallo cerrarlo");
				e1.printStackTrace();
			}
			e.printStackTrace();
		}
		
		return null;
	}

	public void closeServer() {
		try {
			_serverSocket.close();
		} catch (IOException e1) {
			Log.d(TAG, "Fallo cerrarlo");
			e1.printStackTrace();
		}
	}
}
