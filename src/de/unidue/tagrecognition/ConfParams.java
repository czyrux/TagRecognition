package de.unidue.tagrecognition;

import java.io.Serializable;

public class ConfParams implements Serializable {
	private static final long serialVersionUID = 1L;
	public static int rows = 2;
	public static int cols = 4;
	public static float tag_width = 10.0f;
	public static float tag_height = 4.5f;
	public static float tag_border = 1.0f;
	public static String desk_IP = "192.168.137.1";
	public static int desk_Port = 8080;
	public static int my_Port = 8000;
	public static boolean debugMode = true;
}
