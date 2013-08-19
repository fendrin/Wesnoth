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

#ifndef UMCD_WML_REPLY_HPP
#define UMCD_WML_REPLY_HPP

#include <ostream>
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

class config;

namespace umcd{

class wml_reply
{
public:
	wml_reply();
	wml_reply(const config& metadata);
	std::vector<boost::asio::const_buffer> to_buffers() const;

private:
	std::string metadata_;
	boost::uint32_t payload_size_;
};

} // namespace umcd
#endif // UMCD_WML_REPLY_HPP
