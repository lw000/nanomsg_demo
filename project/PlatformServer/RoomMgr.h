#ifndef __room_mgr_h__
#define __room_mgr_h__

#include <vector>
#include <list>

class Room;

class RoomMgr
{
private:
	std::list<Room> _rooms;

public:
	RoomMgr();
	~RoomMgr();
};

#endif	// !__room_mgr_h__
