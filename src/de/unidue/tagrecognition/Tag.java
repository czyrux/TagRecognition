package de.unidue.tagrecognition;

import java.io.Serializable;
import java.text.SimpleDateFormat;
import java.util.Date;

public class Tag implements Serializable {
	private static final long serialVersionUID = 1L;
	private int _x;
	private int _y;
	private String _code;
	private Date _time;

	public Tag(int x, int y, String code, Date time) {
		_x = x;
		_y = y;
		_code = code;
		_time = time;
	}

	public int getX() {
		return _x;
	}

	public void setX(int x) {
		this._x = x;
	}

	public int getY() {
		return _y;
	}

	public void setY(int y) {
		this._y = y;
	}

	public String getCode() {
		return _code;
	}

	public void setCode(String code) {
		this._code = code;
	}

	public Date getTime() {
		return _time;
	}

	public void setTime(Date time) {
		this._time = time;
	}

	@Override
	public String toString() {
		String timeStamp = new SimpleDateFormat("HH:mm:ss dd/MM/yyyy")
				.format(new Date());
		return "Tag [x=" + _x + ", y=" + _y + ", code=" + _code + " t:"
				+ timeStamp + "]";
	}

}
