/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@hyc.io>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifndef UMCD_EVENTS_SET_IMPL_HPP
#define UMCD_EVENTS_SET_IMPL_HPP

#include <boost/mpl/set.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/erase.hpp>
#include <boost/mpl/begin_end.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/facilities/intercept.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <boost/signals2.hpp>

#include "umcd/server/traits/event_slot.hpp"
#include "umcd/boost/static_assert.hpp"

namespace detail{
namespace mpl = boost::mpl;

/** Define a type that is the Sequence type list privates from its first type.
* @note For some reasons it's not available in Boost.MPL (no implementation defined...).
*/
template <class Sequence>
struct pop_front
{
	typedef typename mpl::erase<
			Sequence
		, typename mpl::begin<Sequence>::type
	>::type type;
};

/** Store the signal/slot mechanism of the type in front of EventSequence.
 * Recursively store itself (events_set_impl) until the EventSequence is empty (see specialization).
*/
template <class EventSequence, bool empty = mpl::empty<EventSequence>::value>
class events_set_impl
{
	typedef EventSequence events_type;
	typedef typename mpl::front<events_type>::type event_type;
	typedef typename event_slot<event_type>::type event_slot_type;

public:
	/** The event we want to add a slot function has been found so we can connect it.
	* @param slot_function The slot function we want to call when Event will be triggered.
	* @return boost::signals2::connection of the connected slot, you can use it to deconnect the slot.
	*/
	template <class Event>
	typename boost::enable_if<
	    boost::is_same<Event, event_type>
		, boost::signals2::connection
	>::type on_event(boost::function<event_slot_type> slot_function)
	{
		return signal_.connect(slot_function);
	}

	/** The current event is not the same as the one we want to connect the function.
	* Recursively calls itself until an event match.
	*/
	template <class Event>
	typename boost::disable_if<
			boost::is_same<Event, event_type>
		, boost::signals2::connection
	>::type on_event(boost::function<typename event_slot<Event>::type> slot_function)
	{
		return events_tail_.template on_event<Event>(slot_function);
	}

/** Boilerplate macros that help to select the nth arguments of the function slot and 
* thus making the correct argument type.
*/
#define MAKE_ARG_PARAM_TYPE_IMPL(count) typename boost::function_traits<typename event_slot<Event>::type>::arg##count##_type
#define MAKE_ARG_PARAM_TYPE(count) MAKE_ARG_PARAM_TYPE_IMPL(count)

#define MAKE_EVENT_ARG(z, count, unused) 		\
	BOOST_PP_COMMA_IF(count)									\
	MAKE_ARG_PARAM_TYPE(BOOST_PP_INC(count)) 	\
	arg##count

/** signal_event is used to signal that an event occurs. In the same fashion than on_event, 
* it recursively calls itself until the good event is found.
*
* Moreover, we need to create some functions overload that will takes exactly the number of
* arguments needed by the slot function. BOOST_PP_REPEAT expands the SIGNAL_EVENT to
* EVENT_LIMIT_ARG so we have N overloads of this function.
* We also need to horizontally expand the argument and that's why we use the MAKE_EVENT_ARG
* with a second BOOST_PP_REPEAT macro.
*/
#define SIGNAL_EVENT(z, n, unused) 												\
	template <class Event>																	\
	typename boost::enable_if<															\
	    boost::is_same<Event, event_type>										\
		, void																								\
	>::type signal_event(BOOST_PP_REPEAT(n, MAKE_EVENT_ARG,~)) \
	{																												\
		signal_(BOOST_PP_ENUM_PARAMS(n, arg));								\
	}																												\
																													\
	template <class Event>																	\
	typename boost::disable_if<															\
			boost::is_same<Event, event_type>										\
		, void																								\
	>::type signal_event(BOOST_PP_REPEAT(n, MAKE_EVENT_ARG,~)) \
	{																												\
		events_tail_.template signal_event<Event>(BOOST_PP_ENUM_PARAMS(n, arg)); \
	}

BOOST_PP_REPEAT(EVENT_LIMIT_ARG, SIGNAL_EVENT, ~)

#undef SIGNAL_EVENT
#undef MAKE_EVENT_ARG
#undef MAKE_ARG_PARAM_TYPE
#undef MAKE_ARG_PARAM_TYPE_IMPL

private:
	/** The signal that will be triggered if the current event occurs.
	*/
	boost::signals2::signal<event_slot_type> signal_;

	/** The tail of the events type list. We remove the front (which is actually
	* handled by this class) and we recursively delegate storage to the next instances
	* of this class.
	*/
	typedef typename pop_front<events_type>::type events_tail_type;
	events_set_impl<events_tail_type> events_tail_;
};

/** Denote the end of the typelist, if on_event is called here, it means we didn't found the event
* we try to access.
*/
template <class EventSequence>
class events_set_impl<EventSequence, true>
{
public:
	template <class Event>
	boost::signals2::connection on_event(...)
	{
		BOOST_STATIC_ASSERT_MSG(sizeof(Event) == 0, "** You are trying to record a slot for an unknown event. **");
		return boost::signals2::connection();
	}

	template <class Event>
	void signal_event(...)
	{
		BOOST_STATIC_ASSERT_MSG(sizeof(Event) == 0, "** You are trying to signal an event that is not registered. **");
	}
};

/** Class checking that the sequence meets is a mpl::set.
* We use a set because it doesn't make sense to add twice an event.
*/
template <class EventSequence, class sequence_tag>
class events_set_impl_tag
{
	BOOST_STATIC_ASSERT_MSG(sizeof(EventSequence) == 0, "** The sequence must be a boost::mpl::set. **");
};

template <class EventSequence>
class events_set_impl_tag<EventSequence, mpl::aux::set_tag>
: public events_set_impl<EventSequence>
{};

} // namespace detail

#endif // UMCD_EVENTS_SET_IMPL_HPP
