/*
	Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)

	Distributed under the Boost Software License, Version 1.0.
	(See http://www.boost.org/LICENSE_1_0.txt)
*/
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

#ifndef SERVER_MULTI_THREADED_SERVER_HPP
#define SERVER_MULTI_THREADED_SERVER_HPP

#include "umcd/server/basic_server.hpp"

class server_mt : public basic_server
{
private:
	typedef basic_server base_type;
	typedef boost::asio::ip::tcp::socket socket_type;
	typedef boost::shared_ptr<socket_type> socket_ptr;

public:
	explicit server_mt(const umcd::server_core& server_config, const boost::function<void(const socket_ptr&)> &request_handler);
	void run();
	std::size_t thread_pool_size() const;

private:
	std::size_t thread_pool_size_;
};

#endif // SERVER_MULTI_THREADED_SERVER_HPP
