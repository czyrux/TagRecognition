package de.unidue.tagrecognition;

import java.io.Serializable;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * @file Tag.java
 * @brief Implementation of a serializable tag object. A tag object is the 
 * digital representation of the tag data.
 * @author Antonio Manuel Gutierrez Martinez
 * @version 1.0
 */
public class Tag implements Serializable {
	private static final long serialVersionUID = 1L;
	/** X coordinate in the image. It's a % respect to the image width. */
	private float _x;
	/** Y coordinate in the image. It's a % respect to the image height. */
	private float _y;
	/** String that identify the tag  */
	private String _code;
	/** Date where the tag was localized. */
	private Date _time;

	/**
	 * Constructor
	 */
	public Tag(float x, float y, String code, Date time) {
		_x = x;
		_y = y;
		_code = code;
		_time = time;
	}

	/**
	 * Get X value
	 */
	public float getX() {
		return _x;
	}

	/**
	 * Set X value
	 */
	public void setX(float x) {
		this._x = x;
	}

	/**
	 * Get Y value
	 */
	public float getY() {
		return _y;
	}

	/**
	 * Set Y value
	 */
	public void setY(float y) {
		this._y = y;
	}

	/**
	 * Get tag code
	 */
	public String getCode() {
		return _code;
	}

	/**
	 * Set tag code
	 */
	public void setCode(String code) {
		this._code = code;
	}

	/**
	 * Get tag date
	 */
	public Date getTime() {
		return _time;
	}

	/**
	 * Set tag date
	 */
	public void setTime(Date time) {
		this._time = time;
	}


	/**
	 * Convert tag information to string
	 */
	@Override
	public String toString() {
		String timeStamp = new SimpleDateFormat("HH:mm:ss dd/MM/yyyy")
				.format(new Date());
		return "Tag [x=" + _x + ", y=" + _y + ", code=" + _code + " t:"
				+ timeStamp + "]";
	}

}
