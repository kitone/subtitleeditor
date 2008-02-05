#ifndef _ActionSystem_h
#define _ActionSystem_h

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
 

#include <glibmm/ustring.h>
#include <sigc++/sigc++.h>

/*
 *	A long terme cette class vas servir d'interface 
 *	pour d'autres languages (je pense a LUA)
 *	pour rendre le script plus simple
 *	avec un system de nom + argument
 *
 *	ex:
 *	execute("search", "toto")
 *	execute("open", "/home/toto/subtitle.srt")
 *	execute("change-fps", 25, 23.279)
 *	...
 */
class ActionSystem
{
	sigc::signal<void, Glib::ustring> m_signal_emit;
	sigc::signal<void, Glib::ustring, bool> m_signal_sensitive;
protected:
	/*
	 *
	 */
	ActionSystem();
	
	/*
	 *
	 */
	~ActionSystem();


public:

	/*
	 *	instance
	 */
	static ActionSystem& getInstance();

	/*
	 *	execute une action a partir du nom
	 */
	void execute(const Glib::ustring &name);

	/*
	 *
	 */
	sigc::signal<void, Glib::ustring>& signal_emit();

	/*
	 *	enable/disable action
	 */
	void set_sensitive(const Glib::ustring &name, bool state);

	/*
	 *
	 */
	sigc::signal<void, Glib::ustring, bool>& signal_sensitive_changed();
};

#endif//_ActionSystem_h

