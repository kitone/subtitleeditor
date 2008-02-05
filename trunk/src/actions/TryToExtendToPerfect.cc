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

class TryToExtendToPerfectPlugin : public Plugin
{
public:

	/*
	 *
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("TryToExtendToPerfectPlugin");

		action_group->add(
				Gtk::Action::create("try-to-extend-to-perfect", _("_Try To Extend To Perfect"), _("Try to extend to perfect with the respect of timing preferences")),
					sigc::mem_fun(*this, &TryToExtendToPerfectPlugin::on_extend));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		//ui->add_ui(ui_id, "/menubar/menu-timings/extend-7", "try-to-extend-to-perfect", "try-to-extend-to-perfect");
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

		action_group->get_action("try-to-extend-to-perfect")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_extend()
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

#warning "TODO: with other preferences (overlapping, min-display, min-gab...)"
		Document *doc = get_current_document();

		g_return_val_if_fail(doc, false);

		std::vector<Subtitle> selection = doc->subtitles().get_selection();

		if(selection.empty())
		{
			doc->flash_message(_("Please select at least a subtitle."));
			return false;
		}

		int cps = get_config().get_value_int("timing", "max-characters-per-second");

		doc->start_command(_("Try to extend to perfect"));

		Subtitles subtitles = doc->subtitles();

		for(unsigned int i=0; i<selection.size(); ++i)
		{
			Subtitle sub = selection[i];

			int text_len = sub.get_text().size();
		
			long duration = (text_len * 1000) / cps;

			sub.set_duration(SubtitleTime(duration));
		}
		
		doc->emit_signal("subtitle-time-changed");
		doc->finish_command();

		return true;
	}
	
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_PLUGIN(TryToExtendToPerfectPlugin)
