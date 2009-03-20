/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file config.hpp
 * Definitions for the interface to Wesnoth Markup Language (WML).
 *
 * This module defines the interface to Wesnoth Markup Language (WML).  WML is
 * a simple hierarchical text-based file format.  The format is defined in
 * Wiki, under BuildingScenariosWML
 *
 * All configuration files are stored in this format, and data is sent across
 * the network in this format.  It is thus used extensively throughout the
 * game.
 */

#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include "global.hpp"

#include <boost/shared_ptr.hpp>

#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "game_errors.hpp"
#include "tstring.hpp"
#include "serialization/string_utils.hpp"


typedef std::map<std::string,t_string> string_map;

class config;

typedef boost::shared_ptr<config> config_ptr;

bool operator==(const config &, const config &);
inline bool operator!=(const config &a, const config &b) { return !operator==(a, b); }
std::ostream &operator << (std::ostream &, const config &);

/** A config object defines a single node in a WML file, with access to child nodes. */
class config
{
	friend bool operator==(const config& a, const config& b);

public:
	// Create an empty node.
	config();

	config(const config& cfg);

	// Create a config with an empty child of name 'child'.
	config(const std::string& child);
	~config();

	config& operator=(const config& cfg);

	typedef std::vector<config*> child_list;
	typedef std::map<std::string,child_list> child_map;

	struct const_child_iterator;

	struct child_iterator
	{
		typedef config value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef config *pointer;
		typedef config &reference;
		typedef child_list::iterator Itor;
		explicit child_iterator(const Itor &i = Itor()): i_(i) {}

		child_iterator &operator++() { ++i_; return *this; }
		child_iterator operator++(int) { return child_iterator(i_++); }

		config &operator*() const { return **i_; }
		config *operator->() const { return &**i_; }

		bool operator==(const child_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const child_iterator &i) const { return i_ != i.i_; }

	private:
		Itor i_;
		friend struct const_child_iterator;
	};

	struct const_child_iterator
	{
		typedef config value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef const config *pointer;
		typedef const config &reference;
		typedef child_list::const_iterator Itor;
		explicit const_child_iterator(const Itor &i = Itor()): i_(i) {}
		const_child_iterator(const child_iterator &i): i_(i.i_) {}

		const_child_iterator &operator++() { ++i_; return *this; }
		const_child_iterator operator++(int) { return const_child_iterator(i_++); }

		const config &operator*() const { return **i_; }
		const config *operator->() const { return &**i_; }

		bool operator==(const const_child_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const const_child_iterator &i) const { return i_ != i.i_; }

	private:
		Itor i_;
	};

	typedef std::pair<child_iterator,child_iterator> child_itors;
	typedef std::pair<const_child_iterator,const_child_iterator> const_child_itors;

	typedef std::pair<const std::string, t_string> attribute;

	struct const_attribute_iterator
	{
		typedef attribute value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef const attribute *pointer;
		typedef const attribute &reference;
		typedef string_map::const_iterator Itor;
		explicit const_attribute_iterator(const Itor &i = Itor()): i_(i) {}
		const_attribute_iterator(const const_attribute_iterator &i): i_(i.i_) {}

		const_attribute_iterator &operator++() { ++i_; return *this; }
		const_attribute_iterator operator++(int) { return const_attribute_iterator(i_++); }

		const attribute &operator*() const { return *i_; }
		const attribute *operator->() const { return &*i_; }

		bool operator==(const const_attribute_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const const_attribute_iterator &i) const { return i_ != i.i_; }

	private:
		Itor i_;
	};

	typedef std::pair<const_attribute_iterator,const_attribute_iterator> const_attr_itors;

	typedef std::pair<child_list::iterator, child_list::iterator> child_itors_bak;
	child_itors_bak child_range_bak(const std::string &);
	child_itors child_range(const std::string& key);
	const_child_itors child_range(const std::string& key) const;
	size_t child_count(const std::string& key) const;

	const child_list& get_children(const std::string& key) const;
	const child_map& all_children() const;

