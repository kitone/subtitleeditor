#ifndef _CheckButtonWithMemory_h
#define _CheckButtonWithMemory_h

/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2012, kitone
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

#include <gtkmm/checkbutton.h>
#include <gtkmm/builder.h>

/*
 * CheckButton for selecting how blank lines are treated in plain text files.
 */
class CheckButtonWithMemory: public Gtk::CheckButton
{
public:

	/*
	 * Constructor
	 * - accepts identifiers used to retreive and store the button state in Config 
	 */
	CheckButtonWithMemory(const Glib::ustring &group, const Glib::ustring &key);

	/*
	 * Constructor
 	 * I DONT UNDERSTAND THIS! I guess it needs to be there for Gtk::Builder
	 */
	CheckButtonWithMemory(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

	/*
	 * Sets current state - overloading a ToggleButton method
	 */
	void set_active(bool state);

	/*
	 * Sets the config group and key names
	 */
	void link_to_cfg(const Glib::ustring &group, const Glib::ustring &key, bool def_state = false );

	/*
	 * read state from Config
	 */
	void init_state();

protected:
	/*
	 * on_toggled handler
	 */
	void on_button_toggled();


protected:

	sigc::connection m_button_toggled;

	Glib::ustring group_name;
	Glib::ustring key_name;
};

#endif //_CheckButtonWithMemory_h

