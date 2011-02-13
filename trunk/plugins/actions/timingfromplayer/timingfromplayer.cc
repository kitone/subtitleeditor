/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2011, kitone
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
		// set current subtitle
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

		// set current subtitle and go to the next
		action_group->add(
				Gtk::Action::create(
					"timing-from-player/set-subtitle-start-and-go-next", 
					_("Set Subtitle Start And Go Next"), 
					_("Use the current player position to set the start of the selected subtitle and go to the next")), 
					sigc::mem_fun(*this, &TimingFromPlayer::set_subtitle_start_and_go_next));

		action_group->add(
				Gtk::Action::create(
					"timing-from-player/set-subtitle-end-and-go-next", 
					_("Set Subtitle End And Go Next"), 
					_("Use the current player position to set the end of the selected subtitle and go to the next")), 
					sigc::mem_fun(*this, &TimingFromPlayer::set_subtitle_end_and_go_next));

		// set current subtitle and define the next
		action_group->add(
				Gtk::Action::create(
					"timing-from-player/set-subtitle-start-and-next", 
					_("Set Subtitle Start And Next"), 
					_("Use the current player position to set the start of the current selected subtitle and the position of the next")), 
					sigc::mem_fun(*this, &TimingFromPlayer::set_subtitle_start_and_next));

		action_group->add(
				Gtk::Action::create(
					"timing-from-player/set-subtitle-end-and-next", 
					_("Set Subtitle End And Next"), 
					_("Use the current player position to set the end of the current selected subtitle and the position of the next")), 
					sigc::mem_fun(*this, &TimingFromPlayer::set_subtitle_end_and_next));

		// set current subtile start and and with one key
		action_group->add(
				Gtk::Action::create(
					"timing-from-player/set-subtitle-start-and-end-with-one-key", 
					_("Set Subtitle Start _And End"), 
					_("Use only one key to set beginning of the subtitle when the key "
						"is pressed and the end when the key is released.")), 
					sigc::mem_fun(*this, &TimingFromPlayer::set_subtitle_start_and_end_with_one_key));

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
			"					<separator />"
			"					<menuitem action='timing-from-player/set-subtitle-start-and-go-next'/>"
			"					<menuitem action='timing-from-player/set-subtitle-end-and-go-next'/>"
			"					<separator />"
			"					<menuitem action='timing-from-player/set-subtitle-start-and-next'/>"
			"					<menuitem action='timing-from-player/set-subtitle-end-and-next'/>"
			"					<separator />"
			"					<menuitem action='timing-from-player/set-subtitle-start-and-end-with-one-key'/>"
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

		SET_SENSITIVE("timing-from-player/set-subtitle-start-and-go-next", has_media && has_doc);
		SET_SENSITIVE("timing-from-player/set-subtitle-end-and-go-next", has_media && has_doc);

		SET_SENSITIVE("timing-from-player/set-subtitle-start-and-next", has_media && has_doc);
		SET_SENSITIVE("timing-from-player/set-subtitle-end-and-next", has_media && has_doc);

		SET_SENSITIVE("timing-from-player/set-subtitle-start-and-end-with-one-key", has_media && has_doc);

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
	 */
	enum OPTIONS {
		SET_SUBTITLE_START		= 1 << 0,
		SET_SUBTITLE_END			= 1 << 1,
		SELECT_NEXT_OR_CREATE	= 1 << 2,
		SET_NEXT_SUBTITLE_POS	= 1 << 3
	};

	/*
	 */
	Glib::ustring get_command_name_from_option(int op)
	{
		if(op & SET_SUBTITLE_START)
			return _("Set subtitle start");
		else if(op & SET_SUBTITLE_END)
			return _("Set subtitle end");
		return _("Set subtitle"); // should not have happened
	}

	/*
	 */
	bool set_subtitle_from_player(int op)
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();
		g_return_val_if_fail(doc, false);

		Subtitle sub = doc->subtitles().get_first_selected();
		if(!sub)
			return false;

		SubtitleTime pos = get_subtitleeditor_window()->get_player()->get_position();
		SubtitleTime dur = sub.get_duration();

		// Start recording
		doc->start_command(get_command_name_from_option(op));

		if(op & SET_SUBTITLE_START) // Define the start of the subtitle from the video position, we keep the duration
		{
			sub.set_start_and_end(pos, pos + dur);
		}
		else if (op & SET_SUBTITLE_END)
		{
			sub.set_end(pos);
		}

		// Select or create the next subtitle
		if(op & SELECT_NEXT_OR_CREATE)
		{
			Subtitle next = doc->subtitles().get_next(sub);
			if(!next)
			{
				next = doc->subtitles().append();
				next.set_duration(	get_config().get_value_int("timing", "min-display") );
			}
			if(op & SET_NEXT_SUBTITLE_POS)
			{
				SubtitleTime sub_end = sub.get_end();
				SubtitleTime gap( get_config().get_value_int("timing", "min-gap-between-subtitles") );
				next.set_start_and_end(sub_end + gap, sub_end + next.get_duration());
			}
			doc->subtitles().select(next);
		}

		doc->emit_signal("subtitle-time-changed");
		doc->finish_command();
		return true;
	}

	/*
	 */
	void set_subtitle_start()
	{
		set_subtitle_from_player(SET_SUBTITLE_START);
	}

	/*
	 */
	void set_subtitle_end()
	{
		set_subtitle_from_player(SET_SUBTITLE_END);
	}

	/*
	 */
	void set_subtitle_start_and_go_next()
	{
		set_subtitle_from_player(SET_SUBTITLE_START | SELECT_NEXT_OR_CREATE);
	}

	/*
	 */
	void set_subtitle_end_and_go_next()
	{
		set_subtitle_from_player(SET_SUBTITLE_END | SELECT_NEXT_OR_CREATE);
	}

	/*
	 */
	void set_subtitle_start_and_next()
	{
		set_subtitle_from_player(SET_SUBTITLE_START | SELECT_NEXT_OR_CREATE | SET_NEXT_SUBTITLE_POS);
	}

	/*
	 */
	void set_subtitle_end_and_next()
	{
		set_subtitle_from_player(SET_SUBTITLE_END | SELECT_NEXT_OR_CREATE | SET_NEXT_SUBTITLE_POS);
	}

	/*
	 * Update the subtitle start.
	 * We connect the signal key_release_event to update the 
	 * end of the subtitle when the key is released.
	 */
	void set_subtitle_start_and_end_with_one_key()
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

		set_subtitle_end_and_go_next();
		co.disconnect();
		return true;
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
	sigc::connection co;
};

REGISTER_EXTENSION(TimingFromPlayer)
