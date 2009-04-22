#ifndef _gtkmm_utility_h
#define _gtkmm_utility_h

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
#include "debug.h"

namespace gtkmm_utility {

/*
 * Create and return a widget derived from glade file.
 */
template<class T>
T* get_widget_derived(const Glib::ustring &path, const Glib::ustring &glade_file, const Glib::ustring &name)
{
	se_debug_message(SE_DEBUG_UTILITY, "glade_file=<%s> name=<%s>", glade_file.c_str(), name.c_str());

	T *dialog = NULL;

	Glib::ustring file = Glib::build_filename(path, glade_file);

	Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(file);

	refXml->get_widget_derived(name, dialog);
	return dialog;
}

}//namespace gtkmm_utility

#endif //gtkmm_utility_h
