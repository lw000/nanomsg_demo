#include "Desk.h"

Desk::Desk()
{
	
}

Desk::~Desk()
{

}

bool Desk::create(const DESK_INFO& info)
{
	this->desk_info = info;

	return true;
}

void Desk::destroy()
{

}


void Desk::onGameStartReponse(void* data, int datasize)
{

}

void Desk::onGameEndReponse(void* data, int datasize)
{

}

void Desk::onGameUserSitupReponse(void* data, int datasize)
{

}

void Desk::onGameUserSitdownReponse(void* data, int datasize)
{

}

int Desk::onGameMessage(int cmd, void* data, int datasize)
{

	return 0;
}
