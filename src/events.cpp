/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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

#include "desktop/clipboard.hpp"
#include "cursor.hpp"
#include "events.hpp"
#include "log.hpp"
#include "sound.hpp"
#include "quit_confirmation.hpp"
#include "preferences.hpp"
#include "video.hpp"
#if defined _WIN32
#include "desktop/windows_tray_notification.hpp"
#endif

#include <SDL.h>

#include <algorithm>
#include <cassert>
#include <deque>
#include <utility>
#include <vector>
#include <algorithm>
#include <boost/thread.hpp>

#define ERR_GEN LOG_STREAM(err, lg::general)

namespace events
{

void context::add_handler(sdl_handler* ptr)
{
	handlers.push_back(ptr);
}

void context::delete_handler_index(size_t handler)
{
	if(focused_handler == static_cast<int>(handler)) {
		focused_handler = -1;
	} else if(focused_handler > static_cast<int>(handler)) {
		--focused_handler;
	}

	handlers.erase(handlers.begin()+handler);
}

bool context::remove_handler(sdl_handler* ptr)
{
	if(handlers.empty()) {
		return false;
	}

	static int depth = 0;
	++depth;

	//the handler is most likely on the back of the events array,
	//so look there first, otherwise do a complete search.
	if(handlers.back() == ptr) {
		delete_handler_index(handlers.size()-1);
	} else {
		const std::vector<sdl_handler*>::iterator i = std::find(handlers.begin(),handlers.end(),ptr);
		if(i != handlers.end()) {
			delete_handler_index(i - handlers.begin());
		} else {
			return false;
		}
	}

	--depth;

	if(depth == 0) {
		cycle_focus();
	} else {
		focused_handler = -1;
	}

	return true;
}

int context::cycle_focus()
{
	int index = focused_handler+1;
	for(size_t i = 0; i != handlers.size(); ++i) {
		if(size_t(index) == handlers.size()) {
			index = 0;
		}

		if(handlers[size_t(index)]->requires_event_focus()) {
			focused_handler = index;
			break;
		}
	}

	return focused_handler;
}

void context::set_focus(const sdl_handler* ptr)
{
	const std::vector<sdl_handler*>::const_iterator i = std::find(handlers.begin(),handlers.end(),ptr);
	if(i != handlers.end() && (**i).requires_event_focus()) {
		focused_handler = int(i - handlers.begin());
	}
}

//this object stores all the event handlers. It is a stack of event 'contexts'.
//a new event context is created when e.g. a modal dialog is opened, and then
//closed when that dialog is closed. Each context contains a list of the handlers
//in that context. The current context is the one on the top of the stack
std::deque<context> event_contexts;

//add a global context for event handlers. Whichever object has joined this will always
//receive all events, regardless of the current context.
context global_context;

std::vector<pump_monitor*> pump_monitors;

pump_monitor::pump_monitor() {
	pump_monitors.push_back(this);
}

pump_monitor::~pump_monitor() {
	pump_monitors.erase(
		std::remove(pump_monitors.begin(), pump_monitors.end(), this),
		pump_monitors.end());
}

event_context::event_context()
{
	event_contexts.push_back(context());
}

event_context::~event_context()
{
	assert(event_contexts.empty() == false);
	event_contexts.pop_back();
}

sdl_handler::sdl_handler(const bool auto_join) :
	has_joined_(false),
	has_joined_global_(false)
{

	if(auto_join) {
		assert(!event_contexts.empty());
		event_contexts.back().add_handler(this);
		has_joined_ = true;
	}
}

sdl_handler::~sdl_handler()
{
	if (has_joined_)
		leave();

	if (has_joined_global_)
		leave_global();

}

void sdl_handler::join() {
	join(event_contexts.back());
}

void sdl_handler::join(context &c)
{
	if(has_joined_) {
		leave(); // should not be in multiple event contexts
	}
	//join self
	c.add_handler(this);
	has_joined_ = true;

	//instruct members to join
	sdl_handler_vector members = handler_members();
	if(!members.empty()) {
		for(sdl_handler_vector::iterator i = members.begin(); i != members.end(); ++i) {
			(*i)->join(c);
		}
	}
}

void sdl_handler::join_same(sdl_handler* parent)
{
	if(has_joined_) {
		leave(); // should not be in multiple event contexts
	}

	for(std::deque<context>::reverse_iterator i = event_contexts.rbegin(); i != event_contexts.rend(); ++i) {
		std::vector<sdl_handler *> &handlers = (*i).handlers;
		if (std::find(handlers.begin(), handlers.end(), parent) != handlers.end()) {
			join(*i);
			return;
		}
	}
	join(global_context);

}

void sdl_handler::leave()
{
	sdl_handler_vector members = handler_members();
	if(!members.empty()) {
		for(sdl_handler_vector::iterator i = members.begin(); i != members.end(); ++i) {
			(*i)->leave();
		}
	} else {
		assert(event_contexts.empty() == false);
	}
	for(std::deque<context>::reverse_iterator i = event_contexts.rbegin(); i != event_contexts.rend(); ++i) {
		if(i->remove_handler(this)) {
			break;
		}
	}
	has_joined_ = false;
}

void sdl_handler::join_global()
{
	if(has_joined_global_) {
		leave_global(); // should not be in multiple event contexts
	}
	//join self
	global_context.add_handler(this);
	has_joined_global_ = true;

	//instruct members to join
	sdl_handler_vector members = handler_members();
	if(!members.empty()) {
		for(sdl_handler_vector::iterator i = members.begin(); i != members.end(); ++i) {
			(*i)->join_global();
		}
	}
}

void sdl_handler::leave_global()
{
	sdl_handler_vector members = handler_members();
	if(!members.empty()) {
		for(sdl_handler_vector::iterator i = members.begin(); i != members.end(); ++i) {
			(*i)->leave_global();
		}
	}

	global_context.remove_handler(this);

	has_joined_global_ = false;
}

void focus_handler(const sdl_handler* ptr)
{
	if(event_contexts.empty() == false) {
		event_contexts.back().set_focus(ptr);
	}
}

bool has_focus(const sdl_handler* hand, const SDL_Event* event)
{
	if(event_contexts.empty()) {
		return true;
	}

	if(hand->requires_event_focus(event) == false) {
		return true;
	}

	const int foc_i = event_contexts.back().focused_handler;

	//if no-one has focus at the moment, this handler obviously wants
	//focus, so give it to it.
	if(foc_i == -1) {
		focus_handler(hand);
		return true;
	}

	sdl_handler *const foc_hand = event_contexts.back().handlers[foc_i];
	if(foc_hand == hand){
		return true;
	} else if(!foc_hand->requires_event_focus(event)) {
		//if the currently focused handler doesn't need focus for this event
		//allow the most recent interested handler to take care of it
		int back_i = event_contexts.back().handlers.size() - 1;
		for(int i=back_i; i>=0; --i) {
			sdl_handler *const thief_hand = event_contexts.back().handlers[i];
			if(i != foc_i && thief_hand->requires_event_focus(event)) {
				//steal focus
				focus_handler(thief_hand);
				if(foc_i < back_i) {
					//position the previously focused handler to allow stealing back
					event_contexts.back().delete_handler_index(foc_i);
					event_contexts.back().add_handler(foc_hand);
				}
				return thief_hand == hand;
			}
		}
	}
	return false;
}


const Uint32 resize_timeout = 100;
SDL_Event last_resize_event;
bool last_resize_event_used = true;

static bool remove_on_resize(const SDL_Event &a) {
	if (a.type == DRAW_EVENT || a.type == DRAW_ALL_EVENT) {
		return true;
	}
	if (a.type == SHOW_HELPTIP_EVENT) {
		return true;
	}
	if ((a.type == SDL_WINDOWEVENT) &&
			(a.window.event == SDL_WINDOWEVENT_RESIZED ||
					a.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
					a.window.event == SDL_WINDOWEVENT_EXPOSED)) {
		return true;
	}

	return false;
}

// TODO: I'm uncertain if this is always safe to call at static init; maybe set in main() instead?
static const boost::thread::id main_thread = boost::this_thread::get_id();
void pump()
{
	if(boost::this_thread::get_id() != main_thread) {
		// Can only call this on the main thread!
		return;
	}
	SDL_PumpEvents();
	peek_for_resize();
	pump_info info;

	//used to keep track of double click events
	static int last_mouse_down = -1;
	static int last_click_x = -1, last_click_y = -1;

	SDL_Event temp_event;
	int poll_count = 0;
	int begin_ignoring = 0;
	std::vector< SDL_Event > events;
	while(SDL_PollEvent(&temp_event)) {
		++poll_count;
		peek_for_resize();

		if(!begin_ignoring && temp_event.type == SDL_WINDOWEVENT
				&& (temp_event.window.event == SDL_WINDOWEVENT_ENTER
						|| temp_event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED))
		{
			begin_ignoring = poll_count;
		} else if(begin_ignoring > 0 && is_input(temp_event)) {
			//ignore user input events that occurred after the window was activated
			continue;
		}
		events.push_back(temp_event);
	}

	std::vector<SDL_Event>::iterator ev_it = events.begin();
	for(int i=1; i < begin_ignoring; ++i){
		if(is_input(*ev_it)) {
			//ignore user input events that occurred before the window was activated
			ev_it = events.erase(ev_it);
		} else {
			++ev_it;
		}
	}

	std::vector<SDL_Event>::iterator ev_end = events.end();
	bool resize_found = false;
	for(ev_it = events.begin(); ev_it != ev_end; ++ev_it){
		SDL_Event &event = *ev_it;
		if (event.type == SDL_WINDOWEVENT &&
				event.window.event == SDL_WINDOWEVENT_RESIZED) {
			resize_found = true;
			last_resize_event = event;
			last_resize_event_used = false;

		}
	}
	// remove all inputs, draw events and only keep the last of the resize events
	// This will turn horrible after ~38 days when the Uint32 wraps.
	if (resize_found || SDL_GetTicks() <= last_resize_event.window.timestamp + resize_timeout) {
		events.erase(std::remove_if(events.begin(), events.end(), remove_on_resize), events.end());
	} else if(SDL_GetTicks() > last_resize_event.window.timestamp + resize_timeout && !last_resize_event_used) {
		events.insert(events.begin(), last_resize_event);
		last_resize_event_used = true;
	}

	ev_end = events.end();

	for(ev_it = events.begin(); ev_it != ev_end; ++ev_it){
		SDL_Event &event = *ev_it;
		switch(event.type) {

			case SDL_WINDOWEVENT:
				switch(event.window.event) {
					case SDL_WINDOWEVENT_ENTER:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						cursor::set_focus(1);
						break;

					case SDL_WINDOWEVENT_LEAVE:
					case SDL_WINDOWEVENT_FOCUS_LOST:
						cursor::set_focus(1);
						break;

					case SDL_WINDOWEVENT_RESIZED:
						info.resize_dimensions.first = event.window.data1;
						info.resize_dimensions.second = event.window.data2;
						break;
				}
				//make sure this runs in it's own scope.
				{
					for( std::deque<context>::iterator i = event_contexts.begin() ; i != event_contexts.end(); ++i) {
						const std::vector<sdl_handler*>& event_handlers = (*i).handlers;
						for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
							event_handlers[i1]->handle_window_event(event);
						}
					}
					const std::vector<sdl_handler*>& event_handlers = global_context.handlers;
					for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
						event_handlers[i1]->handle_window_event(event);
					}
				}

				//This event was just distributed, don't re-distribute.
				continue;

			case SDL_MOUSEMOTION: {
				//always make sure a cursor is displayed if the
				//mouse moves or if the user clicks
				cursor::set_focus(true);
				raise_help_string_event(event.motion.x,event.motion.y);
				break;
			}

			case SDL_MOUSEBUTTONDOWN: {
				//always make sure a cursor is displayed if the
				//mouse moves or if the user clicks
				cursor::set_focus(true);
				if(event.button.button == SDL_BUTTON_LEFT) {
					static const int DoubleClickTime = 500;
					static const int DoubleClickMaxMove = 3;
					if(last_mouse_down >= 0 && info.ticks() - last_mouse_down < DoubleClickTime &&
					   abs(event.button.x - last_click_x) < DoubleClickMaxMove &&
					   abs(event.button.y - last_click_y) < DoubleClickMaxMove) {
						SDL_UserEvent user_event;
						user_event.type = DOUBLE_CLICK_EVENT;
						user_event.code = 0;
						user_event.data1 = reinterpret_cast<void*>(event.button.x);
						user_event.data2 = reinterpret_cast<void*>(event.button.y);
						::SDL_PushEvent(reinterpret_cast<SDL_Event*>(&user_event));
					}
					last_mouse_down = info.ticks();
					last_click_x = event.button.x;
					last_click_y = event.button.y;
				}
				break;
			}
			case DRAW_ALL_EVENT:
			{
				CVideo::get_singleton().lock_flips(true);
				/* iterate backwards as the most recent things will be at the top */
				for( std::deque<context>::iterator i = event_contexts.begin() ; i != event_contexts.end(); ++i) {
					const std::vector<sdl_handler*>& event_handlers = (*i).handlers;
					for( std::vector<sdl_handler*>::const_iterator i1 = event_handlers.begin(); i1 != event_handlers.end(); ++i1) {
						(*i1)->handle_event(event);
					}
				}
				CVideo::get_singleton().lock_flips(false);
				//CVideo::get_singleton().flip();
				continue; //do not do further handling here
			}

#ifndef __APPLE__
			case SDL_KEYDOWN: {
				if(event.key.keysym.sym == SDLK_F4 && (event.key.keysym.mod == KMOD_RALT || event.key.keysym.mod == KMOD_LALT)) {
					quit_confirmation::quit_to_desktop();
					continue; // this event is already handled
				}
				break;
			}
#endif

#if defined(_X11) && !defined(__APPLE__)
			case SDL_SYSWMEVENT: {
				//clipboard support for X11
				desktop::clipboard::handle_system_event(event);
				break;
			}
#endif

#if defined _WIN32
			case SDL_SYSWMEVENT: {
				windows_tray_notification::handle_system_event(event);
				break;
			}
#endif

			case SDL_QUIT: {
				quit_confirmation::quit_to_desktop();
				continue; //this event is already handled.
			}
		}

