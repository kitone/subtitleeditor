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
#include <iostream>
#include "PluginSystem.h"
#include "debug.h"
#include "Document.h"

SubtitleEditorWindow* Plugin::m_window = NULL;
/*
 *
 */
Plugin::Plugin()
{
	PluginSystem::get_instance().add(this);
}

/*
 *
 */
Plugin::~Plugin()
{
}

/*
 *
 */
void Plugin::init(SubtitleEditorWindow *window)
{
	se_debug(SE_DEBUG_PLUGINS);

	m_window = window;
}

/*
 *
 */
SubtitleEditorWindow* Plugin::get_subtitleeditor_window()
{
	se_debug(SE_DEBUG_PLUGINS);

	return m_window;
}

/*
 *
 */
Config& Plugin::get_config()
{
	se_debug(SE_DEBUG_PLUGINS);

	return Config::getInstance();
}

/*
 *
 */
Document* Plugin::get_current_document()
{
	se_debug(SE_DEBUG_PLUGINS);

	SubtitleEditorWindow *window = SubtitleEditorWindow::get_instance();

	g_return_val_if_fail(window, NULL);

	Document *doc = window->get_current_document();

	se_debug_message(SE_DEBUG_PLUGINS, "document=%s", ((doc == NULL) ? "NULL" : doc->getFilename().c_str()));

	return doc;
}

/*
 *
 */
Glib::RefPtr<Gtk::UIManager> Plugin::get_ui_manager()
{
	se_debug(SE_DEBUG_PLUGINS);

	g_return_val_if_fail(m_window, Glib::RefPtr<Gtk::UIManager>());

	return m_window->get_ui_manager();
}

/*
 *
 */
void Plugin::activate()
{
	se_debug(SE_DEBUG_PLUGINS);
}

/*
 *
 */
void Plugin::post_activate()
{
	se_debug(SE_DEBUG_PLUGINS);
}

/*
 *
 */
void Plugin::deactivate()
{
	se_debug(SE_DEBUG_PLUGINS);
}

/*
 *
 */
void Plugin::update_ui()
{
	se_debug(SE_DEBUG_PLUGINS);
}

/*
 *
 */
Gtk::Widget* Plugin::create_configure_dialog()
{
	se_debug(SE_DEBUG_PLUGINS);

	return NULL;
}

/*
 *
 */
bool Plugin::is_configurable()
{
	se_debug(SE_DEBUG_PLUGINS);

	return false;
}

