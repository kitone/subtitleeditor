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

/*
 *
 */
class DuplicateSelectedSubtitlesPlugin : public Action
{
public:

	DuplicateSelectedSubtitlesPlugin()
	{
		activate();
		update_ui();
	}

	~DuplicateSelectedSubtitlesPlugin()
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
		action_group = Gtk::ActionGroup::create("DuplicateSelectedSubtitlesPlugin");

		action_group->add(
				Gtk::Action::create("duplicate-selected-subtitles", _("_Duplicate"), _("Duplicate the selected subtitles")),
					sigc::mem_fun(*this, &DuplicateSelectedSubtitlesPlugin::on_execute));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-edit/duplicate-selected-subtitles", "duplicate-selected-subtitles", "duplicate-selected-subtitles");
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

		action_group->get_action("duplicate-selected-subtitles")->set_sensitive(visible);
	}

protected:

	void on_execute()
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

		doc->start_command(_("Duplicate selected subtitles"));
	
		std::vector<Subtitle>::reverse_iterator it;
		for(it = selection.rbegin(); it != selection.rend(); ++it) 
		{
			Subtitle next = subtitles.insert_after(*it);
			(*it).copy_to(next);
		}

		doc->emit_signal("subtitle-time-changed");
		doc->finish_command();

		return true;
	}
	
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(DuplicateSelectedSubtitlesPlugin)
