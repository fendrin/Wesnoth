/*
   Copyright (C) 2012 - 2016 by Fabian Mueller <fabianmueller5@gmx.de>
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

/**
 * @file
 * Manages the hotkey bindings.
 */

#include "preferences_display.hpp"
#include "hotkey/hotkey_item.hpp"

#include "construct_dialog.hpp"
#include "display.hpp"
#include "filesystem.hpp"
#include "formatter.hpp"
#include "formula_string_utils.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "wml_separators.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace preferences {

class hotkey_preferences_parent_dialog;

class hotkey_preferences_dialog: public gui::preview_pane {

private	:
	/** nice little magic number, the size of the truncated hotkey command */
	static const int truncate_at = 25;

public:
	hotkey_preferences_dialog(CVideo& v);

	/**
	 * Populates, sorts and redraws the hotkey menu specified by tab_.
	 * @param keep_viewport feeds keep_viewport param of menu::set_menu_items()
	 */
	void set_hotkey_menu(bool keep_viewport);

private:

	/// overrides events::sdl_handler::process_event
	void process_event();

	/// overrides gui::widget::update_location
	void update_location(SDL_Rect const &rect);

	/// overrides gui::preview_pane::handler_members
	virtual sdl_handler_vector handler_members();

	/// implements gui::preview_pane::set_selection
	void set_selection(int index);

	/// implements gui::preview_pane::left_side
	bool left_side() const {
		return false;
	}

	/**
	 * Sub-dialog, recognizing the hotkey sequence
	 * @param command add the new binding to this item
	 */
	void show_binding_dialog(const std::string& command);

	/**
	 * Buttons to trigger the tools involved in hotkey assignment.
	 * The buttons are shared by all scope tabs.
	 */
	gui::button add_button_, clear_button_;
	static const char* add_button_text;
	static const char* clear_button_text;

	/** The dialog features a tab for each hotkey scope (except SCOPE_COUNTER) */
	hotkey::scope tab_;

	/**
	 * These are to map the menu selection to the corresponding command
	 * example:
	 *
	 *	std::string selected_general_command =
	 *			general_commands_[general_hotkey_.get_selected()];
	 */
	std::vector<std::string> general_commands_;
	std::vector<std::string> game_commands_;
	std::vector<std::string> editor_commands_;
	std::vector<std::string> title_screen_commands_;

	/** The header of all the scope menus */
	const std::string heading_;

	/**
	 * Every scope gets its own menu and sorter to allow keeping the viewport
	 * while switching through the tabs.
	 */
	int selected_command_;
	gui::menu::basic_sorter general_sorter_, game_sorter_, editor_sorter_, title_screen_sorter_;
	gui::menu general_hotkeys_, game_hotkeys_, editor_hotkeys_, title_screen_hotkeys_;

	/** The display, for usage by child dialogs */
	CVideo &video_;

public:
	/**
	 * @todo Okay, we have here a public member in a class.
	 * Like in game_preferences_dialog.
	 */
	util::scoped_ptr<hotkey_preferences_parent_dialog> parent;
};

const char* hotkey_preferences_dialog::add_button_text =
		N_("Add a new binding to \"$hotkey_description|\"");
const char* hotkey_preferences_dialog::clear_button_text =
		N_("Clears the current bindings for \"$hotkey_description|\"");

class hotkey_resetter : public gui::dialog_button_action
{
public:
	hotkey_resetter(CVideo& video, hotkey_preferences_dialog& dialog) :
		video_(video),
		dialog_(dialog)
	{}

	// This method is called when the button is pressed.
	RESULT button_pressed(int /*selection*/)
	{
		clear_hotkeys();
		gui2::show_transient_message(video_, _("Hotkeys Reset"),
				_("All hotkeys have been reset to their default values."));
		dialog_.set_hotkey_menu(true);
		return gui::CONTINUE_DIALOG;
	}

private:
	CVideo& video_;
	hotkey_preferences_dialog& dialog_;
};

