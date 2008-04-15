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

#include "VideoPlayer.h"
#include "utility.h"
#include "DocumentSystem.h"
#include "SubtitleEditorWindow.h"

/*
 * Player Controls Widgets
 */
class PlayerControls : public Gtk::HBox
{
public:

	PlayerControls(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)//GStreamerPlayer *player)
	:Gtk::HBox(cobject)
	{
		m_current_seek = false;

		m_player = NULL;

		refGlade->get_widget("button-play", m_button_play);
		refGlade->get_widget("button-pause", m_button_pause);

		refGlade->get_widget("hscale-seek", m_hscale_seek);

		// hscale
		Gtk::Adjustment *adj = manage(new Gtk::Adjustment(0, 0, 1, 0.1, 1.0, 1.0));

		m_hscale_seek->set_adjustment(*adj);
		m_hscale_seek->set_digits(2);
		m_hscale_seek->set_update_policy(Gtk::UPDATE_CONTINUOUS);
		m_hscale_seek->set_value_pos(Gtk::POS_RIGHT);

		m_hscale_seek->signal_format_value().connect(
				sigc::mem_fun(*this, &PlayerControls::position_to_string));

		m_hscale_seek->signal_event().connect(
				sigc::mem_fun(*this, &PlayerControls::on_seek_event), true);

		// signal play/pause directly connect to the player
		m_button_play->signal_clicked().connect(
			sigc::mem_fun(*this, &PlayerControls::on_play));

		m_button_pause->signal_clicked().connect(
			sigc::mem_fun(*this, &PlayerControls::on_pause));

		show_all();
			
		m_button_pause->hide();

		set_sensitive(false);
	}

	/*
	 *
	 */
	void on_play()
	{
		m_player->play();
	}

	/*
	 *
	 */
	void on_pause()
	{
		m_player->pause();
	}

	/*
	 *
	 */
	void init_with_player(Player *player)
	{
		m_player = player;

		m_player->signal_state_changed().connect(
				sigc::mem_fun(*this, &PlayerControls::on_player_state_changed));

		m_player->signal_timeout().connect(
				sigc::mem_fun(*this, &PlayerControls::on_timeout));
	}

	/*
	 *
	 */
	void on_player_state_changed(Player::State state)
	{
		if(state == Player::NONE)
		{
		}
		else if(state == Player::READY)
		{
			long position = m_player->get_position();
			long duration = m_player->get_duration();

			m_hscale_seek->set_range(0, duration);
			m_hscale_seek->set_value(position);

			m_button_play->show();
			m_button_pause->hide();
		}
		else if(state == Player::PAUSED)
		{
			m_button_play->show();
			m_button_pause->hide();
		}
		else if(state == Player::PLAYING)
		{
			m_button_pause->show();
			m_button_play->hide();
		}

		bool sensitive = (state == Player::PAUSED || state == Player::PLAYING);
	
		set_sensitive(sensitive);
	}

	/*
	 *
	 */
	void on_timeout()
	{
		long position = m_player->get_position();

		set_position(position);
	}

	/*
	 *
	 */
	void set_duration(long duration)
	{
		m_hscale_seek->set_range(0, duration);
	}

	/*
	 *
	 */
	void set_position(long position)
	{
		if(m_current_seek == false)
		{
			m_hscale_seek->set_value(position);
			m_hscale_seek->queue_draw();
		}
	}

protected:

	/*
	 *
	 */
	Gtk::Button* create_button(Gtk::StockID stock)
	{
		Gtk::Button *button = manage(new Gtk::Button);
		button->set_relief(Gtk::RELIEF_NONE);

		Gtk::Image *image = manage(new Gtk::Image(stock, Gtk::ICON_SIZE_MENU));

		button->add(*image);
		return button;
	}

	/*
	 *
	 */
	Glib::ustring position_to_string(double value)
	{
		return SubtitleTime((long)value).str();
	}

	/*
	 *
	 */
	bool on_seek_event(GdkEvent *ev)
	{
		if(ev->type == GDK_BUTTON_PRESS)
		{
			m_current_seek = true;
		}
		else if(ev->type == GDK_BUTTON_RELEASE)
		{
			long pos = (long)m_hscale_seek->get_value();

			m_player->seek(pos);

			m_current_seek = false;
		}
		return false;
	}
	
protected:
	// controls
	Gtk::Button* m_button_play;
	Gtk::Button* m_button_pause;
	Gtk::HScale* m_hscale_seek;
	bool m_current_seek;

	Player* m_player;
};



