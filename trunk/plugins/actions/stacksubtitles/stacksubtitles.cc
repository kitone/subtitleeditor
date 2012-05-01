/*
 *
 *	stacksubtitles.cc
 *	- "Stack selected subtitles as close together as possible"
 *	a subtitleeditor plugin by Eltomito <tomaspartl@centrum.cz>
 *
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2012 kitone
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

class StackSubtitlesPlugin : public Action
{
public:

	StackSubtitlesPlugin()
	{
		activate();
		update_ui();
	}

	~StackSubtitlesPlugin()
	{
		deactivate();
	}

	/*
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("StackSubtitlesPlugin");

		action_group->add(
				Gtk::Action::create("stack-subtitles", _("Stack Subtitles"),
				_("Stack selected subtitles as close together as possible.")),
					sigc::mem_fun(*this, &StackSubtitlesPlugin::on_stack_subtitles));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-timings/stack-subtitles", "stack-subtitles", "stack-subtitles");
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

		action_group->get_action("stack-subtitles")->set_sensitive(visible);
	}

protected:

	/*
	 */
	void on_stack_subtitles()
	{
		se_debug(SE_DEBUG_PLUGINS);

		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();
		g_return_if_fail(doc);

		// Check if the selection is contiguous and complain if it isn't.
		// Can work on multiple contiguous subtitles
		std::list< std::vector<Subtitle> > contiguous_selection;
		if(get_contiguous_selection(contiguous_selection) == false)
			return;

		doc->start_command(_("Stack Subtitles"));

		for(std::list< std::vector<Subtitle> >::iterator it = contiguous_selection.begin(); it != contiguous_selection.end(); ++it)
		{
			stacksubtitles(*it);
		}

		doc->emit_signal("subtitle-time-changed");
		doc->finish_command();
	}

	/*
	 */
	void stacksubtitles(std::vector<Subtitle> &subtitles)
	{
		if(subtitles.size() < 2)
			return;

		// Get relevant preferences
		Config &cfg = get_config();

		SubtitleTime gap = cfg.get_value_int("timing", "min-gap-between-subtitles");
		SubtitleTime endtime;

		// Take each subtitle and snap it after the one before.
		for(guint i=1; i< subtitles.size(); ++i)
		{
			Subtitle &sub = subtitles[i];

			endtime = subtitles[i-1].get_end() + gap;

			sub.set_start_and_end(
					endtime,
					endtime + sub.get_duration());
		}
	}

	/*
	 */
	bool get_contiguous_selection(std::list< std::vector<Subtitle> > &contiguous_selection)
	{
		Document* doc = get_current_document();

		std::vector<Subtitle> selection = doc->subtitles().get_selection();
		if(selection.size() < 2)
		{
			doc->flash_message(_("Stack Subtitles needs at least 2 subtitles to work on."));
			return false;
		}

		contiguous_selection.push_back( std::vector<Subtitle> () );

		guint last_id = 0;

		for(guint i=0; i<selection.size(); ++i)
		{
			Subtitle &sub = selection[i];
			// Is the next subtitle?
			if(sub.get_num() == last_id + 1)
			{
				contiguous_selection.back().push_back( sub );
				++last_id;
			}
			else
			{
				// Create new list only if the previous is empty.
				if(!contiguous_selection.back().empty())
					contiguous_selection.push_back( std::vector<Subtitle> () );

				contiguous_selection.back().push_back( sub );

				last_id = sub.get_num();
			}
		}

		// We check if we have at least one contiguous subtitles.
		for(std::list< std::vector<Subtitle> >::iterator it = contiguous_selection.begin(); it != contiguous_selection.end(); ++it)
		{
			if((*it).size() >= 2)
				return true;
		}
		doc->flash_message(_("Stack Subtitles only works on a continuous selection."));
		return false;
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(StackSubtitlesPlugin)

