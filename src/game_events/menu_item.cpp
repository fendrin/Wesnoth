/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Definitions for a class that implements WML-defined (right-click) menu items.
 */

#include "../global.hpp"
#include "menu_item.hpp"
#include "conditional_wml.hpp"
#include "handlers.hpp"

#include "../game_config.hpp"
#include "../gamestatus.hpp"
#include "../log.hpp"
#include "../resources.hpp"
#include "../terrain_filter.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)


// This file is in the game_events namespace.
namespace game_events
{

wml_menu_item::wml_menu_item(const std::string& id, const config* cfg) :
		item_id_(id),
		event_name_("menu item" + (id.empty() ? "" : ' ' + id)),
		image_(),
		description_(),
		needs_select_(false),
		show_if_(vconfig::empty_vconfig()),
		filter_location_(vconfig::empty_vconfig()),
		command_(),
		default_hotkey_(),
		use_hotkey_(true),
		use_wml_menu_(true)
{
	if(cfg != NULL) {
		image_ = (*cfg)["image"].str();
		description_ = (*cfg)["description"];
		needs_select_ = (*cfg)["needs_select"].to_bool();
		// use_hotkey is already a name of a member of this class.
		const config::attribute_value & use_hotkey_attribute_value = (*cfg)["use_hotkey"];
		if(use_hotkey_attribute_value.str() == "only" )
		{
			use_hotkey_ = true;
			use_wml_menu_ = false;
		}
		else
		{	
			use_hotkey_ = use_hotkey_attribute_value.to_bool(true);
			use_wml_menu_ = true;
		}
		if (const config &c = cfg->child("show_if")) show_if_ = vconfig(c, true);
		if (const config &c = cfg->child("filter_location")) filter_location_ = vconfig(c, true);
		if (const config &c = cfg->child("command")) command_ = c;
		if (const config &c = cfg->child("default_hotkey")) default_hotkey_ = c;
	}
}

/**
 * The image associated with this menu item.
 * The returned string will not be empty; a default will be supplied if needed.
 */
const std::string & wml_menu_item::image() const
{
	// Default the image?
	return image_.empty() ? game_config::images::wml_menu : image_;
}

/**
 * Returns whether or not *this is applicable given the context.
 * Assumes game variables x1, y1, and unit have been set.
 * @param[in]  hex  The hex where the menu will appear.
 */
bool wml_menu_item::can_show(const map_location & hex) const
{
	// Failing the [show_if] tag means no show.
	if ( !show_if_.empty() && !conditional_passed(show_if_) )
		return false;

	// Failing the [fiter_location] tag means no show.
	if ( !filter_location_.empty() &&
	     !terrain_filter(filter_location_, *resources::units)(hex) )
		return false;

	// Failing to have a required selection means no show.
	if ( needs_select_ && !resources::gamedata->last_selected.valid() )
		return false;

	// Passed all tests.
	return true;
}

/**
 * Writes *this to the provided config.
 */
void wml_menu_item::to_config(config & cfg) const
{
	cfg["id"] = item_id_;
	cfg["image"] = image_;
	cfg["description"] = description_;
	cfg["needs_select"] = needs_select_;

	if ( use_hotkey_ && use_wml_menu_ )
		cfg["use_hotkey"] = true;
	if ( use_hotkey_ && !use_wml_menu_ )
		cfg["use_hotkey"] = "only";
	if ( !use_hotkey_ && use_wml_menu_ )
		cfg["use_hotkey"] = false;
	if ( !use_hotkey_ && !use_wml_menu_ )
	{
		ERR_NG << "Bad data: wml_menu_item with both use_wml_menu and use_hotkey set to false is not supposed to be possible.";
		cfg["use_hotkey"] = false;
	}
	if ( !show_if_.empty() )
		cfg.add_child("show_if", show_if_.get_config());
	if ( !filter_location_.empty() )
		cfg.add_child("filter_location", filter_location_.get_config());
	if ( !command_.empty() )
		cfg.add_child("command", command_);
	if ( !default_hotkey_.empty() )
		cfg.add_child("default_hotkey", default_hotkey_);
}

/**
 * Updates *this based on @a vcfg.
 */
void wml_menu_item::update(const vconfig & vcfg)
{
	if ( vcfg.has_attribute("image") )
		image_ = vcfg["image"].str();

	if ( vcfg.has_attribute("description") )
		description_ = vcfg["description"].t_str();

	if ( vcfg.has_attribute("needs_select") )
		needs_select_ = vcfg["needs_select"].to_bool();

	if ( const vconfig & child = vcfg.child("show_if") ) {
		show_if_ = child;
		show_if_.make_safe();
	}

	if ( const vconfig & child = vcfg.child("filter_location") ) {
		filter_location_ = child;
		filter_location_.make_safe();
	}

	if ( const vconfig & child = vcfg.child("default_hotkey") ) {
		default_hotkey_ = child.get_parsed_config();
	}

	if ( vcfg.has_attribute("use_hotkey") ) {
		const config::attribute_value & use_hotkey_attribute_value = vcfg["use_hotkey"];
		if(use_hotkey_attribute_value.str() == "only")
		{
			use_hotkey_ = true;
			use_wml_menu_ = false;
		}
		else
		{
			use_hotkey_ = use_hotkey_attribute_value.to_bool(true);
			use_wml_menu_ = true;
		}
	}

	if ( const vconfig & cmd = vcfg.child("command") ) {
		const bool delayed = cmd["delayed_variable_substitution"].to_bool(true);
		add_wmi_change(item_id_, delayed ? cmd.get_config() : cmd.get_parsed_config());
	}
}

} // end namespace game_events

