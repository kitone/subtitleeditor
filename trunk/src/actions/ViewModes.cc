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
#include "Document.h"
#include "Plugin.h"
#include "utility.h"

class ViewModesPlugin : public Plugin
{
public:
	
	/*
	 * First check if the user has any preferences
	 */
	void check_config()
	{
		std::list<Glib::ustring> keys;

		if(get_config().get_keys("setting-view", keys) && !keys.empty())
				return;

		Config &cfg = get_config();

		cfg.set_value_string("setting-view", _("Simple"), "number;start;end;duration;text");
		cfg.set_value_string("setting-view", _("Advanced"), "number;start;end;duration;style;name;text");
		cfg.set_value_string("setting-view", _("Translation"), "number;text;translation");
		cfg.set_value_string("setting-view", _("Timing"), "number;start;end;duration;cps;text");
	}

	/*
	 *
	 */
	void activate()
	{
		check_config();

		action_group = Gtk::ActionGroup::create("ViewModesPlugin");

		std::list<Glib::ustring> keys;

		get_config().get_keys("setting-view", keys);

		std::list<Glib::ustring>::const_iterator it;

		for(it = keys.begin(); it != keys.end(); ++it)
		{
			Glib::ustring name = *it;

			action_group->add(
				Gtk::Action::create(name, name),
					sigc::bind( sigc::mem_fun(*this, &ViewModesPlugin::on_set_view), name));
				
		}

		get_ui_manager()->insert_action_group(action_group);
	}

	/*
	 *
	 */
	void post_activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		std::list<Glib::ustring> keys;

		if(get_config().get_keys("setting-view", keys))
		{
			std::list<Glib::ustring>::reverse_iterator it;

			for(it = keys.rbegin(); it != keys.rend(); ++it)
			{
				Glib::ustring name = *it;
				
				ui->add_ui(ui_id, "/menubar/menu-view/setting-view-placeholder", name, name);
			}
		}
	}

	/*
	 *
	 */
	void deactivate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->remove_ui(ui_id);
		ui->remove_action_group(action_group);
	}

	/*
	 * Updates the configuration with the columns to display.
	 */
	void on_set_view(const Glib::ustring &name)
	{
		Glib::ustring columns = get_config().get_value_string("setting-view", name);

		get_config().set_value_string("subtitle-view", "columns", columns);
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_PLUGIN(ViewModesPlugin)
