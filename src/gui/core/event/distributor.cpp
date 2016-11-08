/*
   Copyright (C) 2009 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/core/event/distributor.hpp"

#include "events.hpp"
#include "gui/core/log.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"

#include "utils/functional.hpp"

namespace gui2
{

namespace event
{

/**
 * SDL_AddTimer() callback for the hover event.
 *
 * When this callback is called it pushes a new hover event in the event queue.
 *
 * @param interval                The time parameter of SDL_AddTimer.
 * @param param                   Pointer to a widget that's able to show the
 *                                tooltip (will be used as a dispatcher).
 *
 * @returns                       The new timer interval, 0 to stop.
 */

#if 0
/**
 * SDL_AddTimer() callback for the popup event.
 *
 * This event makes sure the popup is removed again.
 *
 * @param interval                The time parameter of SDL_AddTimer.
 * @param param                   Pointer to parameter structure.
 *
 * @returns                       The new timer interval, 0 to stop.
 */
static Uint32 popup_callback(Uint32 /*interval*/, void* /*param*/)
{
	DBG_GUI_E << "Pushing popup removal event in queue.\n";

	SDL_Event event;
	SDL_UserEvent data;

	data.type = HOVER_REMOVE_POPUP_EVENT;
	data.code = 0;
	data.data1 = 0;
	data.data2 = 0;

	event.type = HOVER_REMOVE_POPUP_EVENT;
	event.user = data;

	SDL_PushEvent(&event);
	return 0;
}
#endif

/**
 * Small helper to keep a resource (boolean) locked.
 *
 * Some of the event handling routines can't be called recursively, this due to
 * the fact that they are attached to the pre queue and when the forward an
 * event the pre queue event gets triggered recursively causing infinite
 * recursion.
 *
 * To prevent that those functions check the lock and exit when the lock is
 * held otherwise grab the lock here.
 */
class locker
{
public:
	locker(bool& locked) : locked_(locked)
	{
		assert(!locked_);
		locked_ = true;
	}

	~locker()
	{
		assert(locked_);
		locked_ = false;
	}

private:
	bool& locked_;
};


/***** ***** ***** ***** mouse_motion ***** ***** ***** ***** *****/

#define LOG_HEADER "distributor mouse motion [" << owner_.id() << "]: "

mouse_motion::mouse_motion(widget& owner,
							 const dispatcher::queue_position queue_position)
	: mouse_focus_(nullptr)
	, mouse_captured_(false)
	, owner_(owner)
	, hover_timer_(0)
	, hover_widget_(nullptr)
	, hover_position_(0, 0)
	, hover_shown_(true)
	, signal_handler_sdl_mouse_motion_entered_(false)
{
	owner.connect_signal<event::SDL_MOUSE_MOTION>(
			std::bind(&mouse_motion::signal_handler_sdl_mouse_motion,
						this,
						_2,
						_3,
						_5),
			queue_position);

	owner_.connect_signal<event::SDL_WHEEL_UP>(std::bind(
			&mouse_motion::signal_handler_sdl_wheel, this, _2, _3, _5));
	owner_.connect_signal<event::SDL_WHEEL_DOWN>(std::bind(
			&mouse_motion::signal_handler_sdl_wheel, this, _2, _3, _5));
	owner_.connect_signal<event::SDL_WHEEL_LEFT>(std::bind(
			&mouse_motion::signal_handler_sdl_wheel, this, _2, _3, _5));
	owner_.connect_signal<event::SDL_WHEEL_RIGHT>(std::bind(
			&mouse_motion::signal_handler_sdl_wheel, this, _2, _3, _5));

	owner.connect_signal<event::SHOW_HELPTIP>(
			std::bind(&mouse_motion::signal_handler_show_helptip,
						this,
						_2,
						_3,
						_5),
			queue_position);
}

mouse_motion::~mouse_motion()
{
	stop_hover_timer();
}

void mouse_motion::capture_mouse(const bool capture)
{
	assert(mouse_focus_);
	mouse_captured_ = capture;
}

