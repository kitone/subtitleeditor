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

class CombineSelectedSubtitlesPlugin : public Action
{
public:

	CombineSelectedSubtitlesPlugin()
	{
		activate();
		update_ui();
	}

	~CombineSelectedSubtitlesPlugin()
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
		action_group = Gtk::ActionGroup::create("CombineSelectedSubtitlesPlugin");

		action_group->add(
				Gtk::Action::create("combine-selected-subtitles", _("_Combine"), _("Merge the selected subtitles")),
					sigc::mem_fun(*this, &CombineSelectedSubtitlesPlugin::on_combine_selected_subtitles));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-edit/combine-selected-subtitles", "combine-selected-subtitles", "combine-selected-subtitles");
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

		action_group->get_action("combine-selected-subtitles")->set_sensitive(visible);
	}

protected:

	/*
	 */
	void on_combine_selected_subtitles()
	{
		se_debug(SE_DEBUG_PLUGINS);

		execute();
	}

	/*
	 * Merge a group of subtitles (text, translation and note)
	 * to the first and delete next subtitles.
	 */
	void combine(Document *doc, std::vector<Subtitle> &subs)
	{
		se_debug(SE_DEBUG_PLUGINS);

		if(subs.size() < 2)
			return;

		Glib::ustring text, translation, note;

		std::vector<Subtitle>::iterator it;

		for(it=subs.begin() ; it != subs.end(); ++it)
		{
			if(!text.empty())
				text += "\n";
			text += (*it).get_text();

			if(!translation.empty())
				translation += "\n";
			translation += (*it).get_translation();

			if(!note.empty())
				note += "\n";
			note += (*it).get_note();
		}

		Subtitle first = subs.front();
		Subtitle last = subs.back();

		first.set_text(text);
		first.set_translation(translation);
		first.set_note(note);
		first.set_end(last.get_end());

		// delete subtitles without the first
		std::vector<Subtitle> t(++subs.begin(), subs.end());
		
		doc->subtitles().remove(t);
	}

	/*
	 * Work only if there are at less two subtitles and if they follow.
	 */
	bool execute()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		g_return_val_if_fail(doc, false);

		Subtitles subtitles = doc->subtitles();

		std::vector<Subtitle> selection = subtitles.get_selection();
		if(selection.size() < 2)
		{
			doc->flash_message(_("Please select at least two subtitles."));
			return false;
		}

		doc->start_command(_("Combine subtitles"));

		// This structure is used to merge only the subtitles that follow.
		// We use the subtitle number to check if it's the next.

		std::list< std::vector<Subtitle> > subs;
	
		subs.push_back( std::vector<Subtitle> () );

		unsigned int last_id = 0;

		for(unsigned int i=0; i<selection.size(); ++i)
		{
			Subtitle sub = selection[i];

			// Is the next subtitle?
			if(sub.get_num() == last_id + 1)
			{
				subs.back().push_back( sub );

				++last_id;
			}
			else
			{
				// Create new list only if the previous is empty.
				if(!subs.back().empty())
					subs.push_back( std::vector<Subtitle> () );
			
				subs.back().push_back( sub );

				last_id = sub.get_num();
			}
		}

		// Merge subtitle from the end. This make your life easy.
		// this avoid to have an invalidate subtitle caused by the delete of one.

		while(!subs.empty())
		{
			combine(doc, subs.back());
			subs.pop_back();
		}

		doc->emit_signal("subtitle-time-changed");
		doc->finish_command();

		return true;
	}
	
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(CombineSelectedSubtitlesPlugin)
