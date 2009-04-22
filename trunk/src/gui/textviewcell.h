#ifndef _TextViewCell_h
#define _TextViewCell_h

/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2009, kitone
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

#include <gtkmm/textview.h>
#include <gtkmm/celleditable.h>

/*
 * Cell text editable with multiline support.
 */
class TextViewCell : public Gtk::TextView, public Gtk::CellEditable
{
public:

	/*
	 *
	 */
	TextViewCell();

	/*
	 *
	 */
	Glib::ustring get_text();

	/*
	 *
	 */
	void set_text(const Glib::ustring &text);

protected:

	/*
	 *
	 */
	bool on_key_press_event(GdkEventKey* event);

	/*
	 *
	 */
	bool on_focus_out_event(GdkEventFocus *ev);

	/*
	 *
	 */
	void editing_done();
 
	/*
	 *
	 */
	void on_populate_popup (Gtk::Menu* menu);

	/*
	 *
	 */
	void unmap_popup();

protected:
	bool m_in_popup;
	bool m_canceled;
	bool m_used_ctrl_enter_to_confirm_change;
};

#endif//_TextViewCell_h