void mouse_motion::signal_handler_sdl_mouse_motion(const event::event_t event,
													bool& handled,
													const point& coordinate)
{
	if(signal_handler_sdl_mouse_motion_entered_) {
		return;
	}
	locker lock(signal_handler_sdl_mouse_motion_entered_);

	DBG_GUI_E << LOG_HEADER << event << ".\n";

	if(mouse_captured_) {
		assert(mouse_focus_);
		if(!owner_.fire(event, *mouse_focus_, coordinate)) {
			mouse_hover(mouse_focus_, coordinate);
		}
	} else {
		widget* mouse_over = owner_.find_at(coordinate, true);
		while(mouse_over && !mouse_over->can_mouse_focus() && mouse_over->parent()) {
			mouse_over = mouse_over->parent();
		}
		if(mouse_over) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << event << ".\n";
			if(owner_.fire(event, *mouse_over, coordinate)) {
				return;
			}
		}

		if(!mouse_focus_ && mouse_over) {
			mouse_enter(mouse_over);
		} else if(mouse_focus_ && !mouse_over) {
			mouse_leave();
		} else if(mouse_focus_ && mouse_focus_ == mouse_over) {
			mouse_hover(mouse_over, coordinate);
		} else if(mouse_focus_ && mouse_over) {
			// moved from one widget to the next
			mouse_leave();
			mouse_enter(mouse_over);
		} else {
			assert(!mouse_focus_ && !mouse_over);
		}
	}
	handled = true;
}

void mouse_motion::signal_handler_sdl_wheel(const event::event_t event,
											 bool& handled,
											 const point& coordinate)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	if(mouse_captured_) {
		assert(mouse_focus_);
		owner_.fire(event, *mouse_focus_, coordinate);
	} else {
		widget* mouse_over = owner_.find_at(coordinate, true);
		if(mouse_over) {
			owner_.fire(event, *mouse_over, coordinate);
		}
	}
	handled = true;
}

void mouse_motion::signal_handler_show_helptip(const event::event_t event,
												bool& handled,
												const point& coordinate)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	if(mouse_captured_) {
		assert(mouse_focus_);
		if(owner_.fire(event, *mouse_focus_, coordinate)) {
			stop_hover_timer();
		}
	} else {
		widget* mouse_over = owner_.find_at(coordinate, true);
		if(mouse_over) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << event << ".\n";
			if(owner_.fire(event, *mouse_over, coordinate)) {
				stop_hover_timer();
			}
		}
	}

	handled = true;
}

void mouse_motion::mouse_enter(widget* mouse_over)
{
	DBG_GUI_E << LOG_HEADER << "Firing: " << event::MOUSE_ENTER << ".\n";

	assert(mouse_over);

	mouse_focus_ = mouse_over;
	owner_.fire(event::MOUSE_ENTER, *mouse_over);

	hover_shown_ = false;
	start_hover_timer(mouse_over, get_mouse_position());
}

void mouse_motion::mouse_hover(widget* mouse_over, const point& coordinate)
{
	DBG_GUI_E << LOG_HEADER << "Firing: " << event::MOUSE_MOTION << ".\n";

	assert(mouse_over);

	owner_.fire(event::MOUSE_MOTION, *mouse_over, coordinate);

	if(hover_timer_) {
		if((std::abs(hover_position_.x - coordinate.x) > 5)
		   || (std::abs(hover_position_.y - coordinate.y) > 5)) {

			stop_hover_timer();
			start_hover_timer(mouse_over, coordinate);
		}
	}
}

void mouse_motion::show_tooltip()
{
	DBG_GUI_E << LOG_HEADER << "Firing: " << event::SHOW_TOOLTIP << ".\n";

	if(!hover_widget_) {
		// See mouse_motion::stop_hover_timer.
		ERR_GUI_E << LOG_HEADER << event::SHOW_TOOLTIP
				  << " bailing out, no hover widget.\n";
		return;
	}

	/*
	 * Ignore the result of the event, always mark the tooltip as shown. If
	 * there was no handler, there is no reason to assume there will be one
	 * next time.
	 */
	owner_.fire(SHOW_TOOLTIP, *hover_widget_, hover_position_);

	hover_shown_ = true;

	hover_timer_ = 0;
	hover_widget_ = nullptr;
	hover_position_ = point();
}

