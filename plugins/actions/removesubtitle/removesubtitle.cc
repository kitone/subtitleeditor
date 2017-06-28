/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	https://kitone.github.io/subtitleeditor/
 *	https://github.com/kitone/subtitleeditor/
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

class DeleteSelectedSubtitlePlugin : public Action
{
public:

	DeleteSelectedSubtitlePlugin()
	{
		activate();
		update_ui();
	}

	~DeleteSelectedSubtitlePlugin()
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
		action_group = Gtk::ActionGroup::create("DeleteSelectedSubtitlePlugin");

		action_group->add(
				Gtk::Action::create("delete-selected-subtitles", Gtk::Stock::DELETE, "", _("Delete the selected subtitles")), Gtk::AccelKey("<Control>Delete"),
					sigc::mem_fun(*this, &DeleteSelectedSubtitlePlugin::on_delete_selected_subtitles));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-edit/delete-selected-subtitles", "delete-selected-subtitles", "delete-selected-subtitles");
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

		action_group->get_action("delete-selected-subtitles")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_delete_selected_subtitles()
	{
		se_debug(SE_DEBUG_PLUGINS);

		execute();
	}

	/*
	 *
	 */
	bool execute()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		g_return_val_if_fail(doc, false);

		Subtitles subtitles = doc->subtitles();

		std::vector<Subtitle> selection = subtitles.get_selection();

		if(selection.empty())
		{
			doc->flash_message(_("Please select at least a subtitle."));
			return false;
		}

		Subtitle previous_subtitle = subtitles.get_previous(selection[0]);

		doc->start_command(_("Delete Subtitles"));

		subtitles.remove(selection);

		if(!previous_subtitle)
			previous_subtitle = subtitles.get_first();
		if(previous_subtitle)
			subtitles.select(previous_subtitle);

		doc->finish_command();

		doc->flash_message(ngettext(
					"1 subtitle has been deleted.",
					"%d subtitles have been deleted.", 
					selection.size()), selection.size());

		return true;
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};


REGISTER_EXTENSION(DeleteSelectedSubtitlePlugin)



