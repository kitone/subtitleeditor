#ifndef _ComboBoxSubtitleFormat_h
#define _ComboBoxSubtitleFormat_h

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
#include <gtkmm/comboboxtext.h>
#include <libglademm/xml.h>

/*
 * ComboBox for choosing the format of subtitle.
 */
class ComboBoxSubtitleFormat : public Gtk::ComboBoxText
{
public:

	/*
	 * Constructor
	 */
	ComboBoxSubtitleFormat(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

	/*
	 *
	 */
	void set_value(const Glib::ustring &value);

	/*
	 * Returns the subtitle format selected.
	 */
	Glib::ustring get_value() const;
};

#endif//_ComboBoxSubtitleFormat_h
