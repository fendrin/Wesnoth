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

#ifndef UMCD_REQUEST_INFO_HPP
#define UMCD_REQUEST_INFO_HPP

#include <boost/make_shared.hpp>
#include "serialization/schema_validator.hpp"
#include "umcd/actions/basic_umcd_action.hpp"
#include "filesystem.hpp"
#include "umcd/env/server_info.hpp"

namespace umcd{
class request_info
{
public:
	typedef schema_validation::schema_validator validator_type;
	typedef boost::shared_ptr<basic_umcd_action> action_ptr;
	typedef boost::shared_ptr<validator_type> validator_ptr;

public:
	request_info(const action_ptr& action, const validator_ptr& validator);
	action_ptr action();
	validator_ptr validator();
	boost::shared_ptr<request_info> clone() const;

private:
	action_ptr umcd_action_;
	validator_ptr request_validator_;
};

template <class Action, class Validator>
boost::shared_ptr<request_info> make_request_info(const server_info& info, const std::string& request_name)
{
	return boost::make_shared<request_info>(
		boost::make_shared<Action>(info),
		boost::make_shared<Validator>(
			info.wesnoth_dir() + get_umcd_protocol_schema_dir() + "/" + request_name+".cfg"));
}

} // namespace umcd
#endif // UMCD_REQUEST_INFO_HPP