#ifndef _DocumentPage_h
#define _DocumentPage_h

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

#include "preferencepage.h"
#include "gui/comboboxsubtitleformat.h"
#include "gui/comboboxnewline.h"


class DocumentPage : public PreferencePage
{
public:

	/*
	 *
	 */
	DocumentPage(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& xml)
	:PreferencePage(cobject)
	{
		init_widget_derived<ComboBoxSubtitleFormat>(xml, "combo-format", "document", "format");
		init_widget_derived<ComboBoxNewLine>(xml, "combo-newline", "document", "newline");
	}
};

#endif//_DocumentPage_h