void mouse_motion::mouse_leave()
{
	DBG_GUI_E << LOG_HEADER << "Firing: " << event::MOUSE_LEAVE << ".\n";

	control* ctrl = dynamic_cast<control*>(mouse_focus_);
	if(!ctrl || ctrl->get_active()) {
		owner_.fire(event::MOUSE_LEAVE, *mouse_focus_);
	}

	owner_.fire(NOTIFY_REMOVE_TOOLTIP, *mouse_focus_, nullptr);

	mouse_focus_ = nullptr;

	stop_hover_timer();
}

void mouse_motion::start_hover_timer(widget* widget, const point& coordinate)
{
	assert(widget);
	stop_hover_timer();

	if(hover_shown_ || !widget->wants_mouse_hover()) {
		return;
	}

	DBG_GUI_E << LOG_HEADER << "Start hover timer for widget '" << widget->id()
			  << "' at address " << widget << ".\n";

	hover_timer_
			= add_timer(50, std::bind(&mouse_motion::show_tooltip, this));

	if(hover_timer_) {
		hover_widget_ = widget;
		hover_position_ = coordinate;
	} else {
		ERR_GUI_E << LOG_HEADER << "Failed to add hover timer." << std::endl;
	}
}

void mouse_motion::stop_hover_timer()
{
	if(hover_timer_) {
		assert(hover_widget_);
		DBG_GUI_E << LOG_HEADER << "Stop hover timer for widget '"
				  << hover_widget_->id() << "' at address " << hover_widget_
				  << ".\n";

		if(!remove_timer(hover_timer_)) {
			ERR_GUI_E << LOG_HEADER << "Failed to remove hover timer."
					  << std::endl;
		}

		hover_timer_ = 0;
		hover_widget_ = nullptr;
		hover_position_ = point();
	}
}

/***** ***** ***** ***** mouse_button ***** ***** ***** ***** *****/

#undef LOG_HEADER
#define LOG_HEADER                                                             \
	"distributor mouse button " << name_ << " [" << owner_.id() << "]: "

template <event_t sdl_button_down,
		  event_t sdl_button_up,
		  event_t button_down,
		  event_t button_up,
		  event_t button_click,
		  event_t button_double_click>
mouse_button<sdl_button_down,
			  sdl_button_up,
			  button_down,
			  button_up,
			  button_click,
			  button_double_click>::mouse_button(const std::string& name_,
												  widget& owner,
												  const dispatcher::queue_position
														  queue_position)
	: mouse_motion(owner, queue_position)
	, last_click_stamp_(0)
	, last_clicked_widget_(nullptr)
	, focus_(0)
	, name_(name_)
	, is_down_(false)
	, signal_handler_sdl_button_down_entered_(false)
	, signal_handler_sdl_button_up_entered_(false)
{
	owner_.connect_signal<sdl_button_down>(
			std::bind(&mouse_button<sdl_button_down,
									   sdl_button_up,
									   button_down,
									   button_up,
									   button_click,
									   button_double_click>::
								 signal_handler_sdl_button_down,
						this,
						_2,
						_3,
						_5),
			queue_position);
	owner_.connect_signal<sdl_button_up>(
			std::bind(&mouse_button<sdl_button_down,
									   sdl_button_up,
									   button_down,
									   button_up,
									   button_click,
									   button_double_click>::
								 signal_handler_sdl_button_up,
						this,
						_2,
						_3,
						_5),
			queue_position);
}

template <event_t sdl_button_down,
		  event_t sdl_button_up,
		  event_t button_down,
		  event_t button_up,
		  event_t button_click,
		  event_t button_double_click>
void mouse_button<sdl_button_down,
				   sdl_button_up,
				   button_down,
				   button_up,
				   button_click,
				   button_double_click>::initialize_state(const bool is_down)
{
	last_click_stamp_ = 0;
	last_clicked_widget_ = nullptr;
	focus_ = 0;
	is_down_ = is_down;
}

