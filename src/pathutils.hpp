/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef PATHUTILS_H_INCLUDED
#define PATHUTILS_H_INCLUDED

#include "map_location.hpp"

class gamemap;

class xy_pred : public std::unary_function<map_location const&, bool>
{
public:
	virtual bool operator()(map_location const&) = 0;
protected:
	virtual ~xy_pred() {}
};

/**
 * Function which, given a location, will place all locations in a ring of
 * distance r in res. res must be a std::vector of locations.
 */
void get_tile_ring(const map_location& a, const int r, std::vector<map_location>& res);

/**
 * Function which, given a location, will place all locations within radius 'r'
 * of that location in res. res must be a std::vector of locations.
 */
void get_tiles_in_radius(const map_location& a, const int r, std::vector<map_location>& res);

/**
 * Function which, given a location, will place all locations within radius 'radius'
 * of that location in res. res must be a std::set of locations.
 */
void get_tiles_radius(const map_location& a, size_t radius,
					  std::set<map_location>& res);

/**
 * Function which, given a set of locations, will scan outward from those
 * locations, looking for hexes (on the map, possibly @a with_border) that
 * match 'pred'. Tiles in chains of up to 'radius' tiles will be included in
 * the result (res). res must be a std::set of locations.
 */
void get_tiles_radius(const gamemap& map, const std::vector<map_location>& locs, size_t radius,
					  std::set<map_location>& res, bool with_border=false, xy_pred *pred=NULL);

#endif

