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
#include <utility.h>

/*
 *
 */
class AdjustTimePlugin : public Action
{
public:
	
	AdjustTimePlugin()
	{
		activate();
		update_ui();
	}

	~AdjustTimePlugin()
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
		action_group = Gtk::ActionGroup::create("AdjustTimePlugin");

		// menu add
		action_group->add(
				Gtk::Action::create("menu-adjust-time-add", Gtk::Stock::ADD, _("Add 100 Milliseconds")));

		action_group->add(
				Gtk::Action::create("add-to-start", _("To Start"), _("Add 100 Milliseconds to start for all subtitles selected")),
					sigc::mem_fun(*this, &AdjustTimePlugin::on_add_to_start));
		
		action_group->add(
				Gtk::Action::create("add-to-duration", _("To Duration"), _("Add 100 Milliseconds to duration for all subtitles selected")),
					sigc::mem_fun(*this, &AdjustTimePlugin::on_add_to_duration));

		action_group->add(
				Gtk::Action::create("add-to-start-and-duration", _("To Start And Duration"), _("Add 100 Milliseconds to all subtitles selected")),
					sigc::mem_fun(*this, &AdjustTimePlugin::on_add_to_start_and_duration));

		// menu remove
		action_group->add(
				Gtk::Action::create("menu-adjust-time-remove", Gtk::Stock::REMOVE, _("Remove 100 Milliseconds")));

		action_group->add(
				Gtk::Action::create("remove-to-start", _("To Start"), _("Remove 100 Milliseconds to start for all subtitles selected")),
					sigc::mem_fun(*this, &AdjustTimePlugin::on_remove_to_start));
		
		action_group->add(
				Gtk::Action::create("remove-to-duration", _("To Duration"), _("Remove 100 Milliseconds to duration for all subtitles selected")),
					sigc::mem_fun(*this, &AdjustTimePlugin::on_remove_to_duration));

		action_group->add(
				Gtk::Action::create("remove-to-start-and-duration", _("To Start And Duration"), _("Remove 100 Milliseconds to all subtitles selected")),
					sigc::mem_fun(*this, &AdjustTimePlugin::on_remove_to_start_and_duration));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->insert_action_group(action_group);

		Glib::ustring submenu = 
			"<ui>"
			"	<menubar name='menubar'>"
			"		<menu name='menu-timings' action='menu-timings'>"
			"			<placeholder name='adjust-time'>"
			"				<menu action='menu-adjust-time-add'>"
			"					<menuitem action='add-to-start'/>"
			"					<menuitem action='add-to-duration'/>"
			"					<menuitem action='add-to-start-and-duration'/>"
			"				</menu>"
			"				<menu action='menu-adjust-time-remove'>"
			"					<menuitem action='remove-to-start'/>"
			"					<menuitem action='remove-to-duration'/>"
			"					<menuitem action='remove-to-start-and-duration'/>"
			"				</menu>"
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

		action_group->get_action("menu-adjust-time-add")->set_sensitive(visible);
		action_group->get_action("menu-adjust-time-remove")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_add_to_start()
	{
		se_debug(SE_DEBUG_PLUGINS);

		adjust(START, 100);
	}

	/*
	 *
	 */
	void on_add_to_duration()
	{
		se_debug(SE_DEBUG_PLUGINS);

		adjust(END, 100);
	}

	/*
	 *
	 */
	void on_add_to_start_and_duration()
	{
		se_debug(SE_DEBUG_PLUGINS);

		adjust(START_AND_END, 100);
	}

	/*
	 *
	 */
	void on_remove_to_start()
	{
		se_debug(SE_DEBUG_PLUGINS);

		adjust(START, -100);
	}

	/*
	 *
	 */
	void on_remove_to_duration()
	{
		se_debug(SE_DEBUG_PLUGINS);

		adjust(END, -100);
	}

	/*
	 *
	 */
	void on_remove_to_start_and_duration()
	{
		se_debug(SE_DEBUG_PLUGINS);

		adjust(START_AND_END, -100);
	}


	enum TYPE
	{
		START,
		END,
		START_AND_END
	};

	/*
	 *
	 */
	bool adjust(TYPE type, const long &time_msecs = 100)
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();
		
		g_return_val_if_fail(doc, false);

		Subtitles subtitles = doc->subtitles();
	
		std::vector<Subtitle> selection = subtitles.get_selection();

		if(selection.size() == 0)
		{
			doc->flash_message(_("Please select at least a subtitle."));
			return false;
		}

		doc->start_command(_("Adjust time"));

		if(type == START)
		{
			for(unsigned int i=0; i<selection.size(); ++i)
			{
				Subtitle subtitle = selection[i];

				subtitle.set_start( 
						SubtitleTime( subtitle.get_start().totalmsecs + time_msecs));
			}
		}
		else if(type == END)
		{
			for(unsigned int i=0; i<selection.size(); ++i)
			{
				Subtitle subtitle = selection[i];

				subtitle.set_end( 
						SubtitleTime( subtitle.get_end().totalmsecs + time_msecs));
			}
		}	
		else if(type == START_AND_END)
		{
			for(unsigned int i=0; i<selection.size(); ++i)
			{
				Subtitle subtitle = selection[i];

				subtitle.set_start_and_end( 
						SubtitleTime( subtitle.get_start().totalmsecs + time_msecs),
						SubtitleTime( subtitle.get_end().totalmsecs + time_msecs));
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

REGISTER_EXTENSION(AdjustTimePlugin)
