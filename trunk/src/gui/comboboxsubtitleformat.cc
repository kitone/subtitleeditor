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

#include "comboboxsubtitleformat.h"
#include "subtitleformatsystem.h"

/*
 * Constructor
 */
ComboBoxSubtitleFormat::ComboBoxSubtitleFormat(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
:Gtk::ComboBoxText(cobject)
{
	std::list<SubtitleFormatInfo> infos = SubtitleFormatSystem::instance().get_infos();

	std::list<SubtitleFormatInfo>::const_iterator it;

	for(it = infos.begin(); it != infos.end(); ++it)
		append_text((*it).name);

	set_active(0);
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
