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

#include <boost/optional.hpp>
#include <vector>

class config;

namespace events {
	class mouse_handler;
	class menu_handler;
}

/**
 *
 * Game board class.
 *
 * The purpose of this class is to encapsulate some of the core game logic, including the unit map,
 * the list of teams, and the game map.
 *
 * This should eventually become part of the game state object IMO, which should be a child of play_controller.
 *
 * I also intend to move the pathfinding module to be housed within this class -- this way, we can implement a
 * sound pathfinding data structure to speed up path computations for AI, without having to make "update event"
 * code at all points in the engine which modify the relevant data.
 *
 **/

class game_board {

	std::vector<team> teams_;

	gamemap map_;
	unit_map units_;

	//TODO: Remove these when we have refactored enough to make it possible.
	friend class play_controller;
	friend class replay_controller;
	friend class playsingle_controller;
	friend class playmp_controller;
	friend class events::mouse_handler;
	friend class events::menu_handler;

	friend class game_display;
	/** 
	 * Temporary unit move structs:
	 *
	 * Probably don't remove these friends, this is actually fairly useful. These structs are used by:
	 *  - AI
	 *  - Whiteboard
	 *  - I think certain wml actions
	 * For AI, the ai wants to move two units next to eachother so it can ask for attack calculations. This should not trigger
	 * pathfinding modifications, so the version that directly changes the unit map is probably preferable, although it should be
	 * refactored.
	 * For whiteboard and wml actions, we generally do want pathfinding to be updated, so use the game_board constructors which I 
	 * have added to these structs instead.
	 *
	 **/
	friend struct temporary_unit_placer;
	friend struct temporary_unit_mover;
	friend struct temporary_unit_remover;
	
	public:

	// Constructors and const accessors

	game_board(const config & game_config, const config & level) : teams_(), map_(game_config, level), units_() {}

	const std::vector<team> & teams() const { return teams_; }
	const gamemap & map() const { return map_; }
	const unit_map & units() const { return units_; }

	// Saving

	void write_config(config & cfg) const;

	// Manipulators from play_controller

	void new_turn(int pnum);
	void end_turn(int pnum);
	void set_all_units_user_end_turn();

	void all_survivors_to_recall();

	// Manipulator from playturn

	void side_drop_to (int side_num, team::CONTROLLER ctrl);
	void side_change_controller (int side_num, team::CONTROLLER ctrl, const std::string pname = "");

	// Manipulator from actionwml

	bool try_add_unit_to_recall_list(const map_location& loc, const unit& u);
	boost::optional<std::string> replace_map (const gamemap & r);
	void overlay_map (const gamemap & o, const config & cfg, map_location loc, bool border);

	bool change_terrain(const map_location &loc, const t_translation::t_terrain &t,
                    gamemap::tmerge_mode mode, bool replace_if_failed); //used only by lua

	// Global accessor from unit.hpp

	unit_map::iterator find_visible_unit(const map_location &loc, const team& current_team, bool see_all = false);
	unit_map::iterator find_visible_unit(const map_location & loc, size_t team, bool see_all = false) { return find_visible_unit(loc, teams_[team], see_all); }
	bool has_visible_unit (const map_location & loc, const team & team, bool see_all = false);
	bool has_visible_unit (const map_location & loc, size_t team, bool see_all = false) { return has_visible_unit(loc, teams_[team], see_all); }

	unit* get_visible_unit(const map_location &loc, const team &current_team, bool see_all = false); //TODO: can this not return a pointer?
};

/**
 * This object is used to temporary place a unit in the unit map, swapping out
 * any unit that is already there.  On destruction, it restores the unit map to
 * its original.
 */
struct temporary_unit_placer
{
	temporary_unit_placer(unit_map& m, const map_location& loc, unit& u);
	temporary_unit_placer(game_board& m, const map_location& loc, unit& u);
	virtual  ~temporary_unit_placer();

private:
	unit_map& m_;
	const map_location loc_;
	unit *temp_;
};

// Begin Temporary Unit Move Structs
// TODO: Fix up the implementations which use game_board

/**
 * This object is used to temporary remove a unit from the unit map.
 * On destruction, it restores the unit map to its original.
 * unit_map iterators to this unit must not be accessed while the unit is temporarily
 * removed, otherwise a collision will happen when trying to reinsert the unit.
 */
struct temporary_unit_remover
{
	temporary_unit_remover(unit_map& m, const map_location& loc);
	temporary_unit_remover(game_board& m, const map_location& loc);
	virtual  ~temporary_unit_remover();

private:
	unit_map& m_;
	const map_location loc_;
	unit *temp_;
};


/**
 * This object is used to temporary move a unit in the unit map, swapping out
 * any unit that is already there.  On destruction, it restores the unit map to
 * its original.
 */
struct temporary_unit_mover
{
	temporary_unit_mover(unit_map& m, const map_location& src,
	                     const map_location& dst, int new_moves);
	temporary_unit_mover(unit_map& m, const map_location& src,
	                     const map_location& dst);
	temporary_unit_mover(game_board& b, const map_location& src,
	                     const map_location& dst, int new_moves);
	temporary_unit_mover(game_board& b, const map_location& src,
	                     const map_location& dst);
	virtual  ~temporary_unit_mover();

private:
	unit_map& m_;
	const map_location src_;
	const map_location dst_;
	int old_moves_;
	unit *temp_;
};


#endif
