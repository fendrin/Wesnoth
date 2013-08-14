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

#ifndef CPP_GENERATOR_HPP
#define CPP_GENERATOR_HPP

#include "tools/code_generator/sql2cpp/preprocessor_rule_helper.hpp"
#include "tools/code_generator/sql2cpp/cpp/semantic_actions.hpp"

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace cpp{

namespace karma = boost::spirit::karma;
namespace phx = boost::phoenix;

template <typename OutputIterator>
struct grammar 
: karma::grammar<OutputIterator, sql::ast::schema()>
{
	typedef OutputIterator iterator_type;

	grammar(semantic_actions& sa)
	: grammar::base_type(schema)
	, sa_(sa)
	{
		using karma::eol;

		RULE_DEF(schema,
			= +file
			);

		RULE_DEF(file,
			= karma::eps [phx::bind(&semantic_actions::open_sink, &sa_, phx::at_c<0>(karma::_val))]
			<< header [karma::_1 = karma::_val] 
			<< structure [karma::_1 = karma::_val] 
			<< footer
			);

		RULE_DEF(header,
			= license_header << eol
			<< do_not_modify << eol
			<< define_header
			<< includes << eol
			<< namespace_open << eol
			);

		RULE_DEF(namespace_open,
			= "namespace pod{\n"
			);

		RULE_DEF(namespace_close,
			= "} // namespace pod\n"
			);

		RULE_DEF(do_not_modify,
			=  "// WARNING: This file has been auto-generated with the tool sql2cpp. We keep in sync the SQL schema and the POD classes."
			<< eol
			<< "//          Please do not modify this file by hand. Modify the SQL schema and rebuild the project.\n"
			);

		RULE_NDEF(includes,
			= karma::string [phx::bind(&semantic_actions::includes, &sa_, karma::_1, karma::_val, karma::_a)]
			);

		RULE_DEF(footer,
			= namespace_close
			<< define_footer
			);

		RULE_DEF(define_footer,
			= "#endif\n"
			);

		RULE_DEF(define_header,
			= karma::eps [phx::bind(&semantic_actions::define_name, &sa_, karma::_a, karma::_val)]
			<< "#ifndef "
			<< karma::string [karma::_1 = karma::_a]
			<< "\n#define "
			<< karma::string [karma::_1 = karma::_a]
			<< "\n\n"
			);

		RULE_DEF(license_header,
			= "/*\n" 
			<< karma::string [phx::bind(&semantic_actions::license_header, &sa_, karma::_1)]
			<< "\n*/\n"
			);

		RULE_DEF(structure,
			= "struct " 
			<< karma::string [karma::_1 = phx::at_c<0>(karma::_val)]
			<< "\n{\n"
			<< members [karma::_1 = phx::at_c<1>(karma::_val)]
			<< "};\n\n"
			);

		RULE_DEF(members,
			= *('\t' << member << ";\n")
			);

		RULE_DEF(member,
			= member_type [karma::_1 = phx::at_c<1>(karma::_val)] 
			<< ' ' 
			<< karma::string [karma::_1 = phx::at_c<0>(karma::_val)]
			);

		RULE_DEF(member_type,
			= karma::string [phx::bind(&semantic_actions::type2string, &sa_, karma::_1, karma::_val)]
			);
	}

private:
	semantic_actions &sa_;

	KA_RULE(sql::ast::schema(), schema);
	KA_RULE(sql::ast::table(), file);
	KA_RULE(sql::ast::table(), structure);
	KA_RULE(sql::ast::table(), header);
	KA_RULE_LOC(std::string(), std::string, define_header);
	KA_RULE_LOC(sql::ast::column_list(), std::set<std::string>, includes);

	KA_RULE0(footer);
	KA_RULE0(define_footer);
	KA_RULE0(license_header);
	KA_RULE0(namespace_open);
	KA_RULE0(namespace_close);
	KA_RULE0(do_not_modify);

	KA_RULE(sql::ast::column_list(), members);
	KA_RULE(sql::ast::column(), member);
	KA_RULE(sql::ast::column_type_ptr(), member_type);
};

template <typename OutputIterator>
bool generate(OutputIterator& sink, sql::ast::schema const& sql_ast, std::ofstream& generated, const std::string& output_dir)
{
	semantic_actions sa("../", generated, output_dir);
	grammar<OutputIterator> cpp_grammar(sa);
	return karma::generate(sink, cpp_grammar, sql_ast);
}

} // namespace cpp
#endif // CPP_GENERATOR_HPP