	config* child(const std::string& key);
	const config* child(const std::string& key) const;
	config& add_child(const std::string& key);
	config& add_child(const std::string& key, const config& val);
	config& add_child_at(const std::string& key, const config& val, size_t index);
	t_string& operator[](const std::string& key);
	const t_string& operator[](const std::string& key) const;

	const t_string& get_attribute(const std::string& key) const;
	bool has_attribute(const std::string& key) const {return values.find(key) != values.end();}
	void remove_attribute(const std::string& key) {values.erase(key);}
	void merge_attributes(const config &);

	const_attr_itors attribute_range() const;

	config* find_child(const std::string& key, const std::string& name,
	                   const t_string& value);
	const config* find_child(const std::string& key, const std::string& name,
	                         const t_string& value) const;

	void clear_children(const std::string& key);
	void remove_child(const std::string& key, size_t index);
	void recursive_clear_value(const std::string& key);

	void clear();
	bool empty() const;

	std::string debug() const;
	std::string hash() const;

	struct error : public game::error {
		error(const std::string& message) : game::error(message) {}
	};

	struct child_pos {
		child_pos(child_map::const_iterator p, size_t i) : pos(p), index(i) {}
		child_map::const_iterator pos;
		size_t index;

		bool operator==(const child_pos& o) const { return pos == o.pos && index == o.index; }
		bool operator!=(const child_pos& o) const { return !operator==(o); }
	};

	typedef std::pair<const std::string *, const config *> any_child;

	struct all_children_iterator
	{
		typedef any_child value_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		typedef any_child *pointer;
		typedef any_child &reference;
		typedef std::vector<child_pos>::const_iterator Itor;
		explicit all_children_iterator(const Itor &i = Itor()): i_(i) {}

		all_children_iterator &operator++() { ++i_; return *this; }
		all_children_iterator operator++(int) { return all_children_iterator(i_++); }

		struct arrow_helper
		{
			any_child data;
			arrow_helper(const all_children_iterator &i): data(*i) {}
			any_child *operator->() { return &data; }
		};

		value_type operator*() const;
		arrow_helper operator->() const { return *this; }

		const std::string& get_key() const;
		size_t get_index() const;
		const config& get_child() const;

		bool operator==(const all_children_iterator &i) const { return i_ == i.i_; }
		bool operator!=(const all_children_iterator &i) const { return i_ != i.i_; }

	private:
		Itor i_;
	};

	/** In-order iteration over all children. */
	all_children_iterator ordered_begin() const;
	all_children_iterator ordered_end() const;
	all_children_iterator erase(const all_children_iterator& i);

	/**
	 * A function to get the differences between this object,
	 * and 'c', as another config object.
	 * I.e. calling cfg2.apply_diff(cfg1.get_diff(cfg2))
	 * will make cfg1 identical to cfg2.
	 */
	config get_diff(const config& c) const;
	void get_diff(const config& c, config& res) const;

	void apply_diff(const config& diff); //throw error

	/**
	 * Merge config 'c' into this config.
	 * Overwrites this config's values.
	 */
	void merge_with(const config& c);

	/**
	 * Merge config 'c' into this config.
	 * Keeps this config's values and does not add existing elements.
	 * NOTICE: other nonstandard behavior includes no merge recursion into child
	 * and has limited merging for child lists of differing lengths
	 */
    void merge_and_keep(const config& c);

	bool matches(const config &filter) const;

	/** Removes keys with empty values. */
	void prune();

	/**
	 * Append data from another config object to this one.
	 * Attributes in the latter config object will clobber attributes in this one.
	 */
	void append(const config& cfg);

	/**
	 * All children with the given key will be merged
	 * into the first element with that key.
	 */
	void merge_children(const std::string& key);

	/** Resets the translated values of all strings contained in this object */
	void reset_translation() const;

	//this is a cheap O(1) operation
	void swap(config& cfg);

private:
	/** All the attributes of this node. */
	string_map values;

	/** A list of all children of this node. */
	child_map children;

	std::vector<child_pos> ordered_children;
};

#endif
