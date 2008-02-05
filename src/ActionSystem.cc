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
 
#include "ActionSystem.h"
#include <iostream>

/*
 *	static instance
 */
ActionSystem& ActionSystem::getInstance()
{
	static ActionSystem instance;
	return instance;
}


/*
 *
 */
ActionSystem::ActionSystem()
{
	//std::cout << "ActionSystem::ActionSystem" << std::endl;
}

/*
 *
 */
ActionSystem::~ActionSystem()
{
	//std::cout << "ActionSystem::~ActionSystem" << std::endl;
}

/*
 *
 */
sigc::signal<void, Glib::ustring>& ActionSystem::signal_emit()
{
	return m_signal_emit;
}

/*
 *
 */
void ActionSystem::execute(const Glib::ustring &name)
{
	//std::cout << "ActionSystem::execute: " << name << std::endl;
	m_signal_emit(name);
}

/*
 *	enable/disable action
 */
void ActionSystem::set_sensitive(const Glib::ustring &name, bool state)
{
	m_signal_sensitive(name, state);
}

/*
 *
 */
sigc::signal<void, Glib::ustring, bool>& ActionSystem::signal_sensitive_changed()
{
	return m_signal_sensitive;
}
