#pragma once

#include "scripting/lua_common.hpp"

#include <boost/type_traits.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/has_xxx.hpp>
#include "tstring.hpp"
#include "lua/lauxlib.h"
#include "lua/lua.h"

#include <cassert>

class enum_tag;

namespace lua_check_impl
{
	namespace detail
	{
		BOOST_MPL_HAS_XXX_TRAIT_DEF(value_type)
		BOOST_MPL_HAS_XXX_TRAIT_DEF(iterator)
		BOOST_MPL_HAS_XXX_TRAIT_DEF(size_type)
		BOOST_MPL_HAS_XXX_TRAIT_DEF(reference)
		BOOST_MPL_HAS_XXX_TRAIT_DEF(key_type)
		BOOST_MPL_HAS_XXX_TRAIT_DEF(mapped_type)
	}
	
	template <typename T, typename Enable = void>
	struct is_container
		: boost::mpl::bool_<
			detail::has_value_type<T>::value &&
			detail::has_iterator<T>::value &&
			detail::has_size_type<T>::value &&
			detail::has_reference<T>::value
		>
	{};

	template <typename T, typename Enable = void>
	struct is_map
		: boost::mpl::bool_<
			detail::has_key_type<T>::value &&
			detail::has_mapped_type<T>::value
		>
	{};
	
	template <typename T>
	struct is_container<T&>
		: is_container<T>
	{};

	template <typename T>
	struct is_map<T&>
		: is_map<T>
	{};

	template<typename T>
	struct remove_constref
	{
		typedef typename boost::remove_const<typename boost::remove_reference<typename boost::remove_const<T>::type>::type>::type type;
	};

	//std::string
	template<typename T>
	typename boost::enable_if<typename boost::is_same<T, std::string>::type, std::string>::type lua_check(lua_State *L, int n)
	{
		return luaL_checkstring(L, n);
	}
	template<typename T>
	typename boost::enable_if<typename boost::is_same<T, std::string>::type, void>::type lua_push(lua_State *L, const std::string& val)
	{
		lua_pushlstring(L, val.c_str(), val.size());
	}

	//config
	template<typename T>
	typename boost::enable_if<typename boost::is_same<T, config>::type, config>::type lua_check(lua_State *L, int n)
	{
		return luaW_checkconfig(L, n);
	}
	template<typename T>
	typename boost::enable_if<typename boost::is_same<T, config>::type, void>::type lua_push(lua_State *L, const config& val)
	{
		luaW_pushconfig(L, val);
	}

	//enums generated by MAKE_ENUM
	template<typename T>
	typename boost::enable_if<boost::is_base_of<enum_tag, T>, T>::type lua_check(lua_State *L, int n)
	{
		T val;
		std::string str = lua_check_impl::lua_check<std::string>(L, n);
		if(val.parse(str))
		{
			return val;
		}
		else
		{
			luaL_argerror(L, n, ("cannot convert " + str + " to enum " + T::name()).c_str());
		}
	}
	template<typename T>
	typename boost::enable_if<boost::is_base_of<enum_tag, T>, void>::type lua_push(lua_State *L, T val)
	{
		lua_check_impl::lua_push(L, val.to_string());
	}

	//t_string
	template<typename T>
	typename boost::enable_if<typename boost::is_same<T, t_string>::type, t_string>::type lua_check(lua_State *L, int n)
	{
		return luaW_checktstring(L, n);
	}
	template<typename T>
	typename boost::enable_if<typename boost::is_same<T, t_string>::type, void>::type lua_push(lua_State *L, const t_string& val)
	{
		luaW_pushtstring(L, val);
	}

	//bool
	template<typename T>
	typename boost::enable_if<typename boost::is_same<T, bool>::type, bool>::type lua_check(lua_State *L, int n)
	{
		return luaW_toboolean(L, n);
	}
	template<typename T>
	typename boost::enable_if<typename boost::is_same<T, bool>::type, void>::type lua_push(lua_State *L, bool val)
	{
		lua_pushboolean(L, val);
	}

	//double, float
	template<typename T>
	typename boost::enable_if<boost::is_float<T>, T>::type lua_check(lua_State *L, int n)
	{
		return luaL_checknumber(L, n);
	}
	template<typename T>
	typename boost::enable_if<boost::is_float<T>, void>::type lua_push(lua_State *L, T val)
	{
		lua_pushnumber(L, val);
	}

	//integer types
	template<typename T>
	typename boost::enable_if<
		boost::mpl::and_<
			boost::is_integral<T>,
			boost::mpl::not_<typename boost::is_same<T, bool>::type>
		>,
		T
	>::type lua_check(lua_State *L, int n)
	{
		return luaL_checkinteger(L, n);
	}

	template<typename T>
	typename boost::enable_if<
		boost::mpl::and_<
			boost::is_integral<T>,
			boost::mpl::not_<typename boost::is_same<T, bool>::type>
		>,
		void
	>::type lua_push(lua_State *L, T val)
	{
		lua_pushnumber(L, val);
	}
	
	//std::vector and similar but not std::string
	template<typename T>
	typename boost::enable_if<
		typename boost::mpl::and_<
			typename is_container<T>::type,
			typename boost::mpl::not_<typename boost::is_same<T, std::string> >::type
		>::type,
		T
	>::type
	lua_check(lua_State * L, int n)
	{
		if (lua_istable(L, n))
		{
			T res;
			for (int i = 1, i_end = lua_rawlen(L, n); i <= i_end; ++i)
			{
				lua_rawgeti(L, n, i);
				res.push_back(lua_check_impl::lua_check<typename remove_constref<typename T::value_type>::type>(L, -1));
			}
			return res;
		}
		else
		{
			luaL_argerror(L, n, "Table expected");
			throw "luaL_argerror returned"; //shouldnt happen, luaL_argerror always throws.
		}
	}
	
	//also accepts things like std::vector<int>() | boost::adaptors::transformed(..)
	template<typename T>
	typename boost::enable_if<
		typename boost::mpl::and_<
			typename is_container<T>::type,
			typename boost::mpl::not_<typename boost::is_same<T, std::string> >::type
		>::type,
		void
	>::type
	lua_push(lua_State * L, const T& list )
	{
		// NOTE: T might be some boost::iterator_range type where size might be < 0. (unfortunaltely in this case size() does not return T::size_type)
		assert(list.size() >= 0);
		lua_createtable(L, list.size(), 0);
		for(size_t i = 0, size = static_cast<size_t>(list.size()); i < size; ++i) {
			lua_check_impl::lua_push<typename remove_constref<typename T::value_type>::type>(L, list[i]);
			lua_rawseti(L, -2, i + 1);
		}
	}

}

template<typename T>
typename lua_check_impl::remove_constref<T>::type lua_check(lua_State *L, int n)
{
	//remove possible const& to make life easier for the impl namespace.
	return lua_check_impl::lua_check<typename lua_check_impl::remove_constref<T>::type>(L, n);
}

template<typename T>
void lua_push(lua_State *L, const T& val)
{
	return lua_check_impl::lua_push<typename lua_check_impl::remove_constref<T>::type>(L, val);
}