class hotkey_preferences_parent_dialog: public gui::dialog {

public:

	hotkey_preferences_parent_dialog(CVideo& video,
			hotkey_preferences_dialog& hotkey_preferences_dialog) :
				dialog(video, _("Hotkey Settings"), "", gui::OK_CANCEL),
				clear_buttons_(false),
				hotkey_cfg_(),
				resetter_(video, hotkey_preferences_dialog) {
		gui::dialog_button* reset_button = new gui::dialog_button(video,
				_("Defaults"), gui::button::TYPE_PRESS,
				gui::CONTINUE_DIALOG, &resetter_);
		reset_button->set_help_string(
				_("Reset all bindings to the default values") );
		add_button(reset_button, dialog::BUTTON_HELP);

		// keep the old config in case the user cancels the dialog
		hotkey::save_hotkeys(hotkey_cfg_);
	}

	~hotkey_preferences_parent_dialog() {
		// save or restore depending on the exit result
		if (result() >= 0) {
			save_hotkeys();
		} else {
			hotkey::load_hotkeys(hotkey_cfg_, false);
		}
	}

	void action(gui::dialog_process_info &info) {
		if (clear_buttons_) {
			info.clear_buttons();
			clear_buttons_ = false;
		}
	}

	void clear_buttons() {
		clear_buttons_ = true;
	}

private:
	bool clear_buttons_;
	config hotkey_cfg_;
	hotkey_resetter resetter_;
};


void show_hotkeys_preferences_dialog(CVideo& video) {

	std::vector<std::string> items;
	const std::string pre = IMAGE_PREFIX + std::string("icons/icon-");
	char const sep = COLUMN_SEPARATOR;

	// tab names and icons
	items.push_back(pre + "titlescreen.png" + sep
			+ translation::sgettext("Prefs section^Title Screen"));
	items.push_back(pre + "game.png"    + sep
			+ translation::sgettext("Prefs section^Game"));
	items.push_back(pre + "editor.png"  + sep
			+ translation::sgettext("Prefs section^Editor"));
	items.push_back(pre + "general.png" + sep
			+ translation::sgettext("Prefs section^General"));

	// determine the current scope, but skip general == 0
	int scope;
	for (scope = 1; scope != hotkey::SCOPE_COUNT; scope++) {
		if (hotkey::is_scope_active(static_cast<hotkey::scope>(scope))) {
			break; }
	}

	// The restorer will change the scope back to where we came from
	// when it runs out of the function's scope
	hotkey::scope_changer scope_restorer;
	hotkey_preferences_dialog dialog(video);
	dialog.parent.assign(new hotkey_preferences_parent_dialog(video, dialog));
	dialog.parent->set_menu(items);
	dialog.parent->add_pane(&dialog);
	// select the tab corresponding to the current scope.
	dialog.parent->get_menu().move_selection(scope - 1);
	dialog.parent->show();
	return;
}


/* hotkey_preferences_dialog members ************************/