		const std::vector<sdl_handler*>& event_handlers = global_context.handlers;
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->handle_event(event);
		}

		if(event_contexts.empty() == false) {

			const std::vector<sdl_handler*>& event_handlers = event_contexts.back().handlers;

			//events may cause more event handlers to be added and/or removed,
			//so we must use indexes instead of iterators here.
			for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
				event_handlers[i1]->handle_event(event);
			}
		}

	}

	//inform the pump monitors that an events::pump() has occurred
	for(size_t i1 = 0, i2 = pump_monitors.size(); i1 != i2 && i1 < pump_monitors.size(); ++i1) {
		pump_monitors[i1]->process(info);
	}
}

void raise_process_event()
{
	if(event_contexts.empty() == false) {

		const std::vector<sdl_handler*>& event_handlers = event_contexts.back().handlers;

		//events may cause more event handlers to be added and/or removed,
		//so we must use indexes instead of iterators here.
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->process_event();
		}
	}
}

void raise_resize_event()
{
	SDL_Event event;
	event.window.type = SDL_WINDOWEVENT;
	event.window.event = SDL_WINDOWEVENT_RESIZED;
	event.window.windowID = 0; // We don't check this anyway... I think...
	event.window.data1 = CVideo::get_singleton().getx();
	event.window.data2 = CVideo::get_singleton().gety();
	
	SDL_PushEvent(&event);
}

