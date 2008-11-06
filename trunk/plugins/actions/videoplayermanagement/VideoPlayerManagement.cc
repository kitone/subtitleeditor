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

#include <utility.h>
#include <extension/Action.h>
#include <gui/DialogFileChooser.h>


/*
 * Video Player Management
 */
class VideoPlayerManagement : public Action
{
public:

	VideoPlayerManagement()
	{
		activate();
		update_ui();
	}

	~VideoPlayerManagement()
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
		action_group = Gtk::ActionGroup::create("VideoPlayerManagement");

		// Already create in MenuBar.cc
		/*
		action_group->add(
				Gtk::Action::create(
					"menu-video", 
					_("_Video")));
		*/

		action_group->add(
				Gtk::Action::create(
					"video-player/open", 
					Gtk::Stock::OPEN,
					"", //_("_Open Media"),
					_("Open a multimedia file")), 
					Gtk::AccelKey("<Shift><Control>M"),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_open));

		action_group->add(
				Gtk::Action::create(
					"video-player/close", 
					Gtk::Stock::CLOSE,
					"", //_("_Close Media"),
					_("Close a multimedia file")), 
					Gtk::AccelKey("<Shift><Control>C"),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_close));

		action_group->add(
				Gtk::Action::create(
					"video-player/play", 
					Gtk::Stock::MEDIA_PLAY),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_play));

		action_group->add(
				Gtk::Action::create(
					"video-player/pause", 
					Gtk::Stock::MEDIA_PAUSE),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_pause));
	
		action_group->add(
				Gtk::Action::create(
					"video-player/play-pause", 
					_("_Play / Pause"), 
					_("Play or make a pause")),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_play_pause));

		// Seek Backwards
		action_group->add(
				Gtk::Action::create(
					"video-player/menu-skip-backwards", 
					Gtk::Stock::MEDIA_REWIND,
					_("Skip _Backwards")));

		action_group->add(
				Gtk::Action::create(
					"video-player/skip-backwards-very-short", 
					_("Very Short"), 
					_("FIXME")), // FIXME
					sigc::bind( sigc::mem_fun(*this, &VideoPlayerManagement::on_skip_backwards), VERY_SHORT));

		action_group->add(
				Gtk::Action::create(
					"video-player/skip-backwards-short", 
					_("Short"), 
					_("FIXME")), // FIXME
					sigc::bind( sigc::mem_fun(*this, &VideoPlayerManagement::on_skip_backwards), SHORT));

		action_group->add(
				Gtk::Action::create(
					"video-player/skip-backwards-medium", 
					_("Medium"), 
					_("FIXME")), // FIXME
					sigc::bind( sigc::mem_fun(*this, &VideoPlayerManagement::on_skip_backwards), MEDIUM));

		action_group->add(
				Gtk::Action::create(
					"video-player/skip-backwards-long", 
					_("Long"), 
					_("FIXME")), // FIXME
					sigc::bind( sigc::mem_fun(*this, &VideoPlayerManagement::on_skip_backwards), LONG));

		// Seek Forward
		action_group->add(
				Gtk::Action::create(
					"video-player/menu-skip-forward", 
					Gtk::Stock::MEDIA_FORWARD,
					_("Skip _Forward")));

		action_group->add(
				Gtk::Action::create(
					"video-player/skip-forward-very-short", 
					_("Very Short"), 
					_("FIXME")), // FIXME
					sigc::bind( sigc::mem_fun(*this, &VideoPlayerManagement::on_skip_forward), VERY_SHORT));

		action_group->add(
				Gtk::Action::create(
					"video-player/skip-forward-short", 
					_("Short"), 
					_("FIXME")), // FIXME
					sigc::bind( sigc::mem_fun(*this, &VideoPlayerManagement::on_skip_forward), SHORT));

		action_group->add(
				Gtk::Action::create(
					"video-player/skip-forward-medium", 
					_("Medium"), 
					_("FIXME")), // FIXME
					sigc::bind( sigc::mem_fun(*this, &VideoPlayerManagement::on_skip_forward), MEDIUM));

		action_group->add(
				Gtk::Action::create(
					"video-player/skip-forward-long", 
					_("Long"), 
					_("FIXME")), // FIXME
					sigc::bind( sigc::mem_fun(*this, &VideoPlayerManagement::on_skip_forward), LONG));

		// Rate Slower & Faster
		action_group->add(
				Gtk::Action::create(
					"video-player/menu-rate",
					_("Rate"),
					_("Define the playback rate")));

		action_group->add(
				Gtk::Action::create(
					"video-player/rate-slower", 
					_("_Slower"), 
					_("Define the playback rate")),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_playback_rate_slower));

		action_group->add(
				Gtk::Action::create(
					"video-player/rate-faster", 
					_("_Faster"), 
					_("Define the playback rate")),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_playback_rate_faster));

		action_group->add(
				Gtk::Action::create(
					"video-player/rate-normal", 
					_("_Normal"), 
					_("Define the playback rate")),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_playback_rate_normal));

		// Seek to Selection
		action_group->add(
				Gtk::Action::create(
					"video-player/seek-to-selection", 
					_("_Seek To Selection"), 
					_("Seek to the first selected subtitle")),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_seek_to_selection));

		// Repeat
		bool video_repeat_state = get_config().get_value_bool("video-player", "repeat");
		
		action_group->add(
				Gtk::ToggleAction::create(
					"video-player/repeat", 
					_("_Repeat"), 
					_("FIXME"), video_repeat_state), // FIXME
					sigc::mem_fun(*this, &VideoPlayerManagement::on_video_player_repeat_toggled));


		// Play Subtitle
		action_group->add(
				Gtk::Action::create(
					"video-player/play-previous-subtitle", 
					Gtk::Stock::MEDIA_PREVIOUS, 
					_("Play _Previous Subtitle"), 
					_("Play previous subtitle from the first selected subtitle")),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_play_previous_subtitle));

		action_group->add(
				Gtk::Action::create(
					"video-player/play-current-subtitle", 
					Gtk::Stock::MEDIA_PLAY, 
					_("Play _Selection"), 
					_("Play the selected subtitle")),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_play_current_subtitle));

		action_group->add(
				Gtk::Action::create(
					"video-player/play-next-subtitle", 
					Gtk::Stock::MEDIA_NEXT, 
					_("Play _Next Subtitle"), 
					_("Play next subtitle from the first selected subtitle")),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_play_next_subtitle));

		// Play Second
		action_group->add(
				Gtk::Action::create(
					"video-player/play-previous-second", 
					_("Play Previous Second"), 
					_("Play the second preceding the first selected subtitle")), 
					sigc::mem_fun(*this, &VideoPlayerManagement::on_play_previous_second));

		action_group->add(
				Gtk::Action::create(
					"video-player/play-first-second", 
					_("Play First Second"), 
					_("Play the first second of the subtitle currently selected")),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_play_first_second));

		action_group->add(
				Gtk::Action::create(
					"video-player/play-last-second", 
					_("Play Last Second"), 
					_("Play the last second of the subtitle currently selected")),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_play_last_second));

		action_group->add(
				Gtk::Action::create(
					"video-player/play-next-second", 
					_("Play Next Second"), 
					_("Play the second following the subtitle currently selected")),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_play_next_second));


		// Set Subtitle Time
		action_group->add(
				Gtk::Action::create(
					"video-player/set-subtitle-start", 
					_("Set Subtitle _Start"), 
					_("FIXME")),// FIXME
					sigc::mem_fun(*this, &VideoPlayerManagement::on_set_subtitle_start));

		action_group->add(
				Gtk::Action::create(
					"video-player/set-subtitle-end", 
					_("Set Subtitle _End"), 
					_("FIXME")),// FIXME
					sigc::mem_fun(*this, &VideoPlayerManagement::on_set_subtitle_end));

		// Display Video Player
		bool video_player_display_state = get_config().get_value_bool("video-player", "display");
		
		action_group->add(
				Gtk::ToggleAction::create(
					"video-player/display", 
					_("_Video Player"), 
					_("Show or hide the video player in the current window"), video_player_display_state),
					sigc::mem_fun(*this, &VideoPlayerManagement::on_video_player_display_toggled));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->insert_action_group(action_group);

		Glib::ustring submenu = 
			"<ui>"
			"	<menubar name='menubar'>"
			"		<menu name='menu-video' action='menu-video'>"
			"			<placeholder name='video-player-management'>"
			"					<menuitem action='video-player/open'/>"
			"					<menuitem action='video-player/close'/>"
			"					<separator/>"
			"					<menuitem action='video-player/play'/>"
			"					<menuitem action='video-player/pause'/>"
			"					<menuitem action='video-player/play-pause'/>"
			"					<separator/>"
			"					<menu action='video-player/menu-skip-forward'>"
			"						<menuitem action='video-player/skip-forward-very-short'/>"
			"						<menuitem action='video-player/skip-forward-short'/>"
			"						<menuitem action='video-player/skip-forward-medium'/>"
			"						<menuitem action='video-player/skip-forward-long'/>"
			"					</menu>"
			"					<menu action='video-player/menu-skip-backwards'>"
			"						<menuitem action='video-player/skip-backwards-very-short'/>"
			"						<menuitem action='video-player/skip-backwards-short'/>"
			"						<menuitem action='video-player/skip-backwards-medium'/>"
			"						<menuitem action='video-player/skip-backwards-long'/>"
			"					</menu>"
			"					<menu action='video-player/menu-rate'>"
			"						<menuitem action='video-player/rate-slower'/>"
			"						<menuitem action='video-player/rate-faster'/>"
			"						<menuitem action='video-player/rate-normal'/>"
			"					</menu>"
			"					<separator/>"
			"					<menuitem action='video-player/seek-to-selection'/>"
			"					<separator/>"
			"					<menuitem action='video-player/play-current-subtitle'/>"
			"					<menuitem action='video-player/play-next-subtitle'/>"
			"					<menuitem action='video-player/play-previous-subtitle'/>"
			"					<menuitem action='video-player/repeat'/>"
			"					<separator/>"
			"					<menuitem action='video-player/play-previous-second'/>"
			"					<menuitem action='video-player/play-first-second'/>"
			"					<menuitem action='video-player/play-last-second'/>"
			"					<menuitem action='video-player/play-next-second'/>"
			"					<separator/>"
			"					<menuitem action='video-player/set-subtitle-start'/>"
			"					<menuitem action='video-player/set-subtitle-end'/>"
			"			</placeholder>"
			"		</menu>"
			"	</menubar>"
			"</ui>";

		ui_id = ui->add_ui_from_string(submenu);

		// Show/Hide video player
		ui->add_ui(ui_id, "/menubar/menu-view/display-placeholder",
				"video-player/display", "video-player/display");

		// 
		player()->signal_state_changed().connect(
				sigc::mem_fun(*this, &VideoPlayerManagement::on_player_state_changed));

		get_config().signal_changed("video-player").connect(
				sigc::mem_fun(*this, &VideoPlayerManagement::on_config_video_player_changed));
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
	 * Update the user interface with the state of subtitle (has document)
	 * and the state of the player (has media)
	 */
	void update_ui()
	{
		se_debug(SE_DEBUG_PLUGINS);

		bool has_doc = (get_current_document() != NULL);
		bool has_media = player()->get_state() != Player::NONE;

#define SET_SENSITIVE(action, state) { Glib::RefPtr<Gtk::Action> act = action_group->get_action(action); if(act) act->set_sensitive(state); else g_warning(action); }

		SET_SENSITIVE("video-player/play", has_media);
		SET_SENSITIVE("video-player/close", has_media);
		SET_SENSITIVE("video-player/pause", has_media);
		SET_SENSITIVE("video-player/play-pause", has_media);

		SET_SENSITIVE("video-player/rate-slower", has_media);
		SET_SENSITIVE("video-player/rate-faster", has_media);
		SET_SENSITIVE("video-player/rate-normal", has_media);

		SET_SENSITIVE("video-player/skip-forward-very-short", has_media);
		SET_SENSITIVE("video-player/skip-forward-short", has_media);
		SET_SENSITIVE("video-player/skip-forward-medium", has_media);
		SET_SENSITIVE("video-player/skip-forward-long", has_media);

		SET_SENSITIVE("video-player/skip-backwards-very-short", has_media);
		SET_SENSITIVE("video-player/skip-backwards-short", has_media);
		SET_SENSITIVE("video-player/skip-backwards-medium", has_media);
		SET_SENSITIVE("video-player/skip-backwards-long", has_media);

		SET_SENSITIVE("video-player/repeat", has_media);

		SET_SENSITIVE("video-player/seek-to-selection", has_media && has_doc);
		
		SET_SENSITIVE("video-player/play-previous-subtitle", has_media && has_doc);
		SET_SENSITIVE("video-player/play-current-subtitle", has_media && has_doc);
		SET_SENSITIVE("video-player/play-next-subtitle", has_media && has_doc);
		
		SET_SENSITIVE("video-player/play-previous-second", has_media && has_doc);
		SET_SENSITIVE("video-player/play-first-second", has_media && has_doc);
		SET_SENSITIVE("video-player/play-last-second", has_media && has_doc);
		SET_SENSITIVE("video-player/play-next-second", has_media && has_doc);

		SET_SENSITIVE("video-player/set-subtitle-start", has_media && has_doc);
		SET_SENSITIVE("video-player/set-subtitle-end", has_media && has_doc);
		
#undef SET_SENSITIVE
	}

	/*
	 * Check the state of the player. 
	 * Display the video player if need and update the menu.
	 */
	void on_player_state_changed(Player::State state)
	{
		// only if the player is enable or disable
		// don't update if is playing or paused
		if(state == Player::NONE || state == Player::READY)
		{
			update_ui();

			if(state == Player::READY)
				if(get_config().get_value_bool("video-player", "display") == false)
					get_config().set_value_bool("video-player", "display", true);
		}
	}

	/*
	 * Show or hide the video player, update hte config.
	 */
	void on_video_player_display_toggled()
	{
		Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(action_group->get_action("video-player/display"));
		if(action)
		{
			bool state = action->get_active();
			if(get_config().get_value_bool("video-player", "display") != state)
				get_config().set_value_bool("video-player", "display", state);
		}
	}

	/*
	 * The state of reapet has changed, update the config.
	 */
	void on_video_player_repeat_toggled()
	{
		Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(action_group->get_action("video-player/repeat"));
		if(action)
		{
			bool state = action->get_active();
			if(get_config().get_value_bool("video-player", "repeat") != state)
				get_config().set_value_bool("video-player", "repeat", state);
		}
	}

	/*
	 * The video player config has changed.
	 * Update the menu.
	 */
	void on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value)
	{
		if(key == "display")
		{
			bool state = utility::string_to_bool(value);
			
			Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(action_group->get_action("video-player/display"));
			if(action)
			{
				if(action->get_active() != state)
					action->set_active(state);
			}
		}
		else if(key == "repeat")
		{
			bool state = utility::string_to_bool(value);
			
			Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(action_group->get_action("video-player/repeat"));
			if(action)
			{
				if(action->get_active() != state)
					action->set_active(state);
			}
		}
	}