template <event_t sdl_button_down,
		  event_t sdl_button_up,
		  event_t button_down,
		  event_t button_up,
		  event_t button_click,
		  event_t button_double_click>
void mouse_button<sdl_button_down,
				   sdl_button_up,
				   button_down,
				   button_up,
				   button_click,
				   button_double_click>::
		signal_handler_sdl_button_down(const event::event_t event,
									   bool& handled,
									   const point& coordinate)
{
	if(signal_handler_sdl_button_down_entered_) {
		return;
	}
	locker lock(signal_handler_sdl_button_down_entered_);

	DBG_GUI_E << LOG_HEADER << event << ".\n";

	if(is_down_) {
#ifdef GUI2_SHOW_UNHANDLED_EVENT_WARNINGS
		WRN_GUI_E << LOG_HEADER << event
				  << ". The mouse button is already down, "
				  << "we missed an event.\n";
#endif
		return;
	}
	is_down_ = true;

	if(mouse_captured_) {
		assert(mouse_focus_);
		focus_ = mouse_focus_;
		DBG_GUI_E << LOG_HEADER << "Firing: " << sdl_button_down << ".\n";
		if(!owner_.fire(sdl_button_down, *focus_, coordinate)) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << button_down << ".\n";
			owner_.fire(button_down, *mouse_focus_);
		}
	} else {
		widget* mouse_over = owner_.find_at(coordinate, true);
		if(!mouse_over) {
			return;
		}

		if(mouse_over != mouse_focus_) {
#ifdef GUI2_SHOW_UNHANDLED_EVENT_WARNINGS
			WRN_GUI_E << LOG_HEADER << ". Mouse down on non focused widget "
					  << "and mouse not captured, we missed events.\n";
#endif
			mouse_focus_ = mouse_over;
		}

		focus_ = mouse_over;
		DBG_GUI_E << LOG_HEADER << "Firing: " << sdl_button_down << ".\n";
		if(!owner_.fire(sdl_button_down, *focus_, coordinate)) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << button_down << ".\n";
			owner_.fire(button_down, *focus_);
		}
	}
	handled = true;
}

template <event_t sdl_button_down,
		  event_t sdl_button_up,
		  event_t button_down,
		  event_t button_up,
		  event_t button_click,
		  event_t button_double_click>
void mouse_button<sdl_button_down,
				   sdl_button_up,
				   button_down,
				   button_up,
				   button_click,
				   button_double_click>::
		signal_handler_sdl_button_up(const event::event_t event,
									 bool& handled,
									 const point& coordinate)
{
	if(signal_handler_sdl_button_up_entered_) {
		return;
	}
	locker lock(signal_handler_sdl_button_up_entered_);

	DBG_GUI_E << LOG_HEADER << event << ".\n";

	if(!is_down_) {
		WRN_GUI_E << LOG_HEADER << event
				  << ". The mouse button is already up, we missed an event.\n";
		return;
	}
	is_down_ = false;

	if(focus_) {
		DBG_GUI_E << LOG_HEADER << "Firing: " << sdl_button_up << ".\n";
		if(!owner_.fire(sdl_button_up, *focus_, coordinate)) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << button_up << ".\n";
			owner_.fire(button_up, *focus_);
		}
	}

	widget* mouse_over = owner_.find_at(coordinate, true);
	if(mouse_captured_) {
		const unsigned mask = SDL_BUTTON_LMASK | SDL_BUTTON_MMASK
							  | SDL_BUTTON_RMASK;

		if((SDL_GetMouseState(nullptr, nullptr) & mask) == 0) {
			mouse_captured_ = false;
		}

		if(mouse_focus_ == mouse_over) {
			mouse_button_click(mouse_focus_);
		} else if(!mouse_captured_) {
			mouse_leave();

			if(mouse_over) {
				mouse_enter(mouse_over);
			}
		}
	} else if(focus_ && focus_ == mouse_over) {
		mouse_button_click(focus_);
	}

	focus_ = nullptr;
	handled = true;
}

