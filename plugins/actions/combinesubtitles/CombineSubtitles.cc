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
 
#include <extension/Action.h>
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

		action_group->get_action("combine-selected-subtitles")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_combine_selected_subtitles()
	{
		se_debug(SE_DEBUG_PLUGINS);

		execute();
	}

	/*
	 *	Fusionne un groupe de s-t (text & translation)
	 *	puis efface les s-t suivant
	 */
	void combine(Document *doc, std::vector<Subtitle> &subs)
	{
		se_debug(SE_DEBUG_PLUGINS);

		if(subs.size() < 2)
			return;

		Glib::ustring text, translation;

		std::vector<Subtitle>::iterator it;

		for(it=subs.begin() ; it != subs.end(); ++it)
		{
			//if(!sub.get_text().empty())
			{
				if(!text.empty())
					text += "\n";
				text += (*it).get_text();
			}

			//if(!sub.get_translation().empty())
			{
				if(!translation.empty())
					translation += "\n";

				translation += (*it).get_translation();
			}
		}

		Subtitle first = subs.front();
		Subtitle last = subs.back();

		first.set_text(text);
		first.set_translation(translation);
		first.set_end(last.get_end());

		// efface tous sauf le premiers
		std::vector<Subtitle> t(++subs.begin(), subs.end());
		
		doc->subtitles().remove(t);
	}

	/*
	 *	Ne fonctionne que s'il y a plus d'un sous-titre dans la sélection
	 *	est seulement si ils se suivent.
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

		//	Cette structure nous permet de fusionner 
		//	seulement les sous-titres qui se suivent
		//	le numéro du sous-titre est utiliser pour le teste.

		std::list< std::vector<Subtitle> > subs;
	
		subs.push_back( std::vector<Subtitle> () );

		unsigned int last_id = 0;

		for(unsigned int i=0; i<selection.size(); ++i)
		{
			Subtitle sub = selection[i];

			// Est-ce le sous-titre suivant ?
			if(sub.get_num() == last_id + 1)
			{
				subs.back().push_back( sub );

				++last_id;
			}
			else
			{
				// On crée une nouvelle list seulement si la précédante est vide.
				if(!subs.back().empty())
					subs.push_back( std::vector<Subtitle> () );
			
				subs.back().push_back( sub );

				last_id = sub.get_num();
			}
		}

		//	fusion des sous-titres par la fin
		//	dans un souci de simplicité.
		//	Car après chaque fusion on éfface les sous-titres suivant
		//	il faut donc partir de la fin pour ne pas rendre invalide les 
		//	sous-titre (GtkTreeIter) dans le model.

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
