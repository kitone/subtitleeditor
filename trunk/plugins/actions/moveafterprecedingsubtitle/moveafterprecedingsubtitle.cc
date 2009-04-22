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
class MoveAfterPrecedingSubtitlePlugin : public Action
{
public:

	MoveAfterPrecedingSubtitlePlugin()
	{
		activate();
		update_ui();
	}

	~MoveAfterPrecedingSubtitlePlugin()
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
		action_group = Gtk::ActionGroup::create("MoveAfterPrecedingSubtitlePlugin");

		action_group->add(
				Gtk::Action::create("move-after-preceding-subtitle", _("_Move After Preceding"), _("Move subtitle after the preceding with the respect of the minimum gap between subtitles")),
					sigc::mem_fun(*this, &MoveAfterPrecedingSubtitlePlugin::on_execute));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-timings/move-after-preceding-subtitle", "move-after-preceding-subtitle", "move-after-preceding-subtitle");
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

		action_group->get_action("move-after-preceding-subtitle")->set_sensitive(visible);
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

		SubtitleTime gap( get_config().get_value_int("timing", "min-gap-between-subtitles") );
		SubtitleTime min_display( get_config().get_value_int("timing", "min-display") );

		doc->start_command(_("Move After Preceding"));
		for(unsigned int i=0; i<selection.size(); ++i)
		{
			Subtitle sub = selection[i];

			Subtitle previous = subtitles.get_previous(sub);
			
			if(previous)
			{
				SubtitleTime previous_end = previous.get_end();
				SubtitleTime duration = sub.get_duration();
				if(duration.totalmsecs == 0)
					duration = min_display;

				sub.set_start(previous_end + gap);
				sub.set_duration(duration);
			}
		}
		doc->emit_signal("subtitle-time-changed");
		doc->finish_command();

		return true;
	}
	
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(MoveAfterPrecedingSubtitlePlugin)
