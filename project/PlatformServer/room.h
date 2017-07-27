#ifndef __room_h__
#define __room_h__

#include <vector>

#include "common_struct.h"

class Desk;

class Room
{
private:
	ROOM_INFO _room_info;

private:
	std::vector<Desk*> _desks;	// ×À×Ó

public:
	Room();
	virtual ~Room();

public:
	bool create(const ROOM_INFO& info);
	void destroy();

};

#endif
