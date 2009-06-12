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

#include <extension/action.h>
#include <i18n.h>
#include <debug.h>
#include <documentsystem.h>

/*
 *
 */
class CommandPlugin : public Action
{
public:

	CommandPlugin()
	{
		activate();
		update_ui();
	}

	~CommandPlugin()
	{
		deactivate();
	}

	/*
	 *
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("CommandPlugin");

		action_group->add(
				Gtk::Action::create("undo-command", Gtk::Stock::UNDO, "", _("Undo the last action")), Gtk::AccelKey("<Control>Z"),
					sigc::mem_fun(*this, &CommandPlugin::on_undo_command));
		action_group->add(
				Gtk::Action::create("redo-command", Gtk::Stock::REDO, "", _("Redo the last undone action")), Gtk::AccelKey("<Shift><Control>Z"),
					sigc::mem_fun(*this, &CommandPlugin::on_redo_command));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->insert_action_group(action_group);

		Glib::ustring submenu = 
			"<ui>"
			"	<menubar name='menubar'>"
			"		<menu name='menu-edit' action='menu-edit'>"
			"			<placeholder name='command'>"
			"				<menuitem action='undo-command'/>"
			"				<menuitem action='redo-command'/>"
			"			</placeholder>"
			"		</menu>"
			"	</menubar>"
			"</ui>";

		ui_id = ui->add_ui_from_string(submenu);
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
	 *
	 */
	void update_ui()
	{
		se_debug(SE_DEBUG_PLUGINS);

		bool visible = (get_current_document() != NULL);

		action_group->get_action("undo-command")->set_sensitive(visible);
		action_group->get_action("redo-command")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void update_label()
	{
		/*
		Glib::RefPtr<Gtk::Action> undo = action_group->get_action("undo-command");
		Glib::RefPtr<Gtk::Action> redo = action_group->get_action("redo-command");

		Glib::ustring undo_description, redo_description;

		Document *doc = get_current_document();
		if(doc != NULL)
		{
			undo_description = doc->get_command_system().get_undo_description();
			redo_description = doc->get_command_system().get_redo_description();
		}

		undo->property_label() = build_message("Undo %s", undo_description.c_str());
		redo->property_label() = build_message("Redo %s", redo_description.c_str());
		*/

	}
	/*
	 *
	 */
	void on_undo_command()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		g_return_if_fail(doc);

		Glib::ustring description = doc->get_command_system().get_undo_description();

		se_debug_message(SE_DEBUG_PLUGINS, "description=%s", description.c_str());

		if(!description.empty())
		{
			doc->get_command_system().undo();
			doc->flash_message(_("Undo: %s"), description.c_str());
		}
		update_label();
	}

	/*
	 *
	 */
	void on_redo_command()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		g_return_if_fail(doc);

		Glib::ustring description = doc->get_command_system().get_redo_description();

		se_debug_message(SE_DEBUG_PLUGINS, "description=%s", description.c_str());

		if(!description.empty())
		{
			doc->get_command_system().redo();
			doc->flash_message(_("Redo: %s"), description.c_str());
		}
		update_label();
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(CommandPlugin)
