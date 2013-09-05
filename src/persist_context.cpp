/*
   Copyright (C) 2010 - 2013 by Jody Northup
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "filesystem.hpp"
#include "log.hpp"
#include "persist_context.hpp"
#include "persist_manager.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "util.hpp"

config pack_scalar(const std::string &name, const t_string &val)
{
	config cfg;
	cfg[name] = val;
	return cfg;
}

static std::string get_persist_cfg_name(const std::string &name_space) {
	return (get_dir(get_user_data_dir() + "/persist/") + name_space + ".cfg");
}

void persist_file_context::load()
{
	std::string cfg_name = get_persist_cfg_name(namespace_.root_);
	if (file_exists(cfg_name) && !is_directory(cfg_name)) {
		scoped_istream file_stream = istream_file(cfg_name);
		if (!(file_stream->fail())) {
			try {
				read(cfg_,*file_stream);
			} catch (config::error &err) {
				LOG_PERSIST << err.message;
			}
		}
	}
}

persist_file_context::persist_file_context(const std::string &name_space)
	: persist_context(name_space)
{
	load();
}

bool persist_file_context::clear_var(const std::string &global, bool immediate)
{
	config bak;
	config bactive;
	if (immediate) {
		bak = cfg_;
		config *node = get_node(bak, namespace_);
		if (node)
			bactive = node->child_or_add("variables");
		load();
	}
	config *active = get_node(cfg_, namespace_);
	if (active == NULL)
		return false;

	bool ret = active->has_child("variables");
	if (ret) {
		config &cfg = active->child("variables");
		bool exists = cfg.has_attribute(global);
		if (!exists) {
			if (cfg.has_child(global)) {
				exists = true;
				std::string::const_iterator index_start = std::find(global.begin(),global.end(),'[');
				if (index_start != global.end())
				{
					const std::string::const_iterator index_end = std::find(global.begin(),global.end(),']');
					const std::string index_str(index_start+1,index_end);
					size_t index = static_cast<size_t>(lexical_cast_default<int>(index_str));
					cfg.remove_child(global,index);
					if (immediate) bactive.remove_child(global,index);
				} else {
					cfg.clear_children(global);
					if (immediate) bactive.clear_children(global);
				}
			}
		}
		if (exists) {
			cfg.remove_attribute(global);
			if (immediate) bactive.remove_attribute(global);
			if (cfg.empty()) {
				active->clear_children("variables");
				active->remove_attribute("variables");
				name_space working = namespace_;
				while ((active->empty()) && (!working.lineage_.empty())) {
					name_space prev = working.prev();
					active = get_node(cfg_, prev);
					active->clear_children(working.node_);
					if (active->has_child("variables") && active->child("variables").empty()) {
						active->clear_children("variables");
						active->remove_attribute("variables");
					}
					working = prev;
				}
			}

			if (!in_transaction_)
				ret = save_context();
			else if (immediate) {
				ret = save_context();
				cfg_ = bak;
				active = get_node(cfg_, namespace_);
				if (active != NULL) {
					active->clear_children("variables");
					active->remove_attribute("variables");
					if (!bactive.empty())
						active->add_child("variables",bactive);
				}
			} else {
				ret = true;
			}
		} else {
			if (immediate) {
				cfg_ = bak;
				config *active = get_node(cfg_, namespace_);
				if (active != NULL) {
					active->clear_children("variables");
					active->remove_attribute("variables");
					if (!bactive.empty())
						active->add_child("variables",bactive);
				}
			}
			ret = exists;
		}
	}
	while ((active->empty()) && (!namespace_.lineage_.empty())) {
		name_space prev = namespace_.prev();
		active = get_node(cfg_, prev);
		/// @todo: This assertion replaces a seg fault. Still need to fix the
		/// real bug (documented as bug #21093).
		assert(active != NULL);
		active->clear_children(namespace_.node_);
		if (active->has_child("variables") && active->child("variables").empty()) {
			active->clear_children("variables");
			active->remove_attribute("variables");
		}
		namespace_ = prev;
	}
	return ret;
}

config persist_file_context::get_var(const std::string &global) const
{
	config ret;
	const config *active = get_node(cfg_, namespace_);
	if (active && (active->has_child("variables"))) {
		const config &cfg = active->child("variables");
		size_t arrsize = cfg.child_count(global);
		if (arrsize > 0) {
			for (size_t i = 0; i < arrsize; i++)
				ret.add_child(global,cfg.child(global,i));
		} else {
			ret = pack_scalar(global,cfg[global]);
		}
	} else {
		ret = pack_scalar(global,"");
	}
	return ret;
}
bool persist_file_context::save_context() {
	bool success = false;

	std::string cfg_name = get_persist_cfg_name(namespace_.root_);
	if (!cfg_name.empty()) {
		if (cfg_.empty()) {
			success = delete_directory(cfg_name);
		} else {
			scoped_ostream out = ostream_file(cfg_name);
			if (!out->fail())
			{
				config_writer writer(*out,false);
				try {
					writer.write(cfg_);
					success = true;
				} catch(config::error &err) {
					LOG_PERSIST << err.message;
					success = false;
				}
			}
		}
	}
	return success;
}
bool persist_file_context::set_var(const std::string &global,const config &val, bool immediate)
{
	config bak;
	config bactive;
	if (immediate) {
		bak = cfg_;
		bactive = get_node(bak, namespace_, true)->child_or_add("variables");
		load();
	}

	config *active = get_node(cfg_, namespace_, true);
	config &cfg = active->child_or_add("variables");
	if (val.has_attribute(global)) {
		if (val[global].empty()) {
			clear_var(global,immediate);
		} else {
			cfg[global] = val[global];
			if (immediate) bactive[global] = val[global];
		}
	} else {
		cfg.clear_children(global);
		cfg.append(val);
		if (immediate) {
			bactive.clear_children(global);
			bactive.append(val);
		}
	}
//	dirty_ = true;
	if (!in_transaction_)
		return save_context();
	else if (immediate) {
		bool ret = save_context();
		cfg_ = bak;
		active = get_node(cfg_, namespace_, true);
		active->clear_children("variables");
		active->remove_attribute("variables");
		active->add_child("variables",bactive);
		return ret;
	} else
		return true;
}

void persist_context::set_node(const std::string &name) {
	std::string newspace = namespace_.root_;
	if (!name.empty())
		newspace += "." + name;
	namespace_ = name_space(newspace);
}

std::string persist_context::get_node() const
{
	return namespace_.namespace_;
}

