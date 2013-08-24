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

#ifndef UMCD_PROTOCOL_HPP
#define UMCD_PROTOCOL_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/cstdint.hpp>
	 
#include "wml_exception.hpp"

#include "umcd/env/environment.hpp"
#include "umcd/logger/asio_logger.hpp"
#include "umcd/request_info.hpp"
#include "umcd/protocol/error_sender.hpp"

namespace umcd{

class wml_request;
class protocol_info;

class protocol : 
		public boost::enable_shared_from_this<protocol>
	,	private boost::noncopyable
{
public:
	static std::size_t REQUEST_HEADER_MAX_SIZE;
	typedef boost::asio::ip::tcp::socket socket_type;
	typedef boost::shared_ptr<socket_type> socket_ptr;
	typedef boost::asio::io_service io_service_type;

private:
	typedef basic_umcd_action action_type;
	typedef boost::shared_ptr<request_info> info_ptr;

public:
	static void load(const protocol_info& proto_info);

	// This constructor is only called once in main, so the factory will be created once as well.
	protocol(io_service_type& io_service, const environment& env);

	void handle_request();
	// Precondition: handle_request has been called and connection has been initialized.
	void async_send_reply();

	config& get_reply();

	socket_type& socket();

private:
	void on_error(const boost::system::error_code& error);
	void complete_request(const boost::system::error_code& error, std::size_t bytes_transferred);

	void async_send_invalid_packet(const std::string &where, const std::exception& e);
	void async_send_invalid_packet(const std::string &where, const twml_exception& e);
	
private:
	const environment& environment_;
	socket_ptr socket_;
	config header_metadata_;
	config reply_;
};

boost::shared_ptr<protocol> make_protocol(protocol::io_service_type& io_service, const environment& env);

} // namespace umcd

#endif // UMCD_PROTOCOL_HPP
