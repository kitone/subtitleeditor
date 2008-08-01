#ifndef _GtkUtility_h
#define _GtkUtility_h

/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2008, kitone
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtkmm/spinbutton.h>
#include <libglademm/xml.h>
#include "SubtitleTime.h"
#include "TimeUtility.h"

/*
 *
 */
class SpinButtonTime : public Gtk::SpinButton
{
public:
	
	/*
	 *
	 */
	SpinButtonTime();

	/*
	 *
	 */
	SpinButtonTime(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	
	/*
	 *
	 */
	void set_timing_mode(TIMING_MODE mode);

	/*
	 *
	 */
	TIMING_MODE get_timing_mode();

	/*
	 *
	 */
	void set_negative(bool state);

protected:

	/*
	 *
	 */
	void default_init();

	/*
	 *
	 */
	void init_frame_mode();

	/*
	 *
	 */
	void init_time_mode();

	/*
	 *
	 */
	int on_input(double *new_value);

	/*
	 *
	 */
	bool on_output();

	/*
	 *
	 */
	void on_size_request(Gtk::Requisition *req);

	/*
	 *
	 */
	bool on_scroll_event(GdkEventScroll *ev);

	/*
	 *
	 */
	void on_insert_text(const Glib::ustring &str, int *pos);
	
	/*
	 *
	 */
	void init_range();

protected:
	TIMING_MODE m_timing_mode;
	bool m_negative;
};

#endif//_GtkUtility_h

