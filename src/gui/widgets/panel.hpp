/*
   Copyright (C) 2008 - 2013 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_PANEL_HPP_INCLUDED
#define GUI_WIDGETS_PANEL_HPP_INCLUDED

#include "gui/widgets/container.hpp"

namespace gui2 {

/**
 * Visible container to hold multiple widgets.
 *
 * This widget can draw items beyond the widgets it holds and in front of them.
 * A panel is always active so these functions return dummy values.
 */
class tpanel : public tcontainer_
{

public:

	/**
	 * Constructor.
	 *
	 * @param canvas_count        The canvas count for tcontrol.
	 */
	explicit tpanel(const unsigned canvas_count = 2) :
		tcontainer_(canvas_count)
	{
	}

	/**
	 * Returns the client rect.
	 *
	 * The client rect is the area which is used for child items. The rest of
	 * the area of this widget is used for its own decoration.
	 *
	 * @returns                   The client rect.
	 */
	virtual SDL_Rect get_client_rect() const;

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const OVERRIDE;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const OVERRIDE;

private:

	/** See @ref twidget::impl_draw_background. */
	virtual void impl_draw_background(surface& frame_buffer) OVERRIDE;

	/** See @ref twidget::impl_draw_background. */
	virtual void impl_draw_background(
			  surface& frame_buffer
			, int x_offset
			, int y_offset) OVERRIDE;

	/** See @ref twidget::impl_draw_foreground. */
	virtual void impl_draw_foreground(surface& frame_buffer) OVERRIDE;

	/** See @ref twidget::impl_draw_foreground. */
	virtual void impl_draw_foreground(
			  surface& frame_buffer
			, int x_offset
			, int y_offset) OVERRIDE;

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const;

	/** Inherited from tcontainer_. */
	tpoint border_space() const;

	/** Inherited from tcontainer_. */
	void set_self_active(const bool /*active*/) {}

};

} // namespace gui2

#endif


