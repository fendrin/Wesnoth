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

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "location_palette.hpp"

#include "gettext.hpp"
#include "marked-up_text.hpp"
#include "tooltips.hpp"

#include "editor/action/mouse/mouse_action.hpp"

#include "wml_separators.hpp"
#include "formula/string_utils.hpp"
class location_palette_item : public gui::widget
{
public:
	struct tstate {
		tstate()
			: selected(false)
			, mouseover(false)
		{}
		bool selected;
		bool mouseover;
		friend bool operator==(tstate r, tstate l) 
		{
			return r.selected == l.selected && r.mouseover == l.mouseover;
		}

	};
	location_palette_item(CVideo& video, editor::location_palette& parent)
		: gui::widget(video, true)
		, parent_(parent)
	{
	}

	void draw_contents() override
	{
		if (state_.mouseover) {
			sdl::draw_solid_tinted_rectangle(location().x, location().y, location().w, location().h, 200, 200, 200, 0.1, video().getSurface());
		}
		if (state_.selected) {
			sdl::draw_rectangle(location().x, location().y, location().w, location().h, -1, video().getSurface());
		}
		font::draw_text(&video(), location(), 16, font::NORMAL_COLOR, desc_.empty() ? id_ : desc_, location().x + 2, location().y, 0);
	}

	//TODO move to widget
	bool hit(int x, int y) const
	{
		return sdl::point_in_rect(x, y, location());
	}

	void mouse_up(SDL_MouseButtonEvent const &e)
	{
		if (!(hit(e.x, e.y)))
			return;
		if (e.button == SDL_BUTTON_LEFT) {
			parent_.select_item(id_);
		}
		if (e.button == SDL_BUTTON_RIGHT) {
			//TODO: implement 'jump to item' or 'delete item' here.
		}
	}

	void handle_event(const SDL_Event& e) override
	{
		gui::widget::handle_event(e);

		if (hidden() || !enabled() || mouse_locked())
			return;

		tstate start_state = state_;

		switch (e.type) {
		case SDL_MOUSEBUTTONUP:
			mouse_up(e.button);
			break;
		case SDL_MOUSEMOTION:
			state_.mouseover = hit(e.motion.x, e.motion.y);
			break;
		default:
			return;
		}

		if (!(start_state == state_))
			set_dirty(true);
	}

	void set_item_id(const std::string& id)
	{
		id_ = id;
		bool is_number = std::find_if(id.begin(), id.end(), [](char c) { return !std::isdigit(c); }) == id.end();
		if (is_number) {
			desc_ = vgettext("Player $side_num", utils::string_map{ {"side_num", id} });
		}
		else {
			desc_ = "";
		}
	}
	void set_selected(bool selected)
	{
		state_.selected = selected;
	}
	void draw() { gui::widget::draw(); }
private:
	std::string id_;
	std::string desc_;
	tstate state_;
	editor::location_palette& parent_;
};

