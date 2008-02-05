#ifndef _Plugin_h
#define _Plugin_h

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

#include <gtkmm.h>
#include "SubtitleEditorWindow.h"
#include "Config.h"


class Plugin
{
public:

	/*
	 *
	 */
	Plugin();

	/*
	 *
	 */
	virtual ~Plugin();

	/*
	 *
	 */
	virtual void activate();

	/*
	 *
	 */
	virtual void post_activate();

	/*
	 *
	 */
	virtual void deactivate();

	/*
	 *
	 */
	virtual void update_ui();

	/*
	 *
	 */
	virtual Gtk::Widget* create_configure_dialog();

	/*
	 *
	 */
	bool is_configurable();

	/*
	 *	static method
	 */

	/*
	 *
	 */
	static void init(SubtitleEditorWindow *window);

	/*
	 *
	 */
	static SubtitleEditorWindow* get_subtitleeditor_window();

	/*
	 *
	 */
	static Config& get_config();

	/*
	 *
	 */
	static Document* get_current_document();

	/*
	 *
	 */
	static Glib::RefPtr<Gtk::UIManager> get_ui_manager();

private:
	static SubtitleEditorWindow* m_window;
};

/*
 *	FIXME: HACK
 */
#define REGISTER_PLUGIN(class) static class static_##class;

#endif//_Plugin_h