protected:

	/*
	 * Return the GStreamer Player.
	 */
	Player* player()
	{
		return get_subtitleeditor_window()->get_player();
	}

	/*
	 * Open the dialog "Open Video" and initialize the player with the new uri.
	 */
	void on_open()
	{
		DialogOpenVideo ui;
		
		if(ui.run() == Gtk::RESPONSE_OK)
		{
			ui.hide();

			Glib::ustring uri = ui.get_uri();

			player()->open(uri);
		}
	}

	/*
	 * Close the player
	 */
	void on_close()
	{
		player()->close();
	}

	/*
	 * Reinitialize the current position for disable the repeat method
	 * and sets the player state to playing.
	 */
	void on_play()
	{
		player()->seek(player()->get_position());
		player()->play();
	}

	/*
	 * Sets the player state to paused.
	 */
	void on_pause()
	{
		player()->pause();
	}

	/*
	 * Toggled the player state.
	 * Paused to playing or playing to paused.
	 */
	void on_play_pause()
	{
		if(player()->is_playing())
			player()->pause();
		else
			player()->play();
	}

	/*
	 * Skip type, look the config for the value of the time.
	 */
	enum SkipType
	{
		VERY_SHORT,
		SHORT,
		MEDIUM,
		LONG
	};

	/*
	 * Make a skip backwards depending on the type.
	 */
	void on_skip_backwards(SkipType skip)
	{
		int value = 0;

		Glib::ustring key;

		if(skip == VERY_SHORT)
			key = "skip-very-short";
		else if(skip == SHORT)
			key = "skip-short";
		else if(skip == MEDIUM)
			key = "skip-medium";
		else if(skip == LONG)
			key = "skip-long";

		value = get_config().get_value_int("video-player", key);

		long newpos = player()->get_position() - SubtitleTime(0, 0, value, 0).totalmsecs;

		player()->seek(newpos); // FIXME faster = true
	}

	/*
	 * make a skip forward depending on the type.
	 */
	void on_skip_forward(SkipType skip)
	{
		int value = 0;

		Glib::ustring key;

		if(skip == VERY_SHORT)
			key = "skip-very-short";
		else if(skip == SHORT)
			key = "skip-short";
		else if(skip == MEDIUM)
			key = "skip-medium";
		else if(skip == LONG)
			key = "skip-long";

		value = get_config().get_value_int("video-player", key);

		long newpos = player()->get_position() + SubtitleTime(0, 0, value, 0).totalmsecs;

		player()->seek(newpos); // FIXME faster = true
	}

	/*
	 * Increase the playback rate.
	 */
	void on_playback_rate_faster()
	{
		double rate = player()->get_playback_rate();

		rate += 0.1;

		player()->set_playback_rate(rate);
	}

	/*
	 * Decreases the playback rate.
	 */
	void on_playback_rate_slower()
	{
		double rate = player()->get_playback_rate();

		rate -= 0.1;

		player()->set_playback_rate(rate);
	}

	/*
	 * Sets the playback rate to 1.0 (default).
	 */
	void on_playback_rate_normal()
	{
		player()->set_playback_rate(1.0);
	}

	/*
	 * Seek to the first selected subtitle. 
	 * The state of the player isn't modified.
	 */
	void on_seek_to_selection()
	{
		Document *doc = get_current_document();
		
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
		{
			player()->seek(selected.get_start().totalmsecs);
		}
	}

	/*
	 * Method for playing subtitle.
	 */

	/*
	 * Select and play the previous subtitle.
	 * Repeat is supported.
	 */
	void on_play_previous_subtitle()
	{
		Document *doc = get_current_document();
		
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
		{
			Subtitle previous = doc->subtitles().get_previous(selected);
			if(previous)
			{
				doc->subtitles().select(previous);
				player()->play_subtitle(previous);
			}
		}
	}

	/*
	 * Play the current subtitle.
	 * Repeat is supported.
	 */
	void on_play_current_subtitle()
	{
		Document *doc = get_current_document();
		
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
		{
			player()->play_subtitle(selected);
		}
	}

	/*
	 * Select and play the next subtitle.
	 * Repeat is supported.
	 */
	void on_play_next_subtitle()
	{
		Document *doc = get_current_document();
		
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
		{
			Subtitle next = doc->subtitles().get_next(selected);
			if(next)
			{
				doc->subtitles().select(next);
				player()->play_subtitle(next);
			}
		}
	}

	/*
	 * Method for playing second.
	 */

	/*
	 * Play the second preceding the first selected subtitle.
	 */
	void on_play_previous_second()
	{
		Document *doc = get_current_document();
		
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
		{
			SubtitleTime start = selected.get_start() - SubtitleTime(0,0,1,0);
			SubtitleTime end = selected.get_start();

			player()->play_segment(start, end);
		}
	}

	/*
	 * Play the first second of the subtitle currently selected.
	 */
	void on_play_first_second()
	{
		Document *doc = get_current_document();
		
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
		{
			SubtitleTime start = selected.get_start();
			SubtitleTime end = selected.get_start() + SubtitleTime(0,0,1,0);

			player()->play_segment(start, end);
		}
	}

	/*
	 * Play the last second of the subtitle currently selected
	 */
	void on_play_last_second()
	{
		Document *doc = get_current_document();
		
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
		{
			SubtitleTime start = selected.get_end() - SubtitleTime(0,0,1,0);
			SubtitleTime end = selected.get_end();

			player()->play_segment(start, end);
		}
	}

	/*
	 * Play the second following the subtitle currently selected
	 */
	void on_play_next_second()
	{
		Document *doc = get_current_document();
		
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
		{
			SubtitleTime start = selected.get_end();
			SubtitleTime end = selected.get_end() + SubtitleTime(0,0,1,0);

			player()->play_segment(start, end);
		}
	}

	/*
	 * Set Subtitle Time
	 */

	/*
	 * Sets the begining of the selected subtitle at the current position of the player.
	 */
	void on_set_subtitle_start()
	{
		Document *doc = get_current_document();
		
		Subtitle sub = doc->subtitles().get_first_selected();
		if(sub)
		{
			long position = player()->get_position();

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
	void on_set_subtitle_end()
	{
		Document *doc = get_current_document();
		
		Subtitle sub = doc->subtitles().get_first_selected();
		if(sub)
		{
			long position = player()->get_position();

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
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};


REGISTER_EXTENSION(VideoPlayerManagement)
