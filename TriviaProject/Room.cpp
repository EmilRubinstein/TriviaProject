#include "Room.h"

Room::Room(RoomData data) : m_metadata(data)
{

}

RoomData Room::getRoomData()
{
	return m_metadata;
}

bool Room::userExists(std::string username)
{
	for (auto& it : m_users)
	{
		if (it.getUsername() == username)
			return true;
	}
	return false;
}

void Room::addUser(LoggedUser user)
{
	if (!userExists(user.getUsername()))
		m_users.push_back(user);

}

void Room::removeUser(LoggedUser user)
{
	for (auto it = m_users.begin(); it != m_users.end(); ++it)
	{
		if (it->getUsername() == user.getUsername())
		{
			m_users.erase(it);
			break;
		}
	}
}

std::vector<std::string> Room::getAllUsers()
{
	std::vector<std::string> m;
	for (auto &it : m_users)
	{
		m.push_back(it.getUsername());
	}
	return m;
	
}

void Room::setStatus(RoomStatus status)
{
	m_metadata.status = status;
}