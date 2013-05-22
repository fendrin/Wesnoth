/*
   Copyright (C) 2012 - 2013 by Fabian Mueller <fabianmueller5@gmx.de>
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
 * Manage the item-palette in the editor.
 */

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "item_palette.hpp"

#include "../../gettext.hpp"

//#include "../../item_types.hpp"

#include <boost/foreach.hpp>
#include "config.h"

#include <string>

namespace editor {

std::string item_palette::get_help_string() {
	return selected_fg_item().name;
}

void item_palette::setup(const config& cfg)
{

	BOOST_FOREACH(const config& group, cfg.child_range("item_group")) {

		groups_.push_back(item_group(group));

		BOOST_FOREACH(const config& item, group.child_range("item")) {

			item_map_.insert(std::pair<std::string, overlay>(item["id"], overlay(item)));
			group_map_[group["id"]].push_back(item["id"]);
		}
		nmax_items_ = std::max(nmax_items_, group_map_[group["id"]].size());
	}

//	BOOST_FOREACH(const item_type_data::item_type_map::value_type &i, item_types.types())
//	{
//		item_map_.insert(std::pair<std::string, item_type>(i.second.id(), i.second));
//		group_map_[i.second.race_id()].push_back(i.second.id());
//		nmax_items_ = std::max(nmax_items_, group_map_[i.second.race_id()].size());
//		//TODO
//		bool core = true;
//		if (core) {
//			// Add the item to the default group
//			group_map_["all"].push_back(i.second.id());
//			nmax_items_ = std::max(nmax_items_, group_map_["all"].size());
//		} else {
//			non_core_items_.insert(i.second.id());
//		}
//	}
//
//	BOOST_FOREACH(const race_map::value_type &i, item_types.races())
//	{
//		config cfg;
//		cfg["id"] = i.second.id();
//		cfg["name"] = i.second.plural_name();
//		cfg["icon"] = "icons/item-groups/race_" + i.second.id();
//		cfg["core"] = "yes";
//		groups_.push_back(item_group(cfg));
//	}
//
//	//TODO
//	//move "invalid" items to the end
//	//std::stable_partition(items.begin(), items.end(), is_valid_terrain);
//
	select_fg_item("anvil");
	select_bg_item("altar");
//
//	// Set the default group
	set_group("items");
//
//	if(active_group().empty()) {
//		ERR_ED << "No items found.\n";
//	}
}

void item_palette::draw_item(const overlay& item, surface& image, std::stringstream& tooltip_text) {

	surface screen = gui_.video().getSurface();

	std::stringstream filename;
	filename << item.image; // << "~RC(" << u.flag_rgb() << '>'
	    	 // << team::get_side_color_index(gui_.viewing_side()) << ')';

	image = surface(image::get_image(filename.str()));
	if(image == NULL) {
		tooltip_text << "IMAGE NOT FOUND\n";
		ERR_ED << "image for item type: '" << filename.str() << "' not found\n";
		image = image::get_image(game_config::images::missing);
		if (image == NULL) {
			ERR_ED << "Placeholder image not found\n";
			return;
		}
	}

	if(image->w != item_size_ || image->h != item_size_) {
		image.assign(scale_surface(image,
				item_size_, item_size_));
	}

	tooltip_text << item.name;
}

item_palette::item_palette(editor_display &gui, const config& cfg,
								 mouse_action** active_mouse_action)
//TODO avoid magic numbers
	:	editor_palette<overlay>(gui, cfg, 36, 4, active_mouse_action)
{
}

const std::string& item_palette::get_id(const overlay& item)
{
	return item.id;
}


}

