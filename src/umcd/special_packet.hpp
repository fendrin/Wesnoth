/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifndef UMCD_ERROR_PACKET_HPP
#define UMCD_ERROR_PACKET_HPP

#include <string>

class config;

namespace umcd{

class wml_reply;
	
config make_error_packet(const std::string& message);
config make_warning_packet(const std::string& message);

wml_reply make_error_reply(const std::string& message);
wml_reply make_warning_reply(const std::string& message);

} // namespace umcd
#endif // UMCD_ERROR_PACKET_HPP
