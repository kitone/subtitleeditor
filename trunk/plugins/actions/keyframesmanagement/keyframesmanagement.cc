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
#include <utility.h>
#include <player.h>
#include <keyframes.h>
#include <gui/dialogfilechooser.h>

/*
 * declared in keyframesgenerator.cc
 */
Glib::RefPtr<KeyFrames> generate_keyframes_from_file(const Glib::ustring &uri);

/*
 */
class KeyframesManagementPlugin : public Action
{
public:

	KeyframesManagementPlugin()
	{
		activate();
		update_ui();
	}

	~KeyframesManagementPlugin()
	{
		deactivate();
	}

	/*
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("KeyframesManagementPlugin");

		// already in src/gui/menubar.cc
		//action_group->add(
		//		Gtk::Action::create(
		//			"menu-keyframes", 
		//			_("_KeyFrames")));

		// Open
		action_group->add(
				Gtk::Action::create(
					"keyframes/open", 
					Gtk::Stock::OPEN,
					_("Open Keyframes"), 
					_("Open a keyframe from file")),
					Gtk::AccelKey("<Control>K"),
					sigc::mem_fun(*this, &KeyframesManagementPlugin::on_open));
		// Save
		action_group->add(
				Gtk::Action::create(
					"keyframes/save", 
					Gtk::Stock::SAVE,
					_("Save Keyframes"), 
					_("Save a keyframe to the file")),
					Gtk::AccelKey("<Shift><Control>K"),
					sigc::mem_fun(*this, &KeyframesManagementPlugin::on_save));
		// Generate
		action_group->add(
				Gtk::Action::create(
					"keyframes/generate", 
					Gtk::Stock::EXECUTE,
					_("Generate Keyframes From Video"), 
					_("Generate a keyframe from the current video")),
					sigc::mem_fun(*this, &KeyframesManagementPlugin::on_generate));
		// Close
		action_group->add(
				Gtk::Action::create(
					"keyframes/close", 
					Gtk::Stock::CLOSE,
					_("Close the keyframe"), 
					_("FIXME")),
					Gtk::AccelKey("<Alt><Control>K"),
					sigc::mem_fun(*this, &KeyframesManagementPlugin::on_close));
		// Seek
		action_group->add(
				Gtk::Action::create(
					"keyframes/seek-to-previous", 
					Gtk::Stock::MEDIA_PREVIOUS,
					_("Seek To Previous Keyframe"), 
					_("FIXME")),
					sigc::mem_fun(*this, &KeyframesManagementPlugin::on_seek_previous));

		action_group->add(
				Gtk::Action::create(
					"keyframes/seek-to-next", 
					Gtk::Stock::MEDIA_NEXT,
					_("Seek To Next Keyframe"), 
					_("FIXME")),
					sigc::mem_fun(*this, &KeyframesManagementPlugin::on_seek_next));
		// Snap Start
		action_group->add(
				Gtk::Action::create(
					"keyframes/snap-start-to-previous", 
					Gtk::Stock::GOTO_FIRST,
					_("Snap Start To Previous Keyframe"), 
					_("FIXME")),
					sigc::mem_fun(*this, &KeyframesManagementPlugin::on_snap_start_to_previous));

		action_group->add(
				Gtk::Action::create(
					"keyframes/snap-start-to-next", 
					Gtk::Stock::GOTO_LAST,
					_("Snap Start To Next Keyframe"), 
					_("FIXME")),
					sigc::mem_fun(*this, &KeyframesManagementPlugin::on_snap_start_to_next));
		// Snap End
		action_group->add(
				Gtk::Action::create(
					"keyframes/snap-end-to-previous", 
					Gtk::Stock::GOTO_FIRST,
					_("Snap End To Previous Keyframe"), 
					_("FIXME")),
					sigc::mem_fun(*this, &KeyframesManagementPlugin::on_snap_end_to_previous));

		action_group->add(
				Gtk::Action::create(
					"keyframes/snap-end-to-next", 
					Gtk::Stock::GOTO_LAST,
					_("Snap End To Next Keyframe"), 
					_("FIXME")),
					sigc::mem_fun(*this, &KeyframesManagementPlugin::on_snap_end_to_next));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		Glib::ustring submenu = 
			"<ui>"
			"	<menubar name='menubar'>"
			"		<menu name='menu-keyframes' action='menu-keyframes'>"
			"			<placeholder name='placeholder'>"
			"					<menuitem action='keyframes/open'/>"
			"					<menuitem action='keyframes/save'/>"
			"					<menuitem action='keyframes/generate'/>"
			"					<menuitem action='keyframes/close'/>"
			"					<separator/>"
			"					<menuitem action='keyframes/seek-to-previous'/>"
			"					<menuitem action='keyframes/seek-to-next'/>"
			"					<separator/>"
			"					<menuitem action='keyframes/snap-start-to-previous'/>"
			"					<menuitem action='keyframes/snap-start-to-next'/>"
			"					<menuitem action='keyframes/snap-end-to-previous'/>"
			"					<menuitem action='keyframes/snap-end-to-next'/>"
			"			</placeholder>"
			"		</menu>"
			"	</menubar>"
			"</ui>";

		ui_id = ui->add_ui_from_string(submenu);

		// connect the player state signals
		player()->signal_state_changed().connect(
				sigc::mem_fun(*this, &KeyframesManagementPlugin::on_player_state_changed));
		player()->signal_keyframes_changed().connect(
				sigc::mem_fun(*this, &KeyframesManagementPlugin::update_ui));
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
	void on_player_state_changed(Player::State state)
	{
		// only if the player is enable or disable
		// don't update if is playing or paused
		if(state == Player::NONE || state == Player::READY)
			update_ui();
	}

	/*
	 */
	void update_ui()
	{
		se_debug(SE_DEBUG_PLUGINS);
		
		bool has_doc = (get_current_document() != NULL);
		bool has_kf = (player()->get_keyframes());
		bool has_media = player()->get_state() != Player::NONE;

#define SET_SENSITIVE(action, state) { Glib::RefPtr<Gtk::Action> act = action_group->get_action(action); if(act) act->set_sensitive(state); else g_warning(action); }

		SET_SENSITIVE("keyframes/save", has_kf);
		SET_SENSITIVE("keyframes/close", has_kf);
		SET_SENSITIVE("keyframes/generate", has_media);
		// Update state from keyframes and player
		SET_SENSITIVE("keyframes/seek-to-previous", has_kf && has_media);
		SET_SENSITIVE("keyframes/seek-to-next", has_kf && has_media);
		// Update state from document and keyframes
		SET_SENSITIVE("keyframes/snap-start-to-previous", has_doc && has_kf);
		SET_SENSITIVE("keyframes/snap-start-to-next", has_doc && has_kf);
		SET_SENSITIVE("keyframes/snap-end-to-previous", has_doc && has_kf);
		SET_SENSITIVE("keyframes/snap-end-to-next", has_doc && has_kf);
		
#undef SET_SENSITIVE
	}
protected:

