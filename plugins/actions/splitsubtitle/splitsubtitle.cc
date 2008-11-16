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
 
#include <extension/action.h>
#include <i18n.h>
#include <debug.h>

class SplitSelectedSubtitlesPlugin : public Action
{
public:

	SplitSelectedSubtitlesPlugin()
	{
		activate();
		update_ui();
	}

	~SplitSelectedSubtitlesPlugin()
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
		action_group = Gtk::ActionGroup::create("SplitSelectedSubtitlesPlugin");

		action_group->add(
				Gtk::Action::create("split-selected-subtitles", _("_Split"), _("Split the selected subtitles")),
					sigc::mem_fun(*this, &SplitSelectedSubtitlesPlugin::on_split_selected_subtitles));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-edit/split-selected-subtitles", "split-selected-subtitles", "split-selected-subtitles");
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

		action_group->get_action("split-selected-subtitles")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_split_selected_subtitles()
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

		if(selection.size() < 1)
		{
			doc->flash_message(_("Please select at least two subtitles."));
			return false;
		}

		// respect minimum gap between subtitles
		int min_gap_between_subtitles = get_config().get_value_int("timing", "min-gap-between-subtitles");

		SubtitleTime gap(min_gap_between_subtitles / 2);

		// utilisé pour couper en deux
		Glib::RefPtr<Glib::Regex> re = Glib::Regex::create("^(.*?)\\n(.*?)$");

		//
		doc->start_command(_("Split subtitles"));

		for(std::vector<Subtitle>::reverse_iterator it=selection.rbegin(); it != selection.rend(); ++it)
		{
			Subtitle sub = (*it);		

			Subtitle next = subtitles.insert_after(sub);

			sub.copy_to(next);

			// centre la coupe avec le respect du minimum entre les s-t si possible
			SubtitleTime middle = sub.get_start() + sub.get_duration() * 0.5;
	
			// prev
			if((middle - gap) > sub.get_start())
				sub.set_end(middle - gap);
			else
				sub.set_end(middle);

			// next
			if((middle + gap) < next.get_end())
				next.set_start(middle + gap);
			else
				next.set_start(middle);

			// s'il y a deux lignes alors on coupe le texte en deux 
			{
				if(re->match(sub.get_text()))
				{
					std::vector<Glib::ustring> group = re->split(sub.get_text());
					sub.set_text(group[1]);
					next.set_text(group[2]);
				}
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

REGISTER_EXTENSION(SplitSelectedSubtitlesPlugin)
