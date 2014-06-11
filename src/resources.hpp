/*
   Copyright (C) 2009 - 2014 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef RESOURCES_H_
#define RESOURCES_H_

#include <vector>

class game_board;
class game_config_manager;
class game_display;
class display_context;
class gamemap;
class game_data;
class LuaKernel;
class play_controller;
class team;
class tod_manager;
class unit_map;
class persist_manager;
class game_classification;
struct mp_game_settings;
namespace actions { class undo_list; }

namespace soundsource { class manager; }

namespace pathfind { class manager; }

namespace wb { class manager; } //whiteboard manager

namespace resources
{
	extern game_config_manager    *config_manager;
	extern play_controller        *controller;
	extern game_board             *gameboard;
	extern game_data              *gamedata;
	extern LuaKernel              *lua_kernel;     // Set by game_events::manager.
	extern persist_manager        *persist;
	extern game_classification    *classification;
	extern game_display           *screen;
	extern const display_context  *disp_context;
	extern const mp_game_settings *mp_settings;
	extern soundsource::manager   *soundsources;
	extern std::vector<team>      *teams;
	extern ::tod_manager          *tod_manager;
	extern pathfind::manager      *tunnels;
	extern actions::undo_list     *undo_stack;
	extern unit_map               *units;
	extern wb::manager            *whiteboard;
}

#endif
