/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_RESIZE_MAP_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_RESIZE_MAP_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

class ttoggle_button;

class teditor_resize_map : public tdialog
{
public:
	teditor_resize_map();
	
	void set_map_width(int value);
	int map_width() const;
	void set_map_height(int value);
	int map_height() const;
	void set_old_map_width(int value);
	void set_old_map_height(int value);
	bool copy_edge_terrain() const;
	
	enum EXPAND_DIRECTION {
		EXPAND_BOTTOM_RIGHT,
		EXPAND_BOTTOM,
		EXPAND_BOTTOM_LEFT,
		EXPAND_RIGHT,
		EXPAND_CENTER,
		EXPAND_LEFT,
		EXPAND_TOP_RIGHT,
		EXPAND_TOP,
		EXPAND_TOP_LEFT
	};
	EXPAND_DIRECTION expand_direction() { return expand_direction_; }
	void set_expand_direction(EXPAND_DIRECTION direction) { expand_direction_ = direction; }
	
	void update_expand_direction(twindow& window);

private:
	void set_direction_icon(int index, std::string icon);
	/**
	 * NOTE the map sizes are stored in a text variable since there is no
	 * integer edit widget yet.
	 */
	tfield_integer* map_width_;
	tfield_integer* map_height_;
	tfield_bool* copy_edge_terrain_;
	ttoggle_button* direction_buttons_[9];
	int old_width_;
	int old_height_;
	
	EXPAND_DIRECTION expand_direction_;

	/** Inherited from tdialog. */
	twindow build_window(CVideo& video);
	
	void pre_show(CVideo& video, twindow& window);
	
};

} // namespace gui2

#endif

