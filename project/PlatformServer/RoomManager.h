#ifndef __room_mgr_h__
#define __room_mgr_h__

#include <vector>
#include <list>

class Room;

class RoomManager
{
private:
	std::list<Room> _rooms;

public:
	RoomManager();
	~RoomManager();
};

#endif	// !__room_mgr_h__
