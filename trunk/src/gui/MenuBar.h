#ifndef _MenuBar_h
#define _MenuBar_h

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
#include <map>
#include "Config.h"
#include "Statusbar.h"


class MenuBar : public Gtk::VBox
{
public:
	MenuBar();
	~MenuBar();

	/*
	 *
	 */
	void create(Gtk::Window &window, Statusbar &statusbar);

	/*
	 *
	 */
	void create_ui_from_file();
protected:
	void connect_proxy(const Glib::RefPtr<Gtk::Action> &action, Gtk::Widget *widget);

	void addToggleActionColumn(
											const Glib::ustring &name, 
											const Glib::ustring &label );
	
	/*
	 *
	 */
	void addToggleAction(	const Glib::ustring &name,
												const Glib::ustring &label, 
												const Glib::ustring &group, 
												const Glib::ustring &key );

protected:
	
	void action_activate(const Glib::RefPtr<Gtk::Action> action);

	void execute(const Glib::ustring &name);

	/*
	 *
	 */
	void set_toggle_column(const Glib::ustring &column, bool state);

	/*
	 *
	 */
	void on_setup_view_toggled(const Glib::ustring &column);

	/*
	 *
	 */
	void on_preferences();

	/*
	 *
	 */
	void on_config_subtitle_view_changed(const Glib::ustring &key, const Glib::ustring &value);

	/*
	 *
	 */
	void set_sensitive(const Glib::ustring &name, bool state);

	/*
	 *
	 */
	void dialog_configure_keyboard_shortcuts();
protected:
public:
	Statusbar* m_statusbar;
	Glib::RefPtr<Gtk::UIManager> m_refUIManager;
	Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
	std::map<Glib::ustring, sigc::connection> m_connections;
};

#endif//_MenuBar_h

