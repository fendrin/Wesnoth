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

#include "umcd/actions/request_umc_upload_action.hpp"
#include "umcd/otl/otl.hpp"
#include "filesystem.hpp"
#include "umcd/protocol/wml/umcd_protocol.hpp"

request_umc_upload_action::request_umc_upload_action(const server_info& info)
: server_info_(info)
{}

const config& request_umc_upload_action::get_info(const config& metadata)
{
	return metadata.child("request_umc_upload").child("umc_configuration").child("info");
}

void request_umc_upload_action::execute(boost::shared_ptr<umcd_protocol> p)
{
	protocol_ = p;
	//config& metadata = protocol_->get_metadata();
}

boost::shared_ptr<request_umc_upload_action::base> request_umc_upload_action::clone() const
{
	return boost::shared_ptr<base>(new request_umc_upload_action(*this));
}
