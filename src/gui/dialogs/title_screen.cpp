/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/title_screen.hpp"

#include "addon/manager_ui.hpp"
#include "formatter.hpp"
#include "game_config.hpp"
#include "game_config_manager.hpp"
#include "game_launcher.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/timer.hpp"
#include "gui/core/tips.hpp"
#include "gui/dialogs/core_selection.hpp"
#include "gui/dialogs/debug_clock.hpp"
#include "gui/dialogs/game_version.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/mp_method_selection.hpp"
#include "gui/dialogs/multiplayer/mp_host_game_prompt.hpp"
//#define DEBUG_TOOLTIP
#ifdef DEBUG_TOOLTIP
#include "gui/dialogs/tip.hpp"
#endif
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "help/help.hpp"
#include "hotkey/hotkey_command.hpp"
#include "video.hpp"

#include "utils/functional.hpp"

#include <algorithm>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_title_screen
 *
 * == Title screen ==
 *
 * This shows the title screen.
 *
 * @begin{table}{dialog_widgets}
 * tutorial & & button & m &
 *         The button to start the tutorial. $
 *
 * campaign & & button & m &
 *         The button to start a campaign. $
 *
 * multiplayer & & button & m &
 *         The button to start multiplayer mode. $
 *
 * load & & button & m &
 *         The button to load a saved game. $
 *
 * editor & & button & m &
 *         The button to start the editor. $
 *
 * addons & & button & m &
 *         The button to start managing the addons. $
 *
 * cores & & button & m &
 *         The button to start managing the cores. $
 *
 * language & & button & m &
 *         The button to select the game language. $
 *
 * credits & & button & m &
 *         The button to show Wesnoth's contributors. $
 *
 * quit & & button & m &
 *         The button to quit Wesnoth. $
 *
 * tips & & multi_page & m &
 *         A multi_page to hold all tips, when this widget is used the area of
 *         the tips doesn't need to be resized when the next or previous button
 *         is pressed. $
 *
 * -tip & & label & o &
 *         Shows the text of the current tip. $
 *
 * -source & & label & o &
 *         The source (the one who's quoted or the book referenced) of the
 *         current tip. $
 *
 * next_tip & & button & m &
 *         The button show the next tip of the day. $
 *
 * previous_tip & & button & m &
 *         The button show the previous tip of the day. $
 *
 * logo & & image & o &
 *         The Wesnoth logo. $
 *
 * revision_number & & control & o &
 *         A widget to show the version number when the version number is
 *         known. $
 *
 * @end{table}
 */

REGISTER_DIALOG(title_screen)

bool show_debug_clock_button = false;

ttitle_screen::ttitle_screen(game_launcher& game)
	: game_(game)
	, redraw_background_(true)
	, debug_clock_(nullptr)
{
	set_restore(false);

	// Need to set this in the constructor, pre_show() / post_build() is too late
	set_allow_plugin_skip(false);
}

ttitle_screen::~ttitle_screen()
{
	delete debug_clock_;
}

using btn_callback = std::function<void(twindow&)>;

static void register_button(twindow& window, const std::string& id, hotkey::HOTKEY_COMMAND hk, btn_callback callback)
{
	if(hk != hotkey::HOTKEY_NULL) {
		window.register_hotkey(hk, [callback](event::tdispatcher& win, hotkey::HOTKEY_COMMAND) {
			callback(dynamic_cast<twindow&>(win));
			return true;
		});
	}

	event::connect_signal_mouse_left_click(find_widget<tbutton>(&window, id, false),
		[callback](event::tdispatcher& win, event::tevent, bool&, bool&) { callback(dynamic_cast<twindow&>(win)); });
}

static bool fullscreen(CVideo& video)
{
	video.set_fullscreen(!preferences::fullscreen());
	return true;
}

static bool launch_lua_console(twindow& window)
{
	gui2::tlua_interpreter::display(window.video(), gui2::tlua_interpreter::APP);
	return true;
}

#ifdef DEBUG_TOOLTIP
/*
 * This function is used to test the tooltip placement algorithms as
 * described in the »Tooltip placement« section in the GUI2 design
 * document.
 *
 * Use a 1024 x 768 screen size, set the maximum loop iteration to:
 * - 0   to test with a normal tooltip placement.
 * - 30  to test with a larger normal tooltip placement.
 * - 60  to test with a huge tooltip placement.
 * - 150 to test with a borderline to insanely huge tooltip placement.
 * - 180 to test with an insanely huge tooltip placement.
 */
static void debug_tooltip(twindow& window, bool& handled, const tpoint& coordinate)
{
	std::string message = "Hello world.";

	for(int i = 0; i < 0; ++i) {
		message += " More greetings.";
	}

	gui2::tip::remove();
	gui2::tip::show(window.video(), "tooltip", message, coordinate);

	handled = true;
}
#endif

void ttitle_screen::pre_show(twindow& window)
{
	window.set_click_dismiss(false);
	window.set_enter_disabled(true);
	window.set_escape_disabled(true);

	// Each time the dialog shows, we set this to false
	redraw_background_ = false;

#ifdef DEBUG_TOOLTIP
	window.connect_signal<event::SDL_MOUSE_MOTION>(
			std::bind(debug_tooltip, std::ref(window), _3, _5),
			event::tdispatcher::front_child);
#endif

	window.connect_signal<event::SDL_VIDEO_RESIZE>(std::bind(&ttitle_screen::on_resize, this, std::ref(window)));

	//
	// General hotkeys
	//
	window.register_hotkey(hotkey::TITLE_SCREEN__RELOAD_WML, [this](event::tdispatcher& win, hotkey::HOTKEY_COMMAND) {
		dynamic_cast<twindow&>(win).set_retval(RELOAD_GAME_DATA);
		return true;
	});

	window.register_hotkey(hotkey::HOTKEY_FULLSCREEN, std::bind(fullscreen, std::ref(window.video())));
	window.register_hotkey(hotkey::LUA_CONSOLE, std::bind(&launch_lua_console, std::ref(window)));

	//
	// Background and logo images
	//
	if(game_config::images::game_title.empty()) {
		ERR_CF << "No title image defined" << std::endl;
	} else {
		window.canvas()[0].set_variable("title_image", variant(game_config::images::game_title));
	}

	if(game_config::images::game_title_background.empty()) {
		ERR_CF << "No title background image defined" << std::endl;
	} else {
		window.canvas()[0].set_variable("background_image", variant(game_config::images::game_title_background));
	}

	find_widget<timage>(&window, "logo-bg", false).set_image(game_config::images::game_logo_background);
	find_widget<timage>(&window, "logo", false).set_image(game_config::images::game_logo);

	//
	// Version string
	//
	const std::string version_string = formatter() << ("Version") << " " << game_config::revision;

	if(tlabel* version_label = find_widget<tlabel>(&window, "revision_number", false, false)) {
		version_label->set_label(version_string);
	}

	window.canvas()[0].set_variable("revision_number", variant(version_string));

	//
	// Tip-of-the-day browser
	//
	tmulti_page& tip_pages = find_widget<tmulti_page>(&window, "tips", false);

	std::vector<ttip> tips(settings::get_tips());
	if(tips.empty()) {
		WRN_CF << "There are not tips of day available." << std::endl;
	}

	for(const auto& tip : tips)	{
		string_map widget;
		std::map<std::string, string_map> page;

		widget["use_markup"] = "true";

		widget["label"] = tip.text();
		page.emplace("tip", widget);

		widget["label"] = tip.source();
		page.emplace("source", widget);

		tip_pages.add_page(page);
	}

	update_tip(window, true);

	register_button(window, "next_tip", hotkey::TITLE_SCREEN__NEXT_TIP,
		std::bind(&ttitle_screen::update_tip, this, std::ref(window), true));
	register_button(window, "previous_tip", hotkey::TITLE_SCREEN__PREVIOUS_TIP,
		std::bind(&ttitle_screen::update_tip, this, std::ref(window), false));

	//
	// Help
	//
	register_button(window, "help", hotkey::HOTKEY_HELP, [this](twindow&) {
		help::help_manager help_manager(&game_config_manager::get()->game_config());
		help::show_help(game_.video());
	});

	//
	// About
	//
	register_button(window, "about", hotkey::HOTKEY_NULL, std::bind(&tgame_version::display, std::ref(window.video())));

	//
	// Tutorial
	//
	register_button(window, "tutorial", hotkey::TITLE_SCREEN__TUTORIAL, [this](twindow& window) {
		game_.set_tutorial();
		window.set_retval(LAUNCH_GAME);
	});

	//
	// Campaign
	//
	register_button(window, "campaign", hotkey::TITLE_SCREEN__CAMPAIGN, [this](twindow& window) {
		if(game_.new_campaign()) {
			window.set_retval(LAUNCH_GAME);
		}
	});

	//
	// Multiplayer
	//
	register_button(window, "multiplayer", hotkey::TITLE_SCREEN__MULTIPLAYER, [this](twindow& window) {
		while(true) {
			gui2::tmp_method_selection dlg;
			dlg.show(game_.video());

			if(dlg.get_retval() != gui2::twindow::OK) {
				return;
			}

			const int res = dlg.get_choice();

			if(res == 2 && preferences::mp_server_warning_disabled() < 2) {
				if(!gui2::tmp_host_game_prompt::execute(game_.video())) {
					continue;
				}
			}

			switch(res) {
				case 0:
					game_.select_mp_server(preferences::server_list().front().address);
					window.set_retval(MP_CONNECT);
					break;
				case 1:
					game_.select_mp_server("");
					window.set_retval(MP_CONNECT);
					break;
				case 2:
					game_.select_mp_server("localhost");
					window.set_retval(MP_HOST);
					break;
				case 3:
					window.set_retval(MP_LOCAL);
					break;
			}

			return;
		}
	});

	//
	// Load game
	//
	register_button(window, "load", hotkey::HOTKEY_LOAD_GAME, [this](twindow& window) {
		if(game_.load_game()) {
			window.set_retval(LAUNCH_GAME);
		} else {
			game_.clear_loaded_game();
		}
	});

	//
	// Addons
	//
	register_button(window, "addons", hotkey::TITLE_SCREEN__ADDONS, [this](twindow&) {
		// NOTE: we need the help_manager to get access to the Add-ons section in the game help!
		help::help_manager help_manager(&game_config_manager::get()->game_config());

		if(manage_addons(game_.video())) {
			game_config_manager::get()->reload_changed_game_config();
		}
	});

	//
	// Editor
	//
	register_button(window, "editor", hotkey::TITLE_SCREEN__EDITOR, [&](twindow& window) { window.set_retval(MAP_EDITOR); });

	//
	// Cores
	//
	register_button(window, "cores", hotkey::TITLE_SCREEN__CORES, [this](twindow&) {
		int current = 0;
		std::vector<config> cores;
		for(const config& core : game_config_manager::get()->game_config().child_range("core")) {
			cores.push_back(core);

			if(core["id"] == preferences::core_id()) {
				current = cores.size() - 1;
			}
		}

		gui2::tcore_selection core_dlg(cores, current);
		if(core_dlg.show(game_.video())) {
			const std::string& core_id = cores[core_dlg.get_choice()]["id"];

			preferences::set_core_id(core_id);
			game_config_manager::get()->reload_changed_game_config();
		}
	});

	if(game_config_manager::get()->game_config().child_range("core").size() <= 1) {
		find_widget<tbutton>(&window, "cores", false).set_visible(twindow::tvisible::invisible);
	}

	//
	// Language
	//
	register_button(window, "language", hotkey::HOTKEY_LANGUAGE, [this](twindow& window) {
		try {
			if(game_.change_language()) {
				t_string::reset_translations();
				image::flush_cache();
				on_resize(window);
			}
		} catch(std::runtime_error& e) {
			gui2::show_error_message(game_.video(), e.what());
		}
	});

	//
	// Preferences
	//
	register_button(window, "preferences", hotkey::HOTKEY_PREFERENCES, [this](twindow&) { game_.show_preferences(); });

	//
	// Credits
	//
	register_button(window, "credits", hotkey::TITLE_SCREEN__CREDITS, [&](twindow& window) { window.set_retval(SHOW_ABOUT); });

	//
	// Quit
	//
	register_button(window, "quit", hotkey::HOTKEY_QUIT_TO_DESKTOP, [&](twindow& window) { window.set_retval(QUIT_GAME); });

	//
	// Debug clock
	//
	register_button(window, "clock", hotkey::HOTKEY_NULL,
		std::bind(&ttitle_screen::show_debug_clock_window, this, std::ref(window.video())));

	find_widget<tbutton>(&window, "clock", false).set_visible(show_debug_clock_button
		? twidget::tvisible::visible
		: twidget::tvisible::invisible);
}

void ttitle_screen::on_resize(twindow& window)
{
	redraw_background_ = true;
	window.close();
}

void ttitle_screen::update_tip(twindow& window, const bool previous)
{
	tmulti_page& tips = find_widget<tmulti_page>(&window, "tips", false);
	if(tips.get_page_count() == 0) {
		return;
	}

	int page = tips.get_selected_page();
	if(previous) {
		if(page <= 0) {
			page = tips.get_page_count();
		}
		--page;
	} else {
		++page;
		if(static_cast<unsigned>(page) >= tips.get_page_count()) {
			page = 0;
		}
	}

	tips.select_page(page);

	/**
	 * @todo Look for a proper fix.
	 *
	 * This dirtying is required to avoid the blurring to be rendered wrong.
	 * Not entirely sure why, but since we plan to move to SDL2 that change
	 * will probably fix this issue automatically.
	 */
	window.set_is_dirty(true);
}

void ttitle_screen::show_debug_clock_window(CVideo& video)
{
	assert(show_debug_clock_button);

	if(debug_clock_) {
		delete debug_clock_;
		debug_clock_ = nullptr;
	} else {
		debug_clock_ = new tdebug_clock();
		debug_clock_->show(video, true);
	}
}

} // namespace gui2