void raise_draw_event()
{
	if(event_contexts.empty() == false) {

		const std::vector<sdl_handler*>& event_handlers = event_contexts.back().handlers;

		//events may cause more event handlers to be added and/or removed,
		//so we must use indexes instead of iterators here.
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->draw();
		}
	}
}

void raise_draw_all_event()
{
	for( std::deque<context>::iterator i = event_contexts.begin() ; i != event_contexts.end(); ++i) {
		const std::vector<sdl_handler*>& event_handlers = (*i).handlers;
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->draw();
		}
	}
}

void raise_volatile_draw_event()
{
	if(event_contexts.empty() == false) {

		const std::vector<sdl_handler*>& event_handlers = event_contexts.back().handlers;

		//events may cause more event handlers to be added and/or removed,
		//so we must use indexes instead of iterators here.
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->volatile_draw();
		}
	}
}

void raise_volatile_draw_all_event()
{
	for( std::deque<context>::iterator i = event_contexts.begin() ; i != event_contexts.end(); ++i) {
		const std::vector<sdl_handler*>& event_handlers = (*i).handlers;
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->volatile_draw();
		}
	}
}

void raise_volatile_undraw_event()
{
	if(event_contexts.empty() == false) {

		const std::vector<sdl_handler*>& event_handlers = event_contexts.back().handlers;

		//events may cause more event handlers to be added and/or removed,
		//so we must use indexes instead of iterators here.
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->volatile_undraw();
		}
	}
}

