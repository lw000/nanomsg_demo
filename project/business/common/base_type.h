﻿#ifndef __BaseType_H__
#define __BaseType_H__

typedef		bool					lw_bool;

typedef		char					lw_char8;
typedef		unsigned char			lw_byte8;

typedef		unsigned short			lw_ushort16;

typedef		short					lw_short16;

typedef		int						lw_int32;
typedef		unsigned int			lw_uint32;

typedef		long long				lw_llong64;
typedef		unsigned long long		lw_ullong64;

typedef		double					lw_double;

enum enMsgStatus
{
	msgStatus_UNKNOW			= 0,
	msgStatus_CONNECTING,
	msgStatus_TIMEOUT,
	msgStatus_CONNECED,
	msgStatus_RECV,
	msgStatus_DISCONNECT,
	msgStatus_ERROR
};

#endif	//__BaseType_H__
