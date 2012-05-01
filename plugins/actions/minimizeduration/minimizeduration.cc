/*
 *
 *	minimizeduration.cc
 *	- "the ultimate subtitle fitness center"
 *	a subtitleeditor plugin by Eltomito <tomaspartl@centrum.cz>
 *
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2012, kitone
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
#include <utility.h>

class MinimizeDurationPlugin : public Action
{
public:

	MinimizeDurationPlugin()
	{
		activate();
		update_ui();
	}

	~MinimizeDurationPlugin()
	{
		deactivate();
	}

	/*
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("MinimizeDurationPlugin");

		action_group->add(
				Gtk::Action::create("minimize-duration", _("_Minimize Duration"),
				_("Compact each selected subtitle to its minimum permissible duration.")),
					sigc::mem_fun(*this, &MinimizeDurationPlugin::on_minimize_duration));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-timings/minimize-duration", "minimize-duration", "minimize-duration");
	}

	/*
	 */
	void deactivate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->remove_ui(ui_id);
		ui->remove_action_group(action_group);
	}

	/*
	 */
	void update_ui()
	{
		se_debug(SE_DEBUG_PLUGINS);

		bool visible = (get_current_document() != NULL);

		action_group->get_action("minimize-duration")->set_sensitive(visible);
	}

protected:


	/*
	 */
	void on_minimize_duration()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		g_return_if_fail(doc);

		Subtitles subtitles = doc->subtitles();

		std::vector<Subtitle> selection = subtitles.get_selection();

		if( selection.size() < 1 )
		{
			doc->flash_message(_("Minimize Duration needs at least 1 subtitle to work on."));
			return;
		}

		// Get relevant preferences
		Config &cfg = get_config();

		SubtitleTime mindur = cfg.get_value_int("timing", "min-display");
		unsigned long maxcps = cfg.get_value_int("timing", "max-characters-per-second");

		// Take each subtitle and set its duration to the permissible minimum
		unsigned long subchars = 0;
		SubtitleTime dur;

		doc->start_command(_("Minimize Durations"));

		for(guint i=0; i< selection.size(); ++i)
		{
			Subtitle &sub = selection[i];
			//subchars = utility::get_text_length_for_timing( sub->get_text() );

			dur.totalmsecs = utility::get_min_duration_msecs( sub.get_text(), maxcps );

			//make sure we have at least the minimum duration
			if( dur < mindur )
				dur = mindur;
			sub.set_duration( dur );
		}

		doc->emit_signal("subtitle-time-changed");
		doc->finish_command();
	}
	
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(MinimizeDurationPlugin)