hotkey_preferences_dialog::hotkey_preferences_dialog(CVideo& video) :
		gui::preview_pane(video),
		add_button_(video, _("Add Hotkey")),
		clear_button_(video,	_("Clear Hotkey")),
		tab_(hotkey::SCOPE_COUNT), //SCOPE_COUNT means "hotkey with more than one scope" in this case
		general_commands_(),
		game_commands_(),
		editor_commands_(),
		title_screen_commands_(),
		heading_( (formatter() << HEADING_PREFIX << COLUMN_SEPARATOR << _("Action")
						<< COLUMN_SEPARATOR << _("Binding")).str() ),
		selected_command_(0),
		general_sorter_(),
		game_sorter_(),
		editor_sorter_(),
		title_screen_sorter_(),
		// Note: If you don't instantiate the menus with heading_,
		// the header can't be enabled later, seems to be a bug in gui::menu
		general_hotkeys_(video, boost::assign::list_of(heading_),
				false, -1, -1, &general_sorter_, &gui::menu::bluebg_style),
		game_hotkeys_(video, boost::assign::list_of(heading_),
				false, -1, -1, &game_sorter_, &gui::menu::bluebg_style),
		editor_hotkeys_(video, boost::assign::list_of(heading_),
				false, -1, -1, &editor_sorter_, &gui::menu::bluebg_style),
		title_screen_hotkeys_(video, boost::assign::list_of(heading_),
				false, -1, -1, &title_screen_sorter_, &gui::menu::bluebg_style),
		video_(video),
		parent(NULL)
{

	set_measurements(preferences::width, preferences::height);

	// Populate the command vectors, this needs to happen only once.
	const boost::ptr_vector<hotkey::hotkey_command>& list = hotkey::get_hotkey_commands();

	//for (size_t i = 0; list[i].id != hotkey::HOTKEY_NULL; ++i) {
	BOOST_FOREACH(const hotkey::hotkey_command& command, list)
	{
		if (command.hidden)
		{
			continue;
		}
		// We move hotkeys in all categories thet they belog to, except for hotkeys that
		// belong to all 3 scoped that we put in a seperate HOTKEY_GENERAL category.
		if(command.scope.count() == 1) //Not all
		{
			if(command.scope[hotkey::SCOPE_GAME])
			{
				game_commands_.push_back(command.command);
			}
			if(command.scope[hotkey::SCOPE_EDITOR])
			{
				editor_commands_.push_back(command.command);
			}
			if(command.scope[hotkey::SCOPE_MAIN_MENU])
			{
				title_screen_commands_.push_back(command.command);
			}
		}
		else
		{
			general_commands_.push_back(command.command);
		}
	}

	/// @todo move to the caller?
	video_.clear_all_help_strings();

	// Initialize sorters.
	general_sorter_.set_alpha_sort(1).set_alpha_sort(2);
	game_sorter_.set_alpha_sort(1).set_alpha_sort(2);
	editor_sorter_.set_alpha_sort(1).set_alpha_sort(2);
	title_screen_sorter_.set_alpha_sort(1).set_alpha_sort(2);

	// Populate every menu_
	for (int scope = 0; scope != hotkey::SCOPE_COUNT; scope++) {
		tab_ = hotkey::scope(scope);
		set_hotkey_menu(false);
	}

	general_hotkeys_.sort_by(1);
	game_hotkeys_.sort_by(1);
	editor_hotkeys_.sort_by(1);
	title_screen_hotkeys_.sort_by(1);
}

