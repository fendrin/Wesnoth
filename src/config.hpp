/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include <map>
#include <string>
#include <vector>

struct line_source
{
	line_source(int ln,const std::string& fname, int line) :
	              linenum(ln), file(fname), fileline(line)
	{}

	int linenum;
	std::string file;
	int fileline;
};

bool operator<(const line_source& a, const line_source& b);

std::string read_file(const std::string& fname);
void write_file(const std::string& fname, const std::string& data);
std::string preprocess_file(const std::string& fname,
                            const std::map<std::string,std::string>* defines=0,
                            std::vector<line_source>* src=0);

typedef std::map<std::string,std::string> string_map;

struct config
{
	config() {}
	config(const std::string& data,
	       const std::vector<line_source>* lines=0); //throws config::error
	config(const config& cfg);
	~config();

	config& operator=(const config& cfg);

	void read(const std::string& data,
	          const std::vector<line_source>* lines=0); //throws config::error
	std::string write() const;

	typedef std::vector<config*> child_list;
	typedef std::map<std::string,child_list> child_map;
	child_map children;
	string_map values;

	typedef std::vector<config*>::iterator child_iterator;
	typedef std::vector<config*>::const_iterator const_child_iterator;

	typedef std::pair<child_iterator,child_iterator> child_itors;
	typedef std::pair<const_child_iterator,const_child_iterator>
	                                                  const_child_itors;

	child_itors child_range(const std::string& key);
	const_child_itors child_range(const std::string& key) const;
	
	config* child(const std::string& key);
	const config* child(const std::string& key) const;
	config& add_child(const std::string& key);
	std::string& operator[](const std::string& key);
	const std::string& operator[](const std::string& key) const;

	config* find_child(const std::string& key, const std::string& name,
	                   const std::string& value);
	const config* find_child(const std::string& key, const std::string& name,
	                         const std::string& value) const;

	static std::vector<std::string> split(const std::string& val);
	static std::string& strip(std::string& str);
	static bool has_value(const std::string& values, const std::string& val);

	void clear();

	struct error {
		error(const std::string& msg) : message(msg) {}
		std::string message;
	};
};

struct config_has_value {
	config_has_value(const std::string& name, const std::string& value)
	              : name_(name), value_(value)
	{}

	bool operator()(const config* cfg) const { return (*cfg)[name_] == value_; }

private:
	const std::string name_, value_;
};

#endif
