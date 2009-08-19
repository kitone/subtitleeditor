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

#include <gtkmm/builder.h>
#include "debug.h"
#include <iostream>

namespace gtkmm_utility {

/*
 * Create and return a widget derived from ui file.
 */
template<class T>
T* get_widget_derived(const Glib::ustring &path, const Glib::ustring &ui_file, const Glib::ustring &name)
{
	se_debug_message(SE_DEBUG_UTILITY, "ui_file=<%s> name=<%s>", ui_file.c_str(), name.c_str());

	T *dialog = NULL;

	try
	{
		Glib::ustring file = Glib::build_filename(path, ui_file);

		Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(file);

		refXml->get_widget_derived(name, dialog);
		return dialog;
	}
	catch(const Glib::Error &ex)
	{
		std::cerr << "get_widget_derived failed: " << ex.what() << std::endl;
	}
	return NULL;
}

}//namespace gtkmm_utility

#endif //gtkmm_utility_h
