package de.unidue.tagrecognition;

/**
 * @file Message.java
 * @brief Commands and message exchanged between both applications
 * @author Antonio Manuel Gutierrez Martinez
 * @version 1.0
 */
public enum Message {
	ACK,
	START_SEARCH,
	STOP_SEARCH,
	CALIBRATE,
	SEND_VIEW,
	CALIBRATION_OK,
	CALIBRATION_FAIL,
	QUIT;
}
