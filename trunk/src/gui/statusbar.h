#ifndef _Statusbar_h
#define _Statusbar_h

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
 

#include <gtkmm/statusbar.h>
#include <gtkmm/builder.h>

class Statusbar : public Gtk::Statusbar
{
public:
	Statusbar(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& builder);
	~Statusbar();

	void push_text(const Glib::ustring &text);
	void pop_text();

	/*
	 *	affiche un message pendant 3 sec
	 */
	void flash_message(const Glib::ustring &text);
protected:
	bool on_timeout();

protected:
	sigc::connection m_connection_timeout;
};

#endif//_Statusbar_h