class location_palette_button : public gui::button
{
public:
	location_palette_button(CVideo& video, const SDL_Rect& location, const std::string& text, const std::function<void (void)>& callback)
		: gui::button(video, text)
		, callback_(callback)
	{
		this->set_location(location.x, location.y);
		this->hide(false);
	}
protected:
	virtual void mouse_up(const SDL_MouseButtonEvent& e) override
	{
		gui::button::mouse_up(e);
		if (this->pressed()) {
			callback_();
		}
	}
	std::function<void (void)> callback_;

};
namespace editor {
location_palette::location_palette(editor_display &gui, const config& /*cfg*/, mouse_action** active_mouse_action)
		: common_palette(gui)
		, gui_(gui)
		, item_size_(20)
		//TODO avoid magic number
		, item_space_(20 + 3)
		, palette_y_(0)
		, palette_x_(0)
		, nitems_(0)
		, nmax_items_(0)
		, items_start_(0)
		, selected_item_()
		, items_()
		, active_mouse_action_(active_mouse_action)
		, buttons_()
		, button_add_()
		, button_delete_()
		, button_goto_()
		, help_handle_(-1)
		, disp_(gui)
	{
		for (int i = 1; i < 10; ++i) {
			items_.push_back(std::to_string(i));
		}
		selected_item_ = items_[0];
	}

sdl_handler_vector location_palette::handler_members()
{
	sdl_handler_vector h;
	for (gui::widget& b : buttons_) {
		h.push_back(&b);
	}
	return h;
}

bool location_palette::scroll_up()
{
	unsigned int decrement = 1;
	if(items_start_ >= decrement) {
		items_start_ -= decrement;
		draw();
		return true;
	}
	return false;
}
bool location_palette::can_scroll_up()
{
	return (items_start_ != 0);
}

bool location_palette::can_scroll_down()
{
	return (items_start_ + nitems_ + 1 <= num_items());
}

bool location_palette::scroll_down()
{
	bool end_reached = (!(items_start_ + nitems_ + 1 <= num_items()));
	bool scrolled = false;

	// move downwards
	if(!end_reached) {
		items_start_ += 1;
		scrolled = true;
		set_dirty(true);
	}
	draw();
	return scrolled;
}


void location_palette::adjust_size(const SDL_Rect& target)
{
	palette_x_ = target.x;
	palette_y_ = target.y;
	const int button_height = 30;
	int bottom = target.y + target.h - button_height;
	

	button_add_.reset();
	button_delete_.reset();
	button_goto_.reset();
	
	button_goto_.reset(new location_palette_button(video(), SDL_Rect{ target.x , bottom, target.w - 10, button_height }, _("Go To"), [this]() {
		//static_cast<mouse_action_starting_position&>(**active_mouse_action_).
		map_location pos = disp_.get_map().starting_position(std::stoi(selected_item_));
		if (pos.valid()) {
			disp_.scroll_to_tile(pos, display::WARP);
		}
	}));
	/*
	button_delete_.reset(new location_palette_button(video(), SDL_Rect{ target.x , bottom -= button_height, target.w - 10, button_height }, _("Delete"), [this]() {
		//static_cast<mouse_action_starting_position&>(**active_mouse_action_).
			//we currently don'T have access to editor_controller::perfm_delete here whcih we need to delete statign positions.
			// maybe we can all something like fire_hotkey here.
		}));
	*/
	const size_t space_for_items = bottom - target.x;
	const unsigned items_fitting = static_cast<unsigned> (space_for_items / item_space_);
	nitems_ = std::min<int>(items_fitting, items_.size());
	if (buttons_.size() != nitems_) {
		buttons_.resize(nitems_, &location_palette_item(gui_.video(), *this));
	}
	set_location(target);
	set_dirty(true);
	gui_.video().clear_help_string(help_handle_);
	help_handle_ = gui_.video().set_help_string(get_help_string());
}

void location_palette::select_item(const std::string& item_id)
{
	if (selected_item_ != item_id) {
		selected_item_ = item_id;
		set_dirty();
	}
	gui_.video().clear_help_string(help_handle_);
	help_handle_ = gui_.video().set_help_string(get_help_string());
}

size_t location_palette::num_items()
{
	return items_.size();
}

bool location_palette::is_selected_item(const std::string& id)
{
	return selected_item_ == id;
}

void location_palette::draw_contents()
{
	if (*active_mouse_action_)
		(*active_mouse_action_)->set_mouse_overlay(gui_);
	unsigned int y = palette_y_;
	const unsigned int x = palette_x_;
	const unsigned int starting = items_start_;
	unsigned int ending = starting + nitems_;
	if(ending > num_items() ){
		ending = num_items();
	}

	gui::button* upscroll_button = gui_.find_action_button("upscroll-button-editor");
	if (upscroll_button)
		upscroll_button->enable(starting != 0);
	gui::button* downscroll_button = gui_.find_action_button("downscroll-button-editor");
	if (downscroll_button)
		downscroll_button->enable(ending != num_items());


	unsigned int counter = starting;
	for (unsigned int i = 0 ; i < buttons_.size() ; i++) {
		//TODO check if the conditions still hold for the counter variable
		//for (unsigned int counter = starting; counter < ending; counter++)

		location_palette_item & tile = buttons_[i];

		tile.hide(true);

		if (i >= ending) continue;

		const std::string item_id = items_[counter];

		std::stringstream tooltip_text;

		const int counter_from_zero = counter - starting;
		SDL_Rect dstrect;
		dstrect.x = x;
		dstrect.y = y;
		dstrect.w = location().w - 10;
		dstrect.h = item_size_ + 2;

		tile.set_location(dstrect);
		tile.set_tooltip_string(tooltip_text.str());
		tile.set_item_id(item_id);
		tile.set_selected(is_selected_item(item_id));
		tile.set_dirty(true);
		tile.hide(false);
		tile.draw();

		// Adjust location
		y += item_space_;
		counter++;
	}
	update_rect(location());
}

void location_palette::hide(bool hidden) {
	widget::hide(hidden);
	if (!hidden)
		help_handle_ = gui_.video().set_help_string(get_help_string());
	else gui_.video().clear_help_string(help_handle_);
	for (gui::widget& w : buttons_) {
		w.hide(hidden);
	}
}
} // end namespace editor