/*
 * Constructor
 *
 * Create the GStreamer Player, the PlayerControls (play/pause + seek)
 */
VideoPlayer::VideoPlayer(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:Gtk::VBox(cobject)
{
	m_cfg_display_translated_subtitle = false;

	m_player = manage(new GStreamerPlayer);
	m_player->set_size_request(360, 240);

	Gtk::Frame* m_framePlayer = NULL;
	PlayerControls* m_playerControls = NULL;

	refGlade->get_widget("frame-player", m_framePlayer);
	refGlade->get_widget_derived("player-controls", m_playerControls);

	m_framePlayer->add(*m_player);
	m_playerControls->init_with_player(m_player);

	load_config();

	Config::getInstance().signal_changed("video-player").connect(
			sigc::mem_fun(*this, &VideoPlayer::on_config_video_player_changed));

	DocumentSystem::getInstance().signal_current_document_changed().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_current_document_changed));

	m_player->signal_timeout().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_timeout));

	m_player->signal_state_changed().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_player_state_changed));
}

/*
 * Destructor
 */
VideoPlayer::~VideoPlayer()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);
}

/*
 * Load the video player config.
 */
void VideoPlayer::load_config()
{
	Config &cfg = Config::getInstance();

	if(cfg.get_value_bool("video-player", "display"))
		show();
	else
		hide();

	m_player->set_repeat(cfg.get_value_bool("video-player", "repeat"));

	m_cfg_display_translated_subtitle = cfg.get_value_bool("video-player", "display-translated-subtitle");
}

/*
 * Return the gstreamer player.
 */
GStreamerPlayer* VideoPlayer::player()
{
	return m_player;
}

/*
 * The player state has changed. 
 * Clear subtitle.
 */
void VideoPlayer::on_player_state_changed(Player::State state)
{
	clear_subtitle();
}

/*
 * The config of video player has changed.
 */
void VideoPlayer::on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value)
{
	if(key == "display")
	{
		bool state = utility::string_to_bool(value);

		if(state)
			show();
		else
			hide();
	}
	else if(key == "display-translated-subtitle")
	{
		m_cfg_display_translated_subtitle = utility::string_to_bool(value);
	}
}

/*
 * The current document has changed.
 * Clear subtitle (sub and player text) and try to found the good subtitle.
 */
void VideoPlayer::on_current_document_changed(Document *doc)
{
	clear_subtitle();
	find_subtitle();
}

/*
 * Check or search the good subtitle (find_subtitle).
 */
void VideoPlayer::on_timeout()
{
	find_subtitle();
}

/*
 * Initialize the current subtitle and the player text to NULL.
 */
void VideoPlayer::clear_subtitle()
{
	m_subtitle = Subtitle();
	
	m_player->set_subtitle_text("");
}

/*
 * Check if time is in subtitle.
 */
bool VideoPlayer::is_good_subtitle(const Subtitle &sub, long time)
{
	if(sub)
		if(time >= sub.get_start().totalmsecs && time <= sub.get_end().totalmsecs)
			return true;
		
	return false;
}

/*
 * Try to found the good subtitle and init the player (text).
 */
bool VideoPlayer::find_subtitle()
{
	Document *doc = SubtitleEditorWindow::get_instance()->get_current_document();
	if(doc == NULL)
	{
		clear_subtitle();
		return false;
	}

	long position = m_player->get_position();

	SubtitleTime time(position);

	if(!m_subtitle)
	{
		m_subtitle = doc->subtitles().find(time);
	}
	else
	{
		if(is_good_subtitle(m_subtitle, position)) // is good ?
		{
			show_subtitle_text();
		}
		else if(time < m_subtitle.get_start()) // is the next ?
		{
			show_subtitle_null();
		}
		else if(time > m_subtitle.get_end()) // it's the old, try with the next subtitle
		{
			Subtitle next = doc->subtitles().get_next(m_subtitle);
			if(next)
				m_subtitle = next;
		}
	}
	return true;
}

/*
 * Sets the text of the player to NULL.
 */
void VideoPlayer::show_subtitle_null()
{
	m_player->set_subtitle_text("");
}

/*
 * Sets the text of the player with the current subtitle.
 */
void VideoPlayer::show_subtitle_text()
{
	if(!m_subtitle)
		return;
		
	Glib::ustring text;
		
	if(m_cfg_display_translated_subtitle && !m_subtitle.get_translation().empty())
		text = m_subtitle.get_translation();
	else
		text = m_subtitle.get_text();

	m_player->set_subtitle_text(text);
}