void hotkey_preferences_dialog::set_hotkey_menu(bool keep_viewport) {

	/*
	 * First hide all elements of all tabs,
	 * we might have redrawing glitches otherwise.
	 */
	add_button_.hide(true);
	clear_button_.hide(true);
	game_hotkeys_.hide(true);
	editor_hotkeys_.hide(true);
	general_hotkeys_.hide(true);
	title_screen_hotkeys_.hide(true);

	// Helpers to keep the switch statement small.
	gui::menu* active_hotkeys = NULL;
	std::vector<std::string>* commands = NULL;

	// Determine the menu corresponding to the selected tab.
	switch (tab_) {
	case hotkey::SCOPE_MAIN_MENU:
		active_hotkeys = &title_screen_hotkeys_;
		commands = &title_screen_commands_;
		break;
	case hotkey::SCOPE_GAME:
		active_hotkeys = &game_hotkeys_;
		commands = &game_commands_;
		break;
	case hotkey::SCOPE_EDITOR:
		active_hotkeys = &editor_hotkeys_;
		commands = &editor_commands_;
		break;
	case hotkey::SCOPE_COUNT:
		active_hotkeys = &general_hotkeys_;
		commands = &general_commands_;
		break;
	}

	// Fill the menu rows
	std::vector<std::string> menu_items;
	BOOST_FOREACH(const std::string& command, *commands) {

		const std::string& description = hotkey::get_description(command);
		const std::string& tooltip     = hotkey::get_tooltip(command);
		std::string truncated_description = description;
		if (truncated_description.size() >= (truncate_at + 2) ) {
			utils::ellipsis_truncate(truncated_description, truncate_at);
		}
		const std::string& name = hotkey::get_names(command);

		std::string image_path = "misc/empty.png~CROP(0,0,15,15)";
		if (filesystem::file_exists(game_config::path + "/images/icons/action/" + command + "_25.png"))
			image_path = "icons/action/" + command + "_25.png~CROP(3,3,18,18)";

		menu_items.push_back(
				(formatter() << IMAGE_PREFIX << image_path << COLUMN_SEPARATOR
						<< truncated_description
						<< HELP_STRING_SEPARATOR << description << (tooltip.empty() ? "" : " - ") << tooltip
						<< COLUMN_SEPARATOR << font::NULL_MARKUP
						<< name << HELP_STRING_SEPARATOR << name).str() );
	}

	/* No effect if vector is used in set_items instead of menu's constructor. */
    // menu_items.push_back(heading_);

	// Re-populate the items.
	// Workaround for set_items not keeping the selected item.
	selected_command_ = active_hotkeys->selection();
	active_hotkeys->set_items(menu_items, true, keep_viewport);
	// This shall happen only once at the start of the dialog.
	if (!keep_viewport) {
		active_hotkeys->sort_by(0);
		active_hotkeys->reset_selection();
	} else {
		active_hotkeys->move_selection_keeping_viewport(selected_command_);
	    // !hide and thus redraw only the current tab_'s items
		active_hotkeys->hide(false);
		add_button_.hide(false);
		clear_button_.hide(false);
	}
	selected_command_ = active_hotkeys->selection();
	utils::string_map symbols;
	symbols["hotkey_description"] =
			hotkey::get_description((*commands)[selected_command_]);

	const std::string clear_text =
			vgettext(hotkey_preferences_dialog::clear_button_text, symbols);
	clear_button_.set_help_string(clear_text);
	const std::string   add_text =
			vgettext(hotkey_preferences_dialog::add_button_text, symbols);
	add_button_.set_help_string(add_text);

}

sdl_handler_vector hotkey_preferences_dialog::handler_members() {
	sdl_handler_vector h;
	h.push_back(&add_button_);
	h.push_back(&clear_button_);
	h.push_back(&general_hotkeys_);
	h.push_back(&game_hotkeys_);
	h.push_back(&editor_hotkeys_);
	h.push_back(&title_screen_hotkeys_);
	return h;
}

void hotkey_preferences_dialog::set_selection(int index) {

	tab_ = hotkey::scope(index);

	set_dirty();
	bg_restore();

	hotkey::deactivate_all_scopes();
	switch (tab_) {
	case hotkey::SCOPE_MAIN_MENU:
		hotkey::set_scope_active(hotkey::SCOPE_MAIN_MENU);
		break;
	case hotkey::SCOPE_COUNT:
		hotkey::set_scope_active(hotkey::SCOPE_GAME);
		hotkey::set_scope_active(hotkey::SCOPE_EDITOR);
		hotkey::set_scope_active(hotkey::SCOPE_MAIN_MENU);
		break;
	case hotkey::SCOPE_GAME:
		hotkey::set_scope_active(hotkey::SCOPE_GAME);
		break;
	case hotkey::SCOPE_EDITOR:
		hotkey::set_scope_active(hotkey::SCOPE_EDITOR);
		break;
	}
	set_hotkey_menu(true);
}

