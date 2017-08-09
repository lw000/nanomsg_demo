#ifndef __common_struct_h__
#define __common_struct_h__

#include <string>
#include <vector>
#include <list>

#define ROOM_STATE_Empty	100		//	空
#define ROOM_STATE_Full		101		//	满
#define ROOM_STATE_idle		102		//	空闲
#define ROOM_STATE_Lock		103		//	锁房

#define DESK_STATE_Empty	100		//	空闲
#define DESK_STATE_Full		101		//	满
#define DESK_STATE_Idle		102		//	空闲
#define DESK_STATE_Lock		103		//	锁桌

#define GAME_STATE_Start	300		//
#define GAME_STATE_End		301		//

#define USER_STATE_Playing	400		// 
#define USER_STATE_Lookon	401		//

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tagUSER_INFO
{
	int uid;	// 用户ID
	int rid;	// 房间编号
	int seat;	// 座位编号
	int state;	// 用户状态

	int sex;			// 用户性别
	int age;			// 用户年龄
	
	std::string name;	// 用户姓名
	std::string addr;	// 用户地址
	std::string ext;	// 用户扩展信息
	
public:
	_tagUSER_INFO()
	{
		uid = -1;	// 用户ID
		sex = -1;	// 用户性别
		age = -1;	// 用户年龄

		rid = -1;	// 房间编号
		seat = -1;	// 座位编号
		state = -1;	// 用户状态
	}

} USER_INFO;

typedef struct _tagDESK_INFO
{
	int rid;		// 房间编号
	int did;		// 桌位编号
	int state;		// 桌子状态
	int max_usercount;	// 最大用户个数

	std::string name;	// 桌子名称
	std::string ext;	// 桌子扩展信息

public:
	_tagDESK_INFO()
	{
		rid = -1;		// 房间编号
		did = -1;		// 桌位编号
		state = -1;		// 桌子状态
		max_usercount = -1;	// 最大用户个数
	}

} DESK_INFO;

typedef struct _tagROOM_INFO
{
	int rid;	// 房间编号
	int state;	// 房间状态
	int deskcount;	// 桌子个数
	int usercount;	// 用户个数
	int max_usercount;	// 最大用户个数
	std::string name;	// 桌子名称
	std::string ext;	// 房间扩展信息

public:
	_tagROOM_INFO()
	{
		rid = -1;		// 房间编号
		state = -1;		// 房间状态
		deskcount = -1;	// 桌子个数

		usercount = -1;	// 用户个数
		max_usercount = -1;	// 最大用户个数
	}

} ROOM_INFO;

#endif	// !__common_struct_h__
