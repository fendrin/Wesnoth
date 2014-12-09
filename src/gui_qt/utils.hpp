/*
   Copyright (C) 2014 - 2015 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef QT_UTILS_INCLUDED
#define QT_UTILS_INCLUDED

#include <string>

namespace gui_qt {

	/**
	 * Helper function to display QML Applications.
	 * @param qml_file_path containing the application definition.
	 * @return the exit status of the Qt application.
	 */
	int show_application(const std::string& qml_file_path, int argc = 0, char** argv = NULL);

	/**
	 * Helper function to display QMLQuick dialogs.
	 * @param qml_file_path containing the gui element definition.
	 */
	void show_quick(const std::string& qml_file_path);

}

#endif