void hotkey_preferences_dialog::process_event() {

	std::string id;
	gui::menu* active_menu_ = NULL;
	switch (tab_) {
	case hotkey::SCOPE_MAIN_MENU:
		id = title_screen_commands_[title_screen_hotkeys_.selection()];
		active_menu_ = &title_screen_hotkeys_;
		break;
	case hotkey::SCOPE_GAME:
		id = game_commands_[game_hotkeys_.selection()];
		active_menu_ = &game_hotkeys_;
		break;
	case hotkey::SCOPE_EDITOR:
		id = editor_commands_[editor_hotkeys_.selection()];
		active_menu_ = &editor_hotkeys_;
		break;
	case hotkey::SCOPE_COUNT:
		id = general_commands_[general_hotkeys_.selection()];
		active_menu_ = &general_hotkeys_;
		break;
	}

	if ( selected_command_ != active_menu_->selection()) {
		selected_command_ = active_menu_->selection();

		utils::string_map symbols;
		symbols["hotkey_description"] = hotkey::get_description(id);

		const std::string clear_text = vgettext(clear_button_text, symbols);
		clear_button_.set_help_string(clear_text);

		const std::string   add_text = vgettext(add_button_text, symbols);
		add_button_.set_help_string(add_text);
	}

	if (add_button_.pressed() || active_menu_->double_clicked()) {
		show_binding_dialog(id);
	}

	if (clear_button_.pressed()) {
		// clear hotkey
		hotkey::clear_hotkeys(id);
		set_hotkey_menu(true);
	}

}

void hotkey_preferences_dialog::update_location(SDL_Rect const &rect) {

	bg_register(rect);

	/** some magic numbers :-P
	@todo they match the ones in game_preferences_dialog. */
	const int top_border = 10;
	const int right_border = font::relative_size(10);
	const int h = height() - 75; // well, this one not.
	const int w = rect.w - right_border;

	int xpos = rect.x;
	int ypos = rect.y + top_border;

	title_screen_hotkeys_.set_location(xpos, ypos);
	title_screen_hotkeys_.set_max_height(h);
	title_screen_hotkeys_.set_height(h);
	title_screen_hotkeys_.set_max_width(w);
	title_screen_hotkeys_.set_width(w);

	general_hotkeys_.set_location(xpos, ypos);
	general_hotkeys_.set_max_height(h);
	general_hotkeys_.set_height(h);
	general_hotkeys_.set_max_width(w);
	general_hotkeys_.set_width(w);

	game_hotkeys_.set_location(xpos, ypos);
	game_hotkeys_.set_max_height(h);
	game_hotkeys_.set_height(h);
	game_hotkeys_.set_max_width(w);
	game_hotkeys_.set_width(w);

	editor_hotkeys_.set_location(xpos, ypos);
	editor_hotkeys_.set_max_height(h);
	editor_hotkeys_.set_height(h);
	editor_hotkeys_.set_max_width(w);
	editor_hotkeys_.set_width(w);

	ypos += h + font::relative_size(14);

	add_button_.set_location(xpos, ypos);
	xpos += add_button_.width() + font::relative_size(14);
	clear_button_.set_location(xpos, ypos);
}

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4701)
#endif

