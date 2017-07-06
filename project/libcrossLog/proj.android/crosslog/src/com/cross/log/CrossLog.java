package com.cross.log;

public class CrossLog {
	public static native void hnConfigFastLog(String path);
	public static native void hnStartFastLog();
	public static native void hnStopLog();
	public static native void hnPrintLogD(String log);
	public static native void hnPrintLogI(String log);
	public static native void hnPrintLogW(String log);
	public static native void hnPrintLogE(String log);
}
