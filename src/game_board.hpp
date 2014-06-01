/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GAME_BOARD_HPP_INCLUDED
#define GAME_BOARD_HPP_INCLUDED

#include "global.hpp"

#include "map.hpp"
#include "team.hpp"
#include "unit_map.hpp"

#include <vector>

class config;

struct game_board {
	game_board(const config & game_config, const config & level) : teams_(), map_(game_config, level), units_() {}

	std::vector<team> teams_;

	gamemap map_;
	unit_map units_;

	void new_turn(int pnum);
	void end_turn(int pnum);
	void set_all_units_user_end_turn();

	void write_config(config & cfg) const;
};

#endif
