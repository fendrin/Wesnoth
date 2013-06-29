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

#ifndef CAMPAIGN_SERVER_WML_REQUEST_HPP
#define CAMPAIGN_SERVER_WML_REQUEST_HPP

#include <memory>
#include <istream>

#include "campaign_server/network_data.hpp"
#include "serialization/one_hierarchy_validator.hpp"
#include "serialization/parser.hpp"

class wml_request
{
private:
   network_data data;
public:
   wml_request(std::istream& raw_data_stream)
   {
      using namespace schema_validation;
      std::unique_ptr<one_hierarchy_validator> validator(new one_hierarchy_validator("test.cfg"));
      read(data.get_metadata(), raw_data_stream, validator.get());
      std::cout << "[Info] Request read:\n" << data.get_metadata();
   }

   network_data& get_data() { return data; }
};


#endif // CAMPAIGN_SERVER_WML_REQUEST_HPP