	/*
	 */
	Player* player()
	{
		return get_subtitleeditor_window()->get_player();
	}

	/*
	 */
	void on_open()
	{
		DialogOpenKeyframe ui;
		if(ui.run() == Gtk::RESPONSE_OK)
		{
			ui.hide();
			Glib::RefPtr<KeyFrames> kf = KeyFrames::create_from_file(ui.get_uri());
			if(!kf)
				kf = generate_keyframes_from_file(ui.get_uri());

			if(kf)
				player()->set_keyframes(kf);
		}
	}

	/*
	 */
	void on_save()
	{
		Glib::RefPtr<KeyFrames> kf = player()->get_keyframes();
		if(kf)
		{
			Gtk::FileChooserDialog ui(_("Save Keyframes"), Gtk::FILE_CHOOSER_ACTION_SAVE);
			ui.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			ui.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
			ui.set_default_response(Gtk::RESPONSE_OK);

			if(ui.run() == Gtk::RESPONSE_OK)
			{
				Glib::ustring uri = ui.get_uri();

				// FIXME check return value
				kf->save(uri);
			}
		}
	}

	/*
	 */
	void on_generate()
	{
		Glib::ustring uri = get_subtitleeditor_window()->get_player()->get_uri();
		if(uri.empty())
			return;

		Glib::RefPtr<KeyFrames> kf = generate_keyframes_from_file(uri);
		if(kf)
			player()->set_keyframes(kf);
	}

