/*
   Copyright (C) 2012 - 2013 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR_TOOLKIT_H_INCLUDED
#define EDITOR_TOOLKIT_H_INCLUDED

//#include "../../display.hpp"
//#include "brush.hpp"
//#include "palette_layout.hpp"

#include "config.hpp"
#include <boost/scoped_ptr.hpp>
#include "editor/palette/palette_manager.hpp"
#include "hotkeys.hpp"
#include "editor/toolkit/brush_bar.hpp"
#include "editor/map/context_manager.hpp"

namespace editor {

/** A bar where the brush is drawn */
class editor_toolkit {

public:
	editor_toolkit(editor_display& gui, const CKey& key,
			const config& game_config, context_manager& c_manager);

	~editor_toolkit();

	void adjust_size();

	void update_mouse_action_highlights();

private:
	/** init the sidebar objects */
	void init_sidebar(const config& game_config);

	/** init the brushes */
	void init_brushes(const config& game_config);

	/** init the mouse actions (tools) */
	void init_mouse_actions(const config& game_config, context_manager& c_manager);

public:
	void set_mouseover_overlay();
	void clear_mouseover_overlay();

	/**
	 * Set the current mouse action based on a hotkey id
	 */
	void hotkey_set_mouse_action(hotkey::HOTKEY_COMMAND command);

	/**
	 * @return true if the mouse action identified by the hotkey is active
	 */
	bool is_mouse_action_set(hotkey::HOTKEY_COMMAND command) const;


	/** Get the current mouse action */
 	mouse_action* get_mouse_action() { return mouse_action_; };

	void redraw_toolbar();

	/** Brush related methods */

	/** Cycle to the next brush. */
	void cycle_brush();

	palette_manager* get_palette_manager() { return palette_manager_.get(); };

private:

	editor_display& gui_;

	const CKey& key_;

	/** TODO */
	boost::scoped_ptr<palette_manager> palette_manager_;

//Toolbar

	/** Toolbar-requires-redraw flag */
	bool toolbar_dirty_;

//Tools

	/** The current mouse action */
	mouse_action* mouse_action_;

	/** The mouse actions */
	typedef std::map<hotkey::HOTKEY_COMMAND, mouse_action*> mouse_action_map;
	mouse_action_map mouse_actions_;

	/** Usage tips for mouse actions */
	typedef std::map<hotkey::HOTKEY_COMMAND, std::string> mouse_action_string_map;
	mouse_action_string_map mouse_action_hints_;

//Brush members

	/** The current brush */
	brush* brush_;

	/** All available brushes */
	std::vector<brush> brushes_;

	/** The brush selector */
	boost::scoped_ptr<brush_bar> brush_bar_;

};

}

#endif
