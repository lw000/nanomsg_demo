#include "room.h"
#include "Desk.h"


Room::Room()
{

}

Room::~Room()
{

}

bool Room::create(const ROOM_INFO& info)
{
	this->_room_info = info;

	for (int i = 0; i < _room_info.deskcount; i++)
	{
		DESK_INFO desk_info;
		desk_info.did = i;
		desk_info.max_usercount = 6;
		char buf[64];
		sprintf(buf, "²âÊÔ[%d]", i);
		desk_info.name = std::string(buf);
		desk_info.rid = _room_info.rid;
		desk_info.state = DESK_STATE_Empty;
		Desk * pDesk = new Desk();
		if (pDesk->create(desk_info))
		{
			_desks.push_back(pDesk);
		}
	}

	return true;
}

void Room::destroy()
{
	std::vector<Desk*>::iterator iter = _desks.begin();
	for (; iter != _desks.end(); ++iter)
	{
		Desk* pDesk = *iter;
		delete pDesk;
	}
	std::vector<Desk*>().swap(_desks);
}