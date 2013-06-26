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

#ifndef SERVER_SINGLE_THREAD_SERVER_HPP
#define SERVER_SINGLE_THREAD_SERVER_HPP

#include <memory>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include "campaign_server/server/connection.hpp"

template <class Protocol>
class server : boost::noncopyable
{
  public:
    typedef Protocol protocol_type;
  private:
    typedef connection<protocol_type> connection_type;
    typedef std::shared_ptr<connection_type> connection_ptr;

    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;

    connection_ptr new_connection;
    protocol_type& protocol;

  public:
    explicit server(const config& cfg, protocol_type& protocol);
    void run();

  private:
    void start_accept();
    void handle_accept(const boost::system::error_code& e);
    void handle_stop();
};

template <class Protocol>
server<Protocol>::server(const config &cfg, protocol_type& protocol) 
: acceptor(io_service)
, protocol(protocol)
{
  using namespace boost::asio::ip;

  // Find an endpoint on the port specified, if none found, throw a runtime_error exception.
  std::string port = cfg["port"];
  tcp::resolver resolver(io_service);
  tcp::resolver::query query(port, tcp::resolver::query::address_configured);
  tcp::resolver::iterator endpoint_iter = resolver.resolve(query);
  tcp::resolver::iterator endpoint_end;

  for(; endpoint_iter != endpoint_end; ++endpoint_iter)
  {
    try
    {
      tcp::endpoint endpoint(*endpoint_iter);
      acceptor.open(endpoint.protocol());
      acceptor.bind(endpoint);
      acceptor.listen();
      break;
    }
    catch(std::exception &e)
    {
      std::cerr << e.what() << std::endl;
    }
  }
  if(endpoint_iter == endpoint_end)
  {
    throw std::runtime_error("No endpoints found - Check the status of your network interfaces.\n");
  }
  start_accept();
}

template <class Protocol>
void server<Protocol>::run()
{
  io_service.run();
}

template <class Protocol>
void server<Protocol>::start_accept()
{
  new_connection = std::make_shared<connection_type>(io_service, protocol);
  acceptor.async_accept(new_connection->get_socket(),  
    boost::bind(&server::handle_accept, this, boost::asio::placeholders::error)
  );
}

template <class Protocol>
void server<Protocol>::handle_accept(const boost::system::error_code& e)
{
  if (!e)
  {
    new_connection->start();
  }
  start_accept();
}

template <class Protocol>
void server<Protocol>::handle_stop()
{
  io_service.stop();
}

#endif // SERVER_SINGLE_THREAD_SERVER_HPP