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

/**
 * @file
 * Provides support to subscribe to and to trigger events.
 * This class doesn't need a base event class to store the events
 * as they are statically given via templates.
 *
 * Step to use this class:
 *
 * \code{.cpp}
// Define two events (tag-like structure).

struct transfer_is_complete{};
struct transfer_error{};

// Specialize the event_slot structure. The slot is a function called when
// an event is triggered.

template <>
struct event_slot<transfer_complete>
{
	typedef void type();
};

template <>
struct event_slot<transfer_error>
{
	typedef void type(const boost::system::error_code&);
};

// Create the event class, a simple typedef on events that
// gather all event previously declared with boost::mpl::set.

struct transfer_events
: events<boost::mpl::set<
				transfer_complete
			, transfer_error> >
{};

// Finally you can use this class wherever you need this particular set of
// events.

class network_communicator
{
public:
	// This method is just delegating event registration to the class event.
	template <class Event, class F>
	boost::signals2::connection on_event(F f)
	{
	  events_.on_event<Event>(f);
	}

private:
	// Typical Boost.Asio handler, we signal if something goes wrong or if the transfer is completed.
	// Thus we delegate the responsability of handling these events by others classes and makes this class
	// highly re-usable (because there is no buiseness logic implemented in).
	void network_communicator::on_completion(const boost::system::error_code& error,
		std::size_t bytes_in_buffer)
	{
		if(error)
		{
			events_.signal_event<transfer_error>(error);
		}
		else
		{
			events_.signal_event<transfer_complete>();
		}
	}

	// The event class previously defined.
	transfer_events events_;
};

 * \endcode
*/

#ifndef UMCD_EVENTS_HPP
#define UMCD_EVENTS_HPP

/** The maximum arguments number we can pass to the event slot.
* Increase this if the slots take more than EVENT_LIMIT_ARG arguments.
*/
#ifndef EVENT_LIMIT_ARG
	#define EVENT_LIMIT_ARG 5
#endif

#include "umcd/server/detail/events_type_selector.hpp"

/** Provides slot registration (with on_event) and event triggering (with signal_event)
* for each events in the EventSequence.
*
* @see events_set_impl for a more detailled description of the methods.
*/
template <class EventsType>
class events
: public detail::events_type_selector<EventsType>
{
};

#endif // UMCD_EVENTS_HPP