void hotkey_preferences_dialog::show_binding_dialog(const std::string& id)
{
	hotkey::hotkey_ptr newhk = hotkey::show_binding_dialog(video_, id);
	hotkey::hotkey_ptr oldhk;
	// only if not cancelled.
	if (newhk.get() != NULL) {
		BOOST_FOREACH(const hotkey::hotkey_ptr& hk, hotkey::get_hotkeys()) {
			if(newhk->bindings_equal(hk)) {
				oldhk == hk;
			}
		}
		hotkey::scope_changer scope_restorer;
		hotkey::set_active_scopes(hotkey::get_hotkey_command(id).scope);

#if 0
			//TODO
//			if ( (hotkey::get_id(newhk.get_command()) == hotkey::HOTKEY_SCREENSHOT
//					|| hotkey::get_id(newhk.get_command()) == hotkey::HOTKEY_MAP_SCREENSHOT)
//					&& (mod & any_mod) == 0 ) {
//				gui2::show_transient_message(video, _("Warning"),
/*						_("Screenshot hotkeys should be combined with the \
Control, Alt or Meta modifiers to avoid problems.")); */
//			}
			break;
		}
#endif

		if (oldhk && oldhk->active()) {
			if (oldhk->get_command() != id) {

				utils::string_map symbols;
				symbols["hotkey_sequence"]   = oldhk->get_name();
				symbols["old_hotkey_action"] = hotkey::get_description(oldhk->get_command());
				symbols["new_hotkey_action"] = hotkey::get_description(newhk->get_command());

				std::string text =
						vgettext("\"$hotkey_sequence|\" is in use by \
\"$old_hotkey_action|\". Do you wish to reassign it to \"$new_hotkey_action|\"?"
								, symbols);

				const int res = gui2::show_message(video_,
						_("Reassign Hotkey"), text,
						gui2::tmessage::yes_no_buttons);
				if (res == gui2::twindow::OK) {
					hotkey::add_hotkey(newhk);
					set_hotkey_menu(true);
				}
			}
		} else {
			hotkey::add_hotkey(newhk);
			set_hotkey_menu(true);
		}
	}

}
#ifdef _MSC_VER
#pragma warning (pop)
#endif

} // end namespace preferences
namespace hotkey {

hotkey::hotkey_ptr show_binding_dialog(CVideo& video, const std::string& id)
{

	// Lets bind a hotkey...

	const std::string text = _("Press desired hotkey (Esc cancels)");

	SDL_Rect clip_rect = sdl::create_rect(0, 0, video.getx(), video.gety());
	SDL_Rect text_size = font::draw_text(NULL, clip_rect, font::SIZE_LARGE,
			font::NORMAL_COLOR, text, 0, 0);

	const int centerx = video.getx() / 2;
	const int centery = video.gety() / 2;

	SDL_Rect dlgr = sdl::create_rect(centerx - text_size.w / 2 - 30,
			centery - text_size.h / 2 - 16, text_size.w + 60,
			text_size.h + 32);

	surface_restorer restorer(&video, dlgr);
	gui::dialog_frame mini_frame(video);
	mini_frame.layout(centerx - text_size.w / 2 - 20,
			centery - text_size.h / 2 - 6, text_size.w + 40,
			text_size.h + 12);
	mini_frame.draw_background();
	mini_frame.draw_border();
	font::draw_text(&video, clip_rect, font::SIZE_LARGE,
			font::NORMAL_COLOR, text,
			centerx - text_size.w / 2, centery - text_size.h / 2);
	video.flip();
	SDL_Event event;
	event.type = 0;
	int keycode  = -1, mod    = -1;
	const int any_mod = KMOD_CTRL | KMOD_META | KMOD_ALT;

	while ( event.type != SDL_KEYDOWN && event.type != SDL_JOYBUTTONDOWN
			&& event.type  != SDL_JOYHATMOTION
			&& (event.type != SDL_MOUSEBUTTONDOWN )
	)
		SDL_PollEvent(&event);
#if SDL_VERSION_ATLEAST(2,0,0)
	events::peek_for_resize();
#endif

	do {
		switch (event.type) {

		case SDL_KEYDOWN:
			keycode = event.key.keysym.sym;
			mod = event.key.keysym.mod;
			break;
		}


		SDL_PollEvent(&event);
#if SDL_VERSION_ATLEAST(2,0,0)
	events::peek_for_resize();
#endif

	} while (event.type  != SDL_KEYUP && event.type != SDL_JOYBUTTONUP
			&& event.type != SDL_JOYHATMOTION
			&& event.type != SDL_MOUSEBUTTONUP);

	restorer.restore();
	return keycode == SDLK_ESCAPE ? hotkey::hotkey_ptr() : hotkey::create_hotkey(id, event);
}

} //end namespace hotkey
