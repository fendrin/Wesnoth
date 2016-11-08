/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/dialog.hpp"

#include "gui/auxiliary/field.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "scripting/plugins/context.hpp"
#include "scripting/plugins/manager.hpp"
#include "video.hpp"

namespace gui2
{

tdialog::~tdialog()
{
	for(auto field : fields_)
	{
		delete field;
	}
}

bool tdialog::show(CVideo& video, const unsigned auto_close_time)
{
	if(video.faked() && !show_even_without_video_) {
		if(!allow_plugin_skip_) {
			return false;
		}

		plugins_manager* pm = plugins_manager::get();
		if (pm && pm->any_running())
		{
			plugins_context pc("Dialog");
			pc.set_callback("skip_dialog", [this](config) { retval_ = window::OK; }, false);
			pc.set_callback("quit", [](config) {}, false);
			pc.play_slice();
		}

		return false;
	}

	std::unique_ptr<window> window(build_window(video));
	assert(window.get());

	post_build(*window);

	window->set_owner(this);

	init_fields(*window);

	pre_show(*window);

	open_window_stack.push_back(window.get());

	retval_ = window->show(restore_, auto_close_time);

	open_window_stack.pop_back();

	/*
	 * It can happen that when two clicks follow each other fast that the event
	 * handling code in events.cpp generates a DOUBLE_CLICK_EVENT. For some
	 * reason it can happen that this event gets pushed in the queue when the
	 * window is shown, but processed after the window is closed. This causes
	 * the next window to get this pending event.
	 *
	 * This caused a bug where double clicking in the campaign selection dialog
	 * directly selected a difficulty level and started the campaign. In order
	 * to avoid that problem, filter all pending DOUBLE_CLICK_EVENT events after
	 * the window is closed.
	 */
	SDL_FlushEvent(DOUBLE_CLICK_EVENT);

	finalize_fields(*window, (retval_ == window::OK || always_save_fields_));

	post_show(*window);

	// post_show may have updated the windoe retval. Update it here.
	retval_ = window->get_retval();

	return retval_ == window::OK;
}

field_bool* tdialog::register_bool(
		const std::string& id,
		const bool mandatory,
		const std::function<bool()> callback_load_value,
		const std::function<void(bool)> callback_save_value,
		const std::function<void(widget&)> callback_change,
		const bool initial_fire)
{
	field_bool* field = new field_bool(id,
										 mandatory,
										 callback_load_value,
										 callback_save_value,
										 callback_change,
										 initial_fire);

	fields_.push_back(field);
	return field;
}

field_bool*
tdialog::register_bool(const std::string& id,
					   const bool mandatory,
					   bool& linked_variable,
					   const std::function<void(widget&)> callback_change,
					   const bool initial_fire)
{
	field_bool* field
			= new field_bool(id, mandatory, linked_variable, callback_change, initial_fire);

	fields_.push_back(field);
	return field;
}

field_integer* tdialog::register_integer(
		const std::string& id,
		const bool mandatory,
		const std::function<int()> callback_load_value,
		const std::function<void(const int)> callback_save_value)
{
	field_integer* field = new field_integer(
			id, mandatory, callback_load_value, callback_save_value);

	fields_.push_back(field);
	return field;
}

field_integer* tdialog::register_integer(const std::string& id,
										  const bool mandatory,
										  int& linked_variable)
{
	field_integer* field = new field_integer(id, mandatory, linked_variable);

	fields_.push_back(field);
	return field;
}

field_text* tdialog::register_text(
		const std::string& id,
		const bool mandatory,
		const std::function<std::string()> callback_load_value,
		const std::function<void(const std::string&)> callback_save_value,
		const bool capture_focus)
{
	field_text* field = new field_text(
			id, mandatory, callback_load_value, callback_save_value);

	if(capture_focus) {
		focus_ = id;
	}

	fields_.push_back(field);
	return field;
}

field_text* tdialog::register_text(const std::string& id,
									const bool mandatory,
									std::string& linked_variable,
									const bool capture_focus)
{
	field_text* field = new field_text(id, mandatory, linked_variable);

	if(capture_focus) {
		focus_ = id;
	}

	fields_.push_back(field);
	return field;
}

field_label* tdialog::register_label(const std::string& id,
									  const bool mandatory,
									  const std::string& text,
									  const bool use_markup)
{
	field_label* field = new field_label(id, mandatory, text, use_markup);

	fields_.push_back(field);
	return field;
}

window* tdialog::build_window(CVideo& video) const
{
	return build(video, window_id());
}

void tdialog::post_build(window& /*window*/)
{
	/* DO NOTHING */
}

void tdialog::pre_show(window& /*window*/)
{
	/* DO NOTHING */
}

void tdialog::post_show(window& /*window*/)
{
	/* DO NOTHING */
}

void tdialog::init_fields(window& window)
{
	for(auto field : fields_)
	{
		field->attach_to_window(window);
		field->widget_init(window);
	}

	if(!focus_.empty()) {
		if(widget* widget = window.find(focus_, false)) {
			window.keyboard_capture(widget);
		}
	}
}

void tdialog::finalize_fields(window& window, const bool save_fields)
{
	for(auto field : fields_)
	{
		if(save_fields) {
			field->widget_finalize(window);
		}
		field->detach_from_window();
	}
}

} // namespace gui2


/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 1
 *
 * {{Autogenerated}}
 *
 * = Window definition =
 *
 * The window definition define how the windows shown in the dialog look.
 */

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = ZZZZZZ_footer
 *
 * [[Category: WML Reference]]
 * [[Category: GUI WML Reference]]
 */
