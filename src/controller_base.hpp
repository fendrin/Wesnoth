/*
   Copyright (C) 2006 - 2014 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CONTROLLER_BASE_H_INCLUDED
#define CONTROLLER_BASE_H_INCLUDED

#include "global.hpp"

#include "hotkey/command_executor.hpp"
#include "key.hpp"

#include "joystick.hpp"

#include "map.hpp"

#include <boost/scoped_ptr.hpp>

class CVideo;
class plugins_context;

namespace events { class mouse_handler_base; }

namespace soundsource { class manager; }

class controller_base : public hotkey::command_executor, public events::sdl_handler
{
public:
	controller_base(const int ticks, const config& game_config, CVideo& video);
	virtual ~controller_base();

	void play_slice(bool is_delay_enabled = true);

	int get_ticks();

	plugins_context * get_plugins_context();

protected:
	/**
	 * Get a reference to a mouse handler member a derived class uses
	 */
	virtual events::mouse_handler_base& get_mouse_handler_base() = 0;
	/**
	 * Get a reference to a display member a derived class uses
	 */
	virtual display& get_display() = 0;


	/**
	 * Derived classes should override this to return false when arrow keys
	 * should not scroll the map, hotkeys not processed etc, for example
	 * when a textbox is active
	 * @returns true when arrow keys should scroll the map, false otherwise
	 */
	virtual bool have_keyboard_focus();


	/**
	 * Handle scrolling by keyboard, joystick and moving mouse near map edges
	 * @see is_keyboard_scroll_active
	 * @return true when there was any scrolling, false otherwise
	 */
	bool handle_scroll(CKey& key, int mousex, int mousey, int mouse_flags, double joystickx, double joysticky);

	/**
	 * Process mouse- and keypress-events from SDL.
	 * Not virtual but calls various virtual function to allow specialized
	 * behavior of derived classes.
	 */
	void handle_event(const SDL_Event& event);

	/**
	 * Process keydown (only when the general map display does not have focus).
	 */
	virtual void process_focus_keydown_event(const SDL_Event& event);

	/**
	 * Process keydown (always).
	 * Overridden in derived classes
	 */
	virtual void process_keydown_event(const SDL_Event& event);

	/**
	 * Process keyup (always).
	 * Overridden in derived classes
	 */
	virtual void process_keyup_event(const SDL_Event& event);

	virtual void show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu, display& disp);
	virtual void execute_action(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu);

	virtual bool in_context_menu(hotkey::HOTKEY_COMMAND command) const;

	static const config &get_theme(const config& game_config, std::string theme_name);
	const config& game_config_;
	const int ticks_;
	CKey key_;
	bool browse_;
	bool scrolling_;
	joystick_manager joystick_manager_;

	boost::scoped_ptr<plugins_context> plugins_context_;

	void set_soundsource_manager(soundsource::manager *);

	soundsource::manager * soundsources_;
};


#endif
