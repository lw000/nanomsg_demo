#ifndef __command_H__
#define __command_H__

namespace LW 
{
	enum NET_CMD
	{
		cmd_heart_beat				= 100,
		cmd_connected				= 101,

		/*
			平台消息
		*/
		cmd_platform_cs_login		= 10100001,
		cmd_platform_sc_login		= 10110000,

		cmd_platform_cs_logout		= 10100002,
		cmd_platform_sc_logout		= 10120000,

		cmd_platform_cs_userinfo	= 10100003,
		cmd_platform_sc_userinfo	= 10130000,

		/*
			平台框架消息
		*/
		cmd_platform_frame_cs_join_room = 20000000,
		cmd_platform_frame_cs_leave_room = 20000001,
		cmd_platform_frame_cs_join_game = 20000002,
		cmd_platform_frame_cs_leave_game = 20000003,

		/*
			游戏框架消息
		*/
		
		cmd_game_frame_cs_game_start = 40400001,
		cmd_game_frame_sc_game_end = 40410000,

		cmd_game_frame_cs_sit_up = 40400002,
		cmd_game_frame_sc_sit_up = 40420000,

		cmd_game_frame_cs_sit_down = 40400003,
		cmd_game_frame_sc_sit_down = 40430000,

	};
}

#endif // !__command_H__
