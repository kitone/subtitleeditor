#ifndef _PluginSystem_h
#define _PluginSystem_h

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

#include "Plugin.h"

class PluginSystem
{
protected:

	/*
	 *
	 */
	PluginSystem();

	/*
	 *
	 */
	~PluginSystem();

public:

	/*
	 *
	 */
	void load_plugins();

	/*
	 *
	 */
	void activate_plugins();

	/*
	 *
	 */
	void post_activate_plugins();

	/*
	 *
	 */
	void update_ui();

	/*
	 *	hack
	 */
	void add(Plugin *plugin);

	/*
	 * hack
	 */
	static PluginSystem& get_instance();

protected:

	/*
	 *
	 */
	bool load_module(const Glib::ustring &file);

protected:
	std::list<Plugin*> m_plugins;
};

//#define SUBTITLEEDITOR_PLUGIN_REGISTER()


#endif//_PluginSystem_h