template <event_t sdl_button_down,
		  event_t sdl_button_up,
		  event_t button_down,
		  event_t button_up,
		  event_t button_click,
		  event_t button_double_click>
void mouse_button<sdl_button_down,
				   sdl_button_up,
				   button_down,
				   button_up,
				   button_click,
				   button_double_click>::mouse_button_click(widget* widget)
{
	Uint32 stamp = SDL_GetTicks();
	if(last_click_stamp_ + settings::double_click_time >= stamp
	   && last_clicked_widget_ == widget) {

		DBG_GUI_E << LOG_HEADER << "Firing: " << button_double_click << ".\n";

		owner_.fire(button_double_click, *widget);
		last_click_stamp_ = 0;
		last_clicked_widget_ = nullptr;

	} else {

		DBG_GUI_E << LOG_HEADER << "Firing: " << button_click << ".\n";
		owner_.fire(button_click, *widget);
		last_click_stamp_ = stamp;
		last_clicked_widget_ = widget;
	}
}

/***** ***** ***** ***** distributor ***** ***** ***** ***** *****/

#undef LOG_HEADER
#define LOG_HEADER "distributor mouse motion [" << owner_.id() << "]: "

/**
 * @todo Test whether the state is properly tracked when an input blocker is
 * used.
 */
distributor::distributor(widget& owner,
						   const dispatcher::queue_position queue_position)
	: mouse_motion(owner, queue_position)
	, mouse_button_left("left", owner, queue_position)
	, mouse_button_middle("middle", owner, queue_position)
	, mouse_button_right("right", owner, queue_position)
#if 0
	, hover_pending_(false)
	, hover_id_(0)
	, hover_box_()
	, had_hover_(false)
	, tooltip_(0)
	, help_popup_(0)
#endif
	, keyboard_focus_(0)
	, keyboard_focus_chain_()
{
	if(SDL_WasInit(SDL_INIT_TIMER) == 0) {
		if(SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
			assert(false);
		}
	}

	owner_.connect_signal<event::SDL_KEY_DOWN>(std::bind(
			&distributor::signal_handler_sdl_key_down, this, _5, _6, _7));

	owner_.connect_signal<event::NOTIFY_REMOVAL>(std::bind(
			&distributor::signal_handler_notify_removal, this, _1, _2));

	initialize_state();
}

distributor::~distributor()
{
	owner_.disconnect_signal<event::SDL_KEY_DOWN>(std::bind(
			&distributor::signal_handler_sdl_key_down, this, _5, _6, _7));

	owner_.disconnect_signal<event::NOTIFY_REMOVAL>(std::bind(
			&distributor::signal_handler_notify_removal, this, _1, _2));
}

void distributor::initialize_state()
{
	const Uint8 button_state = SDL_GetMouseState(nullptr, nullptr);

	mouse_button_left::initialize_state((button_state & SDL_BUTTON(1)) != 0);
	mouse_button_middle::initialize_state((button_state & SDL_BUTTON(2)) != 0);
	mouse_button_right::initialize_state((button_state & SDL_BUTTON(3)) != 0);

	init_mouse_location();
}

void distributor::keyboard_capture(widget* widget)
{
	if(keyboard_focus_) {
		DBG_GUI_E << LOG_HEADER << "Firing: " << event::LOSE_KEYBOARD_FOCUS
				  << ".\n";

		owner_.fire(event::LOSE_KEYBOARD_FOCUS, *keyboard_focus_, nullptr);
	}

	keyboard_focus_ = widget;

	if(keyboard_focus_) {
		DBG_GUI_E << LOG_HEADER << "Firing: " << event::RECEIVE_KEYBOARD_FOCUS
				  << ".\n";

		owner_.fire(event::RECEIVE_KEYBOARD_FOCUS, *keyboard_focus_, nullptr);
	}
}

void distributor::keyboard_add_to_chain(widget* widget)
{
	assert(widget);
	assert(std::find(keyboard_focus_chain_.begin(),
					 keyboard_focus_chain_.end(),
					 widget) == keyboard_focus_chain_.end());

	keyboard_focus_chain_.push_back(widget);
}

