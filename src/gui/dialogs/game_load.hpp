/*
   Copyright (C) 2008 - 2016 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_LOAD_GAME_HPP_INCLUDED
#define GUI_DIALOGS_LOAD_GAME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "savegame.hpp"
#include "save_index.hpp"
#include "tstring.hpp"

namespace gui2
{

class tlistbox;
class ttext_;

class tgame_load : public tdialog
{
public:
	tgame_load(const config& cache_config, savegame::load_game_metadata& data);

	static bool execute(const config& cache_config, savegame::load_game_metadata& data, CVideo& video)
	{
		if(savegame::get_saves_list().empty()) {
			gui2::show_transient_message(video, _("No Saved Games"), _("There are no save files to load"));
			return false;
		}

		return tgame_load(cache_config, data).show(video);
	}

private:
	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	void filter_text_changed(ttext_* textbox, const std::string& text);
	void delete_button_callback(twindow& window);

	void display_savegame(twindow& window);
	void evaluate_summary_string(std::stringstream& str, const config& cfg_summary);

	std::string& filename_;

	tfield_bool* change_difficulty_;
	tfield_bool* show_replay_;
	tfield_bool* cancel_orders_;

	config& summary_;

	std::vector<savegame::save_info> games_;
	const config& cache_config_;

	std::vector<std::string> last_words_;
};
}

#endif
