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

#include "umcd/server/basic_server.hpp"
#include "umcd/logger/asio_logger.hpp"
#include <boost/make_shared.hpp>
#include <boost/current_function.hpp>

basic_server::basic_server()
: io_service_()
, acceptor_(io_service_)
, server_on_(false)
{}

void basic_server::start(const std::string& service)
{
	using namespace boost::asio::ip;

	// Find an endpoint on the service specified, if none found, throw a runtime_error exception.
	tcp::resolver resolver(io_service_);
	tcp::resolver::query query(service, tcp::resolver::query::address_configured);
	tcp::resolver::iterator endpoint_iter = resolver.resolve(query);
	tcp::resolver::iterator endpoint_end;
	tcp::endpoint endpoint;

	for(; endpoint_iter != endpoint_end; ++endpoint_iter)
	{
		try
		{
			endpoint = tcp::endpoint(*endpoint_iter);
			acceptor_.open(endpoint.protocol());
			acceptor_.bind(endpoint);
			acceptor_.listen();
			break;
		}
		catch(std::exception &e)
		{
			events_.signal_event<endpoint_failure>(e.what());
		}
	}
	if(endpoint_iter == endpoint_end)
	{
		events_.signal_event<start_failure>();
	}
	else
	{
		server_on_ = true;
		start_accept();
		events_.signal_event<start_success>(endpoint);
	}
}

void basic_server::run()
{
	while(server_on_)
	{
		try
		{
			io_service_.run();
		}
		catch(std::exception& e)
		{
			events_.signal_event<on_run_exception>(e);
		}
		catch(...)
		{
			events_.signal_event<on_run_unknown_exception>();
		}
	}
}

void basic_server::start_accept()
{
	socket_ptr socket = boost::make_shared<socket_type>(boost::ref(io_service_));
	acceptor_.async_accept(*socket,
		boost::bind(&basic_server::handle_accept, this, socket, boost::asio::placeholders::error)
	);
}

void basic_server::handle_accept(const socket_ptr& socket, const boost::system::error_code& e)
{
	if (!e)
	{
		events_.signal_event<on_new_client>(socket);
	}
	start_accept();
}

void basic_server::handle_stop()
{
	server_on_ = false;
	io_service_.stop();
}

boost::asio::io_service& basic_server::get_io_service()
{
	return io_service_;
}
