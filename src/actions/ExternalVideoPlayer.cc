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
#include "gui/DialogFileChooser.h"
#include "SubtitleSystem.h"

/*
 *
 */
class ExternalVideoPlayer : public Plugin
{
public:

	/*
	 *
	 */
	void activate()
	{
		// actions
		action_group = Gtk::ActionGroup::create("ExternalVideoPlayer");

		action_group->add(
				Gtk::Action::create("menu-external-video-player", _("_Preview")));

		action_group->add(
				Gtk::Action::create("external-video-player/open-video", Gtk::Stock::OPEN, _("_Open Movie"), _("Open movie with external video player")), Gtk::AccelKey(""),
					sigc::mem_fun(*this, &ExternalVideoPlayer::on_open_movie));

		action_group->add(
				Gtk::Action::create("external-video-player/play-video", Gtk::Stock::MEDIA_PLAY, _("_Play Movie"), _("Play movie with external video player")), Gtk::AccelKey("<Control>space"),
					sigc::mem_fun(*this, &ExternalVideoPlayer::on_play_movie));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		//ui->add_ui(ui_id, "/menubar/menu-edit/extend-XX", "external-video-player/open-video", "external-video-player/open-video");
	}

	/*
	 *
	 */
	void deactivate()
	{
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->remove_ui(ui_id);
		ui->remove_action_group(action_group);
	}

protected:

	/*
	 *
	 */
	void on_open_movie()
	{
		DialogOpenVideo ui;
		if(ui.run() == Gtk::RESPONSE_OK)
			m_movie_uri = ui.get_uri();
	}


	/*
	 *
	 */
	void on_play_movie()
	{
		Document *doc = get_current_document();

		g_return_if_fail(doc);

		if(m_movie_uri.empty())
		{
			on_open_movie();
		}


		if(m_movie_uri.empty())
		{
			doc->flash_message(_("Please select a movie."));
			return;
		}

		// subtitle.[format]
		Glib::ustring tmp_subtitle_name = "subtitle." + SubtitleSystem::getInstance().get_extension(doc->getFormat());

		// tmp dir + subtitle name
		Glib::ustring subtitle_file = Glib::build_filename(Glib::get_tmp_dir(), tmp_subtitle_name);

		// save now tmp subtitle
		Glib::ustring old_filename = doc->getFilename();

		doc->save(subtitle_file);
		doc->setFilename(old_filename);
	
		long start_position = 0;

		std::vector<Subtitle> selection = doc->subtitles().get_selection();

		if(!selection.empty())
		{
			Subtitle sub = selection[0];
			if(sub)
			{
				SubtitleTime time = sub.get_start() - SubtitleTime(0,0,4,0);

				start_position = time.hours*3600 + time.mins*60 + time.secs;

				if(start_position < 0)
					start_position = 0;
			}
		}

		// command pipe...
		Glib::ustring cmd;

		// load the config or use the default command
		if(!Config::getInstance().get_value_string("external-video-player", "command", cmd))
		{
			Glib::ustring default_cmd = "mplayer \"#video_file\" -sub \"#subtitle_file\" -ss #seconds -osdlevel 2";
			Config::getInstance().set_value_string("external-video-player", "command", default_cmd);
			cmd = default_cmd;
		}
		// create the command
		{
			Glib::ustring video_uri = m_movie_uri;
			Glib::ustring video_file = Glib::filename_from_uri(video_uri);

			Glib::ustring seconds = to_string(start_position);
			
			Glib::ustring subtitle_uri = Glib::filename_to_uri(subtitle_file);
			
			find_and_replace(cmd, "#video_file", video_file);
			find_and_replace(cmd, "#video_uri", video_uri);
			find_and_replace(cmd, "#subtitle_file", subtitle_file);
			find_and_replace(cmd, "#subtitle_uri", subtitle_uri);
			find_and_replace(cmd, "#seconds", seconds);
		}

		std::cout << "COMMAND: " << cmd << std::endl;

		try
		{
			Glib::spawn_command_line_async(cmd);
		}
		catch(const Glib::Error &ex)
		{
			dialog_error(
					_("Failed to launch the external player."), 
					build_message(
						_("%s\n\nCommand: <i>%s</i>"),ex.what().c_str(), cmd.c_str())
					);
		}
	}

	
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;

	Glib::ustring m_movie_uri;
};

REGISTER_PLUGIN(ExternalVideoPlayer)