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

#include "ComboBoxSubtitleFormat.h"

/*
 * Constructor
 */
ComboBoxSubtitleFormat::ComboBoxSubtitleFormat(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:Gtk::ComboBoxText(cobject)
{
	//FIXME: SubtitleSystem
	/*
	std::list<Glib::ustring> formats = SubtitleSystem::getInstance().get_formats();

	std::list<Glib::ustring>::const_iterator it;

	for(it = formats.begin(); it != formats.end(); ++it)
		append_text(*it);

	set_active(0);
	*/
}

/*
 *
 */
void ComboBoxSubtitleFormat::set_value(const Glib::ustring &value)
{
	set_active_text(value);
}

/*
 * Returns the subtitle format selected.
 */
Glib::ustring ComboBoxSubtitleFormat::get_value() const
{
	return get_active_text();
}
