package de.unidue.tagrecognition;

import java.io.Serializable;
import java.text.SimpleDateFormat;
import java.util.Date;

public class Tag implements Serializable {
	private static final long serialVersionUID = 1L;
	private float _x;
	private float _y;
	private String _code;
	private Date _time;

	public Tag(float x, float y, String code, Date time) {
		_x = x;
		_y = y;
		_code = code;
		_time = time;
	}

	public float getX() {
		return _x;
	}

	public void setX(float x) {
		this._x = x;
	}

	public float getY() {
		return _y;
	}

	public void setY(float y) {
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
