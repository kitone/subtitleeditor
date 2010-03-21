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
#include <player.h>

/*
 * Actions to set time from the current player position.
 */
class TimingFromPlayer : public Action
{
public:
	
	TimingFromPlayer()
	{
		activate();
		update_ui();
	}

	~TimingFromPlayer()
	{
		deactivate();
	}

	/*
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("TimingFromPlayer");

		// Set Subtitle Time
		action_group->add(
				Gtk::Action::create(
					"menu-timing-from-player", 
					_("Timing From Player"), 
					_("Use the current player position to set subtitle time")));

		action_group->add(
				Gtk::Action::create(
					"timing-from-player/set-subtitle-start", 
					_("Set Subtitle _Start"), 
					_("Use the current player position to set the subtitle start")), 
					sigc::mem_fun(*this, &TimingFromPlayer::set_subtitle_start));

		action_group->add(
				Gtk::Action::create(
					"timing-from-player/set-subtitle-end", 
					_("Set Subtitle _End"), 
					_("Use the current player position to set the subtitle end")), 
					sigc::mem_fun(*this, &TimingFromPlayer::set_subtitle_end));

		action_group->add(
				Gtk::Action::create(
					"timing-from-player/set-subtitle-start-and-end", 
					_("Set Subtitle Start _And End"), 
					_("Use only one key to set beginning of the subtitle when the key "
						"is pressed and the end when the key is released.")), 
					sigc::mem_fun(*this, &TimingFromPlayer::set_subtitle_start_and_end));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->insert_action_group(action_group);

		Glib::ustring submenu = 
			"<ui>"
			"	<menubar name='menubar'>"
			"		<menu name='menu-video' action='menu-video'>"
			"			<placeholder name='placeholder'>"
			"				<menu action='menu-timing-from-player'>"
			"					<menuitem action='timing-from-player/set-subtitle-start'/>"
			"					<menuitem action='timing-from-player/set-subtitle-end'/>"
			"					<menuitem action='timing-from-player/set-subtitle-start-and-end'/>"
			"				</menu>"
			"			</placeholder>"
			"		</menu>"
			"	</menubar>"
			"</ui>";

		ui_id = ui->add_ui_from_string(submenu);

		// Connect to the player state changed
		// the actions can only be used when the player has a media
		get_subtitleeditor_window()->get_player()->signal_message().connect(
				sigc::mem_fun(*this, &TimingFromPlayer::on_player_message));
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

		bool has_doc = (get_current_document() != NULL);
		bool has_media = get_subtitleeditor_window()->get_player()->get_state() != Player::NONE;

#define SET_SENSITIVE(action, state) { Glib::RefPtr<Gtk::Action> act = action_group->get_action(action); if(act) act->set_sensitive(state); else g_warning(action); }

		SET_SENSITIVE("timing-from-player/set-subtitle-start", has_media && has_doc);
		SET_SENSITIVE("timing-from-player/set-subtitle-end", has_media && has_doc);
		SET_SENSITIVE("timing-from-player/set-subtitle-start-and-end", has_media && has_doc);

#undef SET_SENSITIVE
	}

	/*
	 * Check the state of the player. 
	 * Update the menu from the current state of the player.
	 */
	void on_player_message(Player::Message msg)
	{
		se_debug(SE_DEBUG_PLUGINS);
		// only if the player is enable or disable
		// don't update if is playing or paused
		if(msg == Player::STATE_NONE || msg == Player::STREAM_READY)
			update_ui();
	}

	/*
	 * Sets the begining of the selected subtitle at the current position of the player.
	 */
	void set_subtitle_start()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();
		g_return_if_fail(doc);

		Subtitle sub = doc->subtitles().get_first_selected();
		if(sub)
		{
			long position = get_subtitleeditor_window()->get_player()->get_position();

			doc->start_command(_("Set subtitle start"));

			sub.set_start(SubtitleTime(position));

			doc->emit_signal("subtitle-time-changed");
			doc->finish_command();
		}
	}

	/*
	 * Sets the end of the selected subtitle at the current position of the player.
	 * The next subtitle is selected or created after that.
	 */
	void set_subtitle_end()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();
		g_return_if_fail(doc);

		Subtitle sub = doc->subtitles().get_first_selected();
		if(sub)
		{
			long position = get_subtitleeditor_window()->get_player()->get_position();

			doc->start_command(_("Set subtitle end"));

			sub.set_end(SubtitleTime(position));

			// try to select the next subtitle
			// TODO option for enable/disable this ?
			{
				Subtitle next = doc->subtitles().get_next(sub);
				if(!next)
				{
					next = doc->subtitles().append();
				}
				doc->subtitles().select(next);
			}

			doc->emit_signal("subtitle-time-changed");
			doc->finish_command();
		}
	}

	/*
	 * Update the subtitle start.
	 * We connect the signal key_release_event to update the 
	 * end of the subtitle when the key is released.
	 */
	void set_subtitle_start_and_end()
	{
		se_debug(SE_DEBUG_PLUGINS);

		if(co)
			return;

		Document *doc = get_current_document();
		g_return_if_fail(doc);

		// connect the keyboard release signal to the main window 
		SubtitleEditorWindow* win = get_subtitleeditor_window();
		Gtk::Window *gtk_win = dynamic_cast<Gtk::Window*>(win);
		Glib::RefPtr<Gdk::Window> gdk_win = gtk_win->get_window();

		co = gtk_win->signal_key_release_event().connect(
				sigc::mem_fun(*this, &TimingFromPlayer::on_key_release_event), false);

		set_subtitle_start();
	}

	/*
	 * Any key have been released. 
	 * Update the end of the subtitle and disconnect
	 * the callback.
	 */
	bool on_key_release_event(GdkEventKey *ev)
	{
		se_debug(SE_DEBUG_PLUGINS);

		set_subtitle_end();
		co.disconnect();
		return true;
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
	sigc::connection co;
};

REGISTER_EXTENSION(TimingFromPlayer)
