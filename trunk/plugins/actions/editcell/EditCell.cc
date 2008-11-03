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
 
#include <extension/Action.h>
#include <utility.h>

/*
 *
 */
class EditCellPlugin : public Action
{
public:

	EditCellPlugin()
	{
		activate();
		update_ui();
	}

	~EditCellPlugin()
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
		action_group = Gtk::ActionGroup::create("EditCellPlugin");

		action_group->add(
				Gtk::Action::create("edit-cell", Gtk::Stock::EDIT, _("_Edit Cell"), _("Start the editing of the focused cell")),
					sigc::mem_fun(*this, &EditCellPlugin::on_edit_cell));

		action_group->add(
				Gtk::Action::create("edit-next-cell", Gtk::Stock::EDIT, _("Edit _Next Cell"), _("Start the editing of the next cell")),
					sigc::mem_fun(*this, &EditCellPlugin::on_edit_next_cell));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->insert_action_group(action_group);

		Glib::ustring submenu = 
			"<ui>"
			"	<menubar name='menubar'>"
			"		<menu name='menu-edit' action='menu-edit'>"
			"			<placeholder name='edit-cell'>"
			"				<menuitem action='edit-cell'/>"
			"				<menuitem action='edit-next-cell'/>"
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

		action_group->get_action("edit-cell")->set_sensitive(visible);
		action_group->get_action("edit-next-cell")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_edit_cell()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		g_return_if_fail(doc);

		Subtitle sub = doc->subtitles().get_first_selected();
		if(sub)
			doc->subtitles().select(sub, true);
	}

	/*
	 *
	 */
	void on_edit_next_cell()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		g_return_if_fail(doc);

		Subtitle sub = doc->subtitles().get_first_selected();
		if(sub)
		{
			sub = doc->subtitles().get_next(sub);
			if(sub)
				doc->subtitles().select(sub, true);
		}
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(EditCellPlugin)
