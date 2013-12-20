/*
   Copyright (C) 2010 - 2013 by Fabian Müller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_EDIT_SIDE_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_EDIT_SIDE_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "config.hpp"
#include "team.hpp"

namespace gui2 {

class ttoggle_button;

class teditor_edit_side : public tdialog
{
public:

	teditor_edit_side(std::string& id, std::string& name, int& gold, int& income,
			bool& fog, bool& share_view, bool& shroud, bool& share_maps, team::CONTROLLER& controller);

	/** The execute function see @ref tdialog for more information. */
	static bool execute(std::string& id, std::string& name, int& gold, int& income,
			bool& fog, bool& share_view, bool& shroud, bool& share_maps, team::CONTROLLER& controller,
			CVideo& video)
	{
		return teditor_edit_side(id, name, gold, income, fog, share_view, shroud, share_maps, controller).show(video);
	}

private:

	void pre_show(CVideo& /*video*/, twindow& window);

	void register_controller_toggle(twindow& window, const std::string& toggle_id, team::CONTROLLER value);

	team::CONTROLLER& controller_;

	typedef std::pair<ttoggle_button*, team::CONTROLLER> controller_toggle;
	// Dialog display state variables.
	std::vector<controller_toggle> controller_tgroup_;

	void toggle_controller_callback(ttoggle_button* active);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

};

}

#endif /* ! GUI_DIALOGS_EDIT_LABEL_INCLUDED */
