/* $Id$ */
/*
   Copyright (C) 2012 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SERVER_ROOMS_HPP_INCLUDED
#define SERVER_ROOMS_HPP_INCLUDED

#include "player_connection.hpp"

#include <boost/bimap/unordered_multiset_of.hpp>

class Room : public boost::noncopyable
{
	std::string name_;

	public:
	Room(const std::string& name);
	~Room();
};

typedef boost::shared_ptr<Room> room_ptr;

class RoomList : public boost::noncopyable
{
	room_ptr lobby_;

	typedef boost::bimaps::bimap<
		boost::bimaps::unordered_multiset_of<socket_ptr>, 
		boost::bimaps::unordered_multiset_of<std::string>,
		boost::bimaps::set_of_relation<>,
		boost::bimaps::with_info<room_ptr>
	> RoomMap;
	RoomMap room_map_;
	PlayerMap player_connections_;
	public:
	RoomList(PlayerMap& player_connections);

	void enter_room(const std::string& room_name, socket_ptr socket);
	void leave_room(const std::string& room_name, socket_ptr socket);
};

#endif
