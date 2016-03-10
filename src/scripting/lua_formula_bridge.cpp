/*
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 
 See the COPYING file for more details.
 */

#include "scripting/lua_formula_bridge.hpp"

#include "boost/variant/static_visitor.hpp"

#include "scripting/game_lua_kernel.hpp"
#include "scripting/lua_api.hpp"
#include "scripting/lua_common.hpp"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "variable.hpp"

void luaW_pushfaivariant(lua_State* L, variant val);
variant luaW_tofaivariant(lua_State* L, int i);

using namespace game_logic;

class lua_callable : public formula_callable {
	lua_State* mState;
	int table_i;
public:
	lua_callable(lua_State* L, int i) : mState(L), table_i(lua_absindex(L,i)) {}
	variant get_value(const std::string& key) const {
		if(key == "__list") {
			std::vector<variant> values;
			size_t n = lua_rawlen(mState, table_i);
			if(n == 0) {
				return variant();
			}
			for(size_t i = 1; i <= n; i++) {
				lua_pushinteger(mState, i);
				lua_gettable(mState, table_i);
				values.push_back(luaW_tofaivariant(mState, -1));
			}
			return variant(&values);
		} else if(key == "__map") {
			std::map<variant,variant> values;
			for(lua_pushnil(mState); lua_next(mState, table_i); lua_pop(mState, 1)) {
				values[luaW_tofaivariant(mState, -2)] = luaW_tofaivariant(mState, -1);
			}
			return variant(&values);
		}
		lua_pushlstring(mState, key.c_str(), key.size());
		lua_gettable(mState, table_i);
		variant result = luaW_tofaivariant(mState, -1);
		lua_pop(mState, 1);
		return result;
	}
	void get_inputs(std::vector<formula_input>* inputs) const {
		inputs->push_back(formula_input("__list", FORMULA_READ_ONLY));
		inputs->push_back(formula_input("__map", FORMULA_READ_ONLY));
		for(lua_pushnil(mState); lua_next(mState, table_i); lua_pop(mState,1)) {
			if(lua_isstring(mState, -2) && !lua_isnumber(mState, -2)) {
				std::string key = lua_tostring(mState, -2);
				if(key.find_first_not_of(formula::id_chars) != std::string::npos) {
					inputs->push_back(formula_input(key, FORMULA_READ_ONLY));
				}
			}
		}
	}
};

void luaW_pushfaivariant(lua_State* L, variant val) {
	if(val.is_int()) {
		lua_pushinteger(L, val.as_int());
	} else if(val.is_decimal()) {
		lua_pushnumber(L, val.as_decimal() / 1000.0);
	} else if(val.is_string()) {
		const std::string result_string = val.as_string();
		lua_pushlstring(L, result_string.c_str(), result_string.size());
	} else if(val.is_list()) {
		lua_newtable(L);
		for(const variant& v : val.as_list()) {
			lua_pushinteger(L, lua_rawlen(L, -1) + 1);
			luaW_pushfaivariant(L, v);
			lua_settable(L, -3);
		}
	} else if(val.is_map()) {
		typedef std::map<variant,variant>::value_type kv_type;
		lua_newtable(L);
		for(const kv_type& v : val.as_map()) {
			luaW_pushfaivariant(L, v.first);
			luaW_pushfaivariant(L, v.second);
			lua_settable(L, -3);
		}
	} else if(val.is_callable()) {
		// First try a few special cases (well, currently, only one)
		if(unit_callable* u_ref = val.try_convert<unit_callable>()) {
			const unit& u = u_ref->get_unit();
			unit_map::iterator un_it = resources::units->find(u.get_location());
			if(&*un_it == &u) {
				new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(u.underlying_id());
			} else {
				new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(u.side(), u.underlying_id());
			}
			lua_pushlightuserdata(L, getunitKey);
			lua_rawget(L, LUA_REGISTRYINDEX);
			lua_setmetatable(L, -2);
		} else {
			// If those fail, convert generically to a map
			const formula_callable* obj = val.as_callable();
			std::vector<formula_input> inputs;
			obj->get_inputs(&inputs);
			lua_newtable(L);
			for(const formula_input& attr : inputs) {
				if(attr.access == FORMULA_WRITE_ONLY) {
					continue;
				}
				lua_pushstring(L, attr.name.c_str());
				luaW_pushfaivariant(L, obj->query_value(attr.name));
				lua_settable(L, -3);
			}
		}
	} else if(val.is_null()) {
		lua_pushnil(L);
	}
}

variant luaW_tofaivariant(lua_State* L, int i) {
	switch(lua_type(L, i)) {
		case LUA_TBOOLEAN:
			return variant(lua_tointeger(L, i));
		case LUA_TNUMBER:
			return variant(lua_tonumber(L, i), variant::DECIMAL_VARIANT);
		case LUA_TSTRING:
			return variant(lua_tostring(L, i));
		case LUA_TTABLE:
			return variant(new lua_callable(L, i));
		case LUA_TUSERDATA:
			static t_string tstr;
			static vconfig vcfg = vconfig::unconstructed_vconfig();
			if(luaW_totstring(L, i, tstr)) {
				return variant(tstr.str());
			} else if(luaW_tovconfig(L, i, vcfg)) {
				return variant(new config_callable(vcfg.get_parsed_config()));
			} else if(unit* u = luaW_tounit(L, i)) {
				return variant(new unit_callable(*u));
			}
			break;
	}
	return variant();
}

/**
 * Evaluates a formula in the formula engine.
 * - Arg 1: Formula string.
 * - Arg 2: optional context; can be a unit or a Lua table.
 * - Ret 1: Result of the formula.
 */
int lua_formula_bridge::intf_eval_formula(lua_State *L)
{
	using namespace game_logic;
	if(!lua_isstring(L, 1)) {
		luaL_typerror(L, 1, "string");
	}
	const formula form(lua_tostring(L, 1));
	boost::shared_ptr<formula_callable> context, fallback;
	if(unit* u = luaW_tounit(L, 2)) {
		context.reset(new unit_callable(*u));
	} else if(lua_istable(L, 2)) {
		context.reset(new lua_callable(L, 2));
	} else {
		context.reset(new map_formula_callable);
	}
	variant result = form.evaluate(*context);
	luaW_pushfaivariant(L, result);
	return 1;
}