void distributor::keyboard_remove_from_chain(widget* w)
{
	assert(w);
	std::vector<widget*>::iterator itor = std::find(
			keyboard_focus_chain_.begin(), keyboard_focus_chain_.end(), w);

	if(itor != keyboard_focus_chain_.end()) {
		keyboard_focus_chain_.erase(itor);
	}
}

void distributor::signal_handler_sdl_key_down(const SDL_Keycode key,
											   const SDL_Keymod modifier,
											   const utf8::string& unicode)
{
	/** @todo Test whether recursion protection is needed. */

	DBG_GUI_E << LOG_HEADER << event::SDL_KEY_DOWN << ".\n";

	if(keyboard_focus_) {
		// Attempt to cast to control, to avoid sending events if the
		// widget is disabled. If the cast fails, we assume the widget
		// is enabled and ready to receive events.
		control* ctrl = dynamic_cast<control*>(keyboard_focus_);
		if(!ctrl || ctrl->get_active()) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << event::SDL_KEY_DOWN
					  << ".\n";
			if(owner_.fire(event::SDL_KEY_DOWN,
						   *keyboard_focus_,
						   key,
						   modifier,
						   unicode)) {
				return;
			}
		}
	}

	for(std::vector<widget*>::reverse_iterator ritor
		= keyboard_focus_chain_.rbegin();
		ritor != keyboard_focus_chain_.rend();
		++ritor) {

		if(*ritor == keyboard_focus_) {
			continue;
		}

		if(*ritor == &owner_) {
			/**
			 * @todo Make sure we're not in the event chain.
			 *
			 * No idea why we're here, but needs to be fixed, otherwise we keep
			 * calling this function recursively upon unhandled events...
			 *
			 * Probably added to make sure the window can grab the events and
			 * handle + block them when needed, this is no longer needed with
			 * the chain.
			 */
			continue;
		}

		// Attempt to cast to control, to avoid sending events if the
		// widget is disabled. If the cast fails, we assume the widget
		// is enabled and ready to receive events.
		control* ctrl = dynamic_cast<control*>(keyboard_focus_);
		if(ctrl != nullptr && !ctrl->get_active()) {
			continue;
		}

		DBG_GUI_E << LOG_HEADER << "Firing: " << event::SDL_KEY_DOWN << ".\n";
		if(owner_.fire(event::SDL_KEY_DOWN, **ritor, key, modifier, unicode)) {

			return;
		}
	}
}

void distributor::signal_handler_notify_removal(dispatcher& w,
												 const event_t event)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	/**
	 * @todo Evaluate whether moving the cleanup parts in the subclasses.
	 *
	 * It might be cleaner to do it that way, but creates extra small
	 * functions...
	 */

	if(hover_widget_ == &w) {
		stop_hover_timer();
	}

	if(mouse_button_left::last_clicked_widget_ == &w) {
		mouse_button_left::last_clicked_widget_ = nullptr;
	}
	if(mouse_button_left::focus_ == &w) {
		mouse_button_left::focus_ = nullptr;
	}

	if(mouse_button_middle::last_clicked_widget_ == &w) {
		mouse_button_middle::last_clicked_widget_ = nullptr;
	}
	if(mouse_button_middle::focus_ == &w) {
		mouse_button_middle::focus_ = nullptr;
	}

	if(mouse_button_right::last_clicked_widget_ == &w) {
		mouse_button_right::last_clicked_widget_ = nullptr;
	}
	if(mouse_button_right::focus_ == &w) {
		mouse_button_right::focus_ = nullptr;
	}

	if(mouse_focus_ == &w) {
		mouse_focus_ = nullptr;
	}

	if(keyboard_focus_ == &w) {
		keyboard_focus_ = nullptr;
	}
	const std::vector<widget*>::iterator itor
			= std::find(keyboard_focus_chain_.begin(),
						keyboard_focus_chain_.end(),
						&w);
	if(itor != keyboard_focus_chain_.end()) {
		keyboard_focus_chain_.erase(itor);
	}
}

} // namespace event

} // namespace gui2
