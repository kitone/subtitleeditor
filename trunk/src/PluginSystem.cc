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

#include "PluginSystem.h"
#include <iostream>
#include "utility.h"


typedef Plugin* (*GetSubtitleeditorPluginFunc) (void);

/*
 *
 */
PluginSystem::PluginSystem()
{
	se_debug(SE_DEBUG_APP);

	g_return_if_fail(Glib::Module::get_supported());

}

/*
 *
 */
PluginSystem::~PluginSystem()
{
	se_debug(SE_DEBUG_APP);
	//for(std::list<Plugin*>::iterator it = m_plugins.begin(); it!=m_plugins.end(); ++it)
	//	delete *it;
}

/*
 *
 */
void PluginSystem::load_plugins()
{
	/*
	SubtitleEditorWindow *window = SubtitleEditorWindow::get_instance();
	
	g_return_if_fail(window);

	g_return_if_fail(load_module("plugins/sample/.libs/libsample.so"));

	std::cout << "load plugins..." << std::endl;

	for(std::list<Plugin*>::iterator it = m_plugins.begin(); it!=m_plugins.end(); ++it)
	{
		(*it)->init(window);
		(*it)->activate();
	}
	std::cout << "load plugins...OK" << std::endl;
	*/
}


/*
 *
 */
bool PluginSystem::load_module(const Glib::ustring &file)
{
	/*
	Glib::Module module(file);
	
	std::cout << "load module ..." << std::endl;

	if(!module)
	{
		std::cerr << "Failed to create module: " << Glib::Module::get_last_error() << std::endl;
		return false;
	}
	//
	{
		void *func = NULL;

		if(module.get_symbol("get_module", func) == false)
			std::cerr << "get_symbol failed: " << Glib::Module::get_last_error() << std::endl;
	}
	//
	{
		void *func = NULL;

		if(module.get_symbol("get_subtitleeditor_plugin", func) == false)
		{
			std::cerr << "get_symbol failed: " << Glib::Module::get_last_error() << std::endl;
			return false;
		}

		GetSubtitleeditorPluginFunc get_subtitleeditor_plugin = NULL;
		get_subtitleeditor_plugin = (GetSubtitleeditorPluginFunc)func;

		g_return_val_if_fail(get_subtitleeditor_plugin, false);

		Plugin *plugin = get_subtitleeditor_plugin();

		g_return_val_if_fail(plugin, false);

		m_plugins.push_back(plugin);
	}
	// TODO: ptr to module
	module.make_resident();
	//delete module;

	std::cout << "load_module (" << file << ") OK" << std::endl;
	return true;
	*/
	return false;
}


/*
 *	hack
 */
void PluginSystem::add(Plugin *plugin)
{
	se_debug(SE_DEBUG_APP);

	m_plugins.push_back(plugin);
}

/*
 *
 */
void PluginSystem::activate_plugins()
{
	se_debug(SE_DEBUG_APP);

	SubtitleEditorWindow *window = SubtitleEditorWindow::get_instance();

	g_return_if_fail(window);

	Plugin::init(window);

	for(std::list<Plugin*>::iterator it = m_plugins.begin(); it!=m_plugins.end(); ++it)
	{
		(*it)->activate();
		
		(*it)->update_ui();
	}
}

/*
 *
 */
void PluginSystem::post_activate_plugins()
{
	se_debug(SE_DEBUG_APP);

	SubtitleEditorWindow *window = SubtitleEditorWindow::get_instance();

	g_return_if_fail(window);

	Plugin::init(window);

	for(std::list<Plugin*>::iterator it = m_plugins.begin(); it!=m_plugins.end(); ++it)
	{
		(*it)->post_activate();
		
		(*it)->update_ui();
	}
}

/*
 *
 */
void PluginSystem::update_ui()
{
	se_debug(SE_DEBUG_APP);

	SubtitleEditorWindow *window = SubtitleEditorWindow::get_instance();

	g_return_if_fail(window);

	for(std::list<Plugin*>::iterator it = m_plugins.begin(); it!=m_plugins.end(); ++it)
	{
		(*it)->update_ui();
	}
}


/*
 * hack
 */
PluginSystem& PluginSystem::get_instance()
{
	se_debug(SE_DEBUG_APP);

	static PluginSystem ps;
	return ps;
}