void raise_help_string_event(int mousex, int mousey)
{
	if(event_contexts.empty() == false) {

		const std::vector<sdl_handler*>& event_handlers = event_contexts.back().handlers;

		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->process_help_string(mousex,mousey);
			event_handlers[i1]->process_tooltip_string(mousex,mousey);
		}
	}
}

int pump_info::ticks(unsigned *refresh_counter, unsigned refresh_rate) {
	if(!ticks_ && !(refresh_counter && ++*refresh_counter % refresh_rate)) {
		ticks_ = ::SDL_GetTicks();
	}
	return ticks_;
}


/* The constants for the minimum and maximum are picked from the headers. */
#define INPUT_MIN 0x300
#define INPUT_MAX 0x8FF

bool is_input(const SDL_Event& event)
{
	return event.type >= INPUT_MIN && event.type <= INPUT_MAX;
}

void discard_input()
{
	SDL_FlushEvents(INPUT_MIN, INPUT_MAX);
}

void peek_for_resize()
{
	SDL_Event events[100];
	int num = SDL_PeepEvents(events, 100, SDL_PEEKEVENT, SDL_WINDOWEVENT, SDL_WINDOWEVENT);
	for (int i = 0; i < num; ++i) {
		if (events[i].type == SDL_WINDOWEVENT &&
				events[i].window.event == SDL_WINDOWEVENT_RESIZED) {
			CVideo::get_singleton().update_framebuffer();

		}
	}
}


} //end events namespace

