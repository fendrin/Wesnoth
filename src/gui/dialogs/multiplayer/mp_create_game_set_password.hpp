/*
   Copyright (C) 2010 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_CREATE_GAME_SET_PASSWORD_HPP_INCLUDED
#define GUI_DIALOGS_MP_CREATE_GAME_SET_PASSWORD_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2
{
namespace dialogs
{

class mp_create_game_set_password : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param [in, out] password  The parameter's usage is:
	 *                            - Input: The initial value for the password.
	 *                            - Output: The password selected by the user
	 *                              if the dialog returns @ref window::OK
	 *                              undefined otherwise.
	 */
	explicit mp_create_game_set_password(std::string& password);

	/** The excute function see @ref modal_dialog for more information. */
	static bool execute(std::string& password, CVideo& video)
	{
		return mp_create_game_set_password(password).show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

} // namespace dialogs
} // namespace gui2

#endif /* ! GUI_DIALOGS_MP_CREATE_GAME_SET_PASSWORD_HPP_INCLUDED */
