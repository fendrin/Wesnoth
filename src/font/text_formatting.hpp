/*
   Copyright (C) 2003 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TEXT_FORMATTING_HPP_INCLUDED
#define TEXT_FORMATTING_HPP_INCLUDED

#include "sdl/color.hpp"

#include <SDL.h>
#include <string>

/**
 * Collection of helper functions relating to Pango formatting.
 */

namespace font {

/**
 * Retuns a Pango formatting string using the provided color_t object.
 *
 * The string returned will be in format: '<span foreground=#color>'
 * Callers will need to manually append the closing</span>' tag.
 *
 * @param color        The color_t object from which to retrieve the color.
 */
std::string span_color(const color_t& color);

/**
 * Returns a hex color string from a color range.
 *
 * @param id           The id of the color range.
 */
std::string get_pango_color_from_id(const std::string& id);

/**
 * Returns the name of a color range, colored with its own color.
 *
 * @param id           The id of the color range.
 */
std::string get_color_string_pango(const std::string& id);

}

#endif
