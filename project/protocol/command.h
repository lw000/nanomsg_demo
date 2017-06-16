#ifndef __command_H__
#define __command_H__

namespace LW 
{
	enum NET_CMD
	{
		cmd_heart_beat				= 100,

		/*
			平台消息
		*/

		cmd_platform_connected		= 101,

		cmd_platform_cs_login		= 10100001,
		cmd_platform_sc_login		= 10110000,

		cmd_platform_cs_logout		= 10100002,
		cmd_platform_sc_logout		= 10120000,

		cmd_platform_cs_userinfo	= 10100003,
		cmd_platform_sc_userinfo	= 10130000,


		/*
			游戏消息
		*/
		
		cmd_game_connected		= 202,
		cmd_game_cs_login		= 20200001,
		cmd_game_sc_login		= 20210000,

		cmd_game_cs_logout		= 20200002,
		cmd_game_sc_logout		= 20220000,
	};
}

#endif // !__command_H__
