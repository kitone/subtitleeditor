#ifndef _ComboBoxNewLine_h
#define _ComboBoxNewLine_h

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
#include <gtkmm/builder.h>

/*
 * ComboBox for choosing the NewLine type. 
 * Windows or Unix.
 */
class ComboBoxNewLine : public Gtk::ComboBoxText
{
public:

	/*
	 * Constructor
	 */
	ComboBoxNewLine(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

	/*
	 * Sets current value.
	 * "Unix" or "Windows"
	 */
	void set_value(const Glib::ustring &value);

	/*
	 * Returns the NewLine type.
	 * Windows or Unix.
	 */
	Glib::ustring get_value() const;
};

#endif//_ComboBoxNewLine_h