	/*
	 */
	void on_close()
	{
		player()->set_keyframes(Glib::RefPtr<KeyFrames>(NULL));
	}

	/*
	 */
	void on_seek_next()
	{
		Glib::RefPtr<KeyFrames> keyframes = player()->get_keyframes();
		g_return_if_fail(keyframes);

		long pos = player()->get_position();

		for(KeyFrames::const_iterator it = keyframes->begin(); it != keyframes->end(); ++it)
		{
			if(*it > pos)
			{
				player()->seek(*it);
				return;
			}
		}
	}

	/*
	 */
	void on_seek_previous()
	{
		Glib::RefPtr<KeyFrames> keyframes = player()->get_keyframes();
		g_return_if_fail(keyframes);

		long pos = player()->get_position();

		for(KeyFrames::const_reverse_iterator it = keyframes->rbegin(); it != keyframes->rend(); ++it)
		{
			if(*it < pos)
			{
				player()->seek(*it);
				return;
			}
		}
	}

	/*
	 */
	bool get_previous_keyframe(const long pos, long &prev)
	{
		Glib::RefPtr<KeyFrames> keyframes = player()->get_keyframes();
		if(!keyframes)
			return false;

		for(KeyFrames::const_reverse_iterator it = keyframes->rbegin(); it != keyframes->rend(); ++it)
		{
			if(*it < pos)
			{
				prev = *it;
				return true;
			}
		}
		return false;
	}

	/*
	 */
	bool get_next_keyframe(const long pos, long &next)
	{
		Glib::RefPtr<KeyFrames> keyframes = player()->get_keyframes();
		if(!keyframes)
			return false;

		for(KeyFrames::const_iterator it = keyframes->begin(); it != keyframes->end(); ++it)
		{
			if(*it > pos)
			{
				next = *it;
				return true;
			}
		}
		return false;
	}

	/*
	 */
	bool snap_start_to_keyframe(bool previous)
	{
		Document* doc = get_current_document();
		g_return_val_if_fail(doc, false);

		Subtitle sub = doc->subtitles().get_first_selected();
		g_return_val_if_fail(sub, false);

		long pos = sub.get_start().totalmsecs;
		long kf = 0;
		bool ret = (previous) ? get_previous_keyframe(pos, kf) : get_next_keyframe(pos, kf);
		if(!ret)
			return false;

		doc->start_command(_("Snap Start to Keyframe"));
		sub.set_start(SubtitleTime(kf));
		doc->emit_signal("subtitle-time-changed");
		doc->finish_command();
		return true;
	}

	/*
	 */
	bool snap_end_to_keyframe(bool previous)
	{
		Document* doc = get_current_document();
		g_return_val_if_fail(doc, false);

		Subtitle sub = doc->subtitles().get_first_selected();
		g_return_val_if_fail(sub, false);

		long pos = sub.get_end().totalmsecs;
		long kf = 0;
		bool ret = (previous) ? get_previous_keyframe(pos, kf) : get_next_keyframe(pos, kf);
		if(!ret)
			return false;
		doc->start_command(_("Snap End to Keyframe"));
		sub.set_end(SubtitleTime(kf));
		doc->emit_signal("subtitle-time-changed");
		doc->finish_command();
		return true;
	}

	/*
	 */
	void on_snap_start_to_previous()
	{
		snap_start_to_keyframe(true);
	}

	/*
	 */
	void on_snap_start_to_next()
	{
		snap_start_to_keyframe(false);
	}

	/*
	 */
	void on_snap_end_to_previous()
	{
		snap_end_to_keyframe(true);
	}

	/*
	 */
	void on_snap_end_to_next()
	{
		snap_end_to_keyframe(false);
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(KeyframesManagementPlugin)
