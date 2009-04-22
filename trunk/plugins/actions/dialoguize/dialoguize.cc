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
class DialoguizeSelectedSubtitlesPlugin : public Action
{
public:

	DialoguizeSelectedSubtitlesPlugin()
	{
		activate();
		update_ui();
	}

	~DialoguizeSelectedSubtitlesPlugin()
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
		action_group = Gtk::ActionGroup::create("DialoguizeSelectedSubtitlesPlugin");

		action_group->add(
				Gtk::Action::create("dialoguize-selected-subtitles", _("_Dialogue"), _("Add or remove dialogue line")), Gtk::AccelKey("D"),
					sigc::mem_fun(*this, &DialoguizeSelectedSubtitlesPlugin::on_execute));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->insert_action_group(action_group);

		Glib::ustring submenu = 
			"<ui>"
			"	<menubar name='menubar'>"
			"		<menu name='menu-edit' action='menu-edit'>"
			"			<placeholder name='text-formatting'>"
			"				<menuitem action='dialoguize-selected-subtitles'/>"
			"			</placeholder>"
			"		</menu>"
			"	</menubar>"
			"</ui>";

		ui_id = ui->add_ui_from_string(submenu);
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

		action_group->get_action("dialoguize-selected-subtitles")->set_sensitive(visible);
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

		doc->start_command(_("Italic"));
	
		bool state = parial_match(selection, "^[^-\\s*]");

		global_replace(selection, "^-\\s*", "");

		if(state)
			global_replace(selection, "^", "- ");
		
		doc->finish_command();

		return true;
	}
	
	/*
	 *
	 */
	bool parial_match(std::vector<Subtitle> &subs, const std::string &pattern)
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Glib::Regex> re = Glib::Regex::create(pattern);

		for(unsigned int i=0; i<subs.size(); ++i)
		{
			if(re->match(subs[i].get_text()))
				return true;
		}
		
		return false;
	}

	/*
	 *
	 */
	void global_replace(std::vector<Subtitle> &subs, const std::string &pattern, const std::string &replace)
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Glib::Regex> re = Glib::Regex::create(pattern, Glib::REGEX_MULTILINE);

		for(unsigned int i=0; i<subs.size(); ++i)
		{
			Subtitle sub = subs[i];

			Glib::ustring text = sub.get_text();

			text = re->replace(text, 0, replace, (Glib::RegexMatchFlags)0);

			sub.set_text(text);
		}
	}
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(DialoguizeSelectedSubtitlesPlugin)
