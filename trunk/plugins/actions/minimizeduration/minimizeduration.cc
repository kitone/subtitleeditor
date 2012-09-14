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
	 *
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("MinimizeDurationPlugin");

		action_group->add(
				Gtk::Action::create("minimize-duration", _("_Minimize Duration From Start"),
				_("Compact each selected subtitle to its minimum permissible duration, start time is unchanged.")),
					sigc::mem_fun(*this, &MinimizeDurationPlugin::on_minimize_duration_from_start));
		action_group->add(
				Gtk::Action::create("minimize-duration-from-end", _("M_inimize Duration From End"),
				_("Compact each selected subtitle to its minimum permissible duration, end time is unchanged.")),
					sigc::mem_fun(*this, &MinimizeDurationPlugin::on_minimize_duration_from_end));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-timings/minimize-duration", "minimize-duration", "minimize-duration");
		ui->add_ui(ui_id, "/menubar/menu-timings/minimize-duration-from-end", "minimize-duration-from-end", "minimize-duration-from-end");
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

		action_group->get_action("minimize-duration")->set_sensitive(visible);
		action_group->get_action("minimize-duration-from-end")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_minimize_duration_from_start()
	{
		se_debug(SE_DEBUG_PLUGINS);

		execute(true);
	}

	/*
	 *
	 */
	void on_minimize_duration_from_end()
	{
		se_debug(SE_DEBUG_PLUGINS);

		execute(false);
	}

	/*
	 *
	 */
	bool execute(bool from_start)
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		g_return_val_if_fail(doc, false);

		Subtitles subtitles = doc->subtitles();

		//NOTE:	the selection returned is always sorted regardless of the order the user clicked on the subtitles in
		//	or at least it was when I tried it.
		std::vector<Subtitle> selection = subtitles.get_selection();

		unsigned int subcnt = selection.size();

		if( subcnt < 1 )
		{
			doc->flash_message(_("Minimize Duration needs at least 1 subtitle to work on."));
			return false;
		}

		// get relevant preferences
		Config &cfg = get_config();

		SubtitleTime mindur = cfg.get_value_int("timing", "min-display");
		unsigned long maxcps = cfg.get_value_int("timing", "max-characters-per-second");

		doc->start_command(_("Minimize Durations"));

		Glib::ustring subtext = "";
		Subtitle *sub = (Subtitle *)NULL;

		//take each subtitle and set its duration to the permissible minimum
		unsigned long subchars = 0;
		SubtitleTime dur;

		for(unsigned int i=0; i < subcnt; ++i)
		{
			sub = &selection[i];
			subtext = sub->get_text();
			subchars = utility::get_text_length_for_timing( subtext );
			dur.totalmsecs = utility::get_min_duration_msecs( subchars, maxcps );
			//doc->flash_message ( _("duration calculated is 1000 * %i / %i = %i"), (int)subchars, (int)maxcps, (int)dur.totalmsecs );
			//make sure we have at least the minimum duration
			if( dur < mindur )
				dur = mindur;

			if( from_start )
			{
				//the start time is fixed, we are changind the end time
				sub->set_duration( dur );
			}
			else
			{
				//the end time is fixed, we are changing the start time
				SubtitleTime endtime = sub->get_end();
				sub->set_start_and_end( endtime-dur, endtime );
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

REGISTER_EXTENSION(MinimizeDurationPlugin)

