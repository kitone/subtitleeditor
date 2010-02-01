/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2010, kitone
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
 */
class SortSubtitlesPlugin : public Action
{
public:

	SortSubtitlesPlugin()
	{
		activate();
		update_ui();
	}

	~SortSubtitlesPlugin()
	{
		deactivate();
	}

	/*
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("SortSubtitlesPlugin");

		action_group->add(
				Gtk::Action::create("sort-subtitles", Gtk::Stock::SORT_ASCENDING, 
					_("S_ort Subtitles"), _("Sort subtitles based on their start time")), 
					sigc::mem_fun(*this, &SortSubtitlesPlugin::sort_subtitles));
		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();
		ui_id = ui->new_merge_id();
		ui->insert_action_group(action_group);
		ui->add_ui(ui_id, "/menubar/menu-timings/placeholder", "sort-subtitles", "sort-subtitles");
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
		action_group->get_action("sort-subtitles")->set_sensitive(visible);
	}

protected:

	/*
	 * There is some gym there because inserting or removing subtitle 
	 * make that the iterator turn to an invalid state.
	 *
	 * We check if the next subtitle is superior with the current.
	 * If not, we move the next subtitle in the good place.
	 */
	void sort_subtitles()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();
		g_return_if_fail(doc);

		Subtitles subtitles = doc->subtitles();
		if(subtitles.size() < 2)
			return;

		guint number_of_sub_reorder = 0;

		Subtitle current = subtitles.get_first();
		Subtitle next = current; ++next;
		while(next)
		{
			if(next.get_start() < current.get_start())
			{
				// We start to record the command only once and only when we need it.
				if(number_of_sub_reorder == 0)
					doc->start_command("Sort Subtitle");

				++number_of_sub_reorder;

				// We need to keep an index of the current position. We cannot used the 
				// subtitle directly because it will come invalid when we add the 
				// new one at the good position and when we remove the old one. 
				// Then we can continue from the index to check the rest of subtitles. 
				guint orig_next_id = next.get_num();

				// We try to find (backwards) the good position of the subtitle (next).
				Subtitle previous=current;
				while(previous && !(previous.get_start() < next.get_start()))
				{
					previous = subtitles.get_previous(previous);
				}

				// Like before we need to keep the position of the subtitle before creating the new one.
				guint index = next.get_num();
				// Two cases, we have found a subtitle or the position found is to go to the first place.
				Subtitle cp = (previous) ? subtitles.insert_after(previous) : subtitles.insert_before(subtitles.get_first());

				// Copy the original subtitle to the new one and remove it.
				next = subtitles.get(index+1);
				next.copy_to(cp);
				subtitles.remove(next);

				// We need to reset the subtitle next to the good position. (See previous comments)
				next = subtitles.get(orig_next_id);
			}
			// Go to the next subtitle
			current = next;
			++next;
		}

		if(number_of_sub_reorder > 0)
		{
			doc->finish_command();
			doc->emit_signal("subtitle-time-changed");
		
			doc->flash_message( ngettext(
					"1 subtitle has been reordered.",
					"%d subtitles have been reordered.", 
					number_of_sub_reorder), number_of_sub_reorder);
		}
		else
			doc->flash_message(_("No need to sort subtitles."));
	}
	
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(SortSubtitlesPlugin)
