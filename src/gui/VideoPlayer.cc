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
#include "Config.h"
#include "debug.h"
#include "utility.h"
#include "DocumentSystem.h"
#include <gst/interfaces/xoverlay.h>
#include <gst/interfaces/colorbalance.h>
#include <gdk/gdkx.h>
#include <cairomm/context.h>

#include "ActionSystem.h"
#include "DocumentSystem.h"
#include "gui/DialogFileChooser.h"

#include "gui/MPlayer.h"
#include "gui/GStreamerPlayer.h"

/*
 *
 */
VideoPlayer::VideoPlayer(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:Gtk::Frame(cobject)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	m_interface_state = -1;

	refGlade->get_widget("drawing-area", m_drawingArea);
	refGlade->get_widget("button-play", m_buttonPlay);
	refGlade->get_widget("button-pause", m_buttonPause);
	refGlade->get_widget_derived("hscale-time", m_hscaleTime);
	refGlade->get_widget("progressbar-volume", m_progressbarVolume);
	refGlade->get_widget("label-time", m_labelTime);

	//
	m_buttonPlay->signal_clicked().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_button_play_clicked));

	m_buttonPause->signal_clicked().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_button_pause_clicked));

	// init drawing area
	m_drawingArea->modify_bg(Gtk::STATE_NORMAL, Gdk::Color("black"));
	
	m_drawingArea->signal_configure_event().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_configure_event_video));
	
	m_drawingArea->signal_visibility_notify_event().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_visibility_notify_event_video));
	
	m_drawingArea->signal_expose_event().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_expose_event_video));

	// slider	
	m_hscaleTime->get_adjustment()->signal_value_changed().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_slider_seek_value_changed));

	m_hscaleTime->signal_change_value().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_slider_seek_change_value));
	// volume
	m_progressbarVolume->add_events(Gdk::SCROLL_MASK);

	m_progressbarVolume->signal_scroll_event().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_scroll_event_volume));

	update_interface(Player::DISABLE);

	ActionSystem::getInstance().signal_emit().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_action_emit));

	Config::getInstance().signal_changed("video-player").connect(
			sigc::mem_fun(*this, &VideoPlayer::on_config_video_player_changed));

	DocumentSystem::getInstance().signal_current_document_changed().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_current_document_changed));


	get_video_player()->get_signal_timeout().connect(
			sigc::mem_fun(*this, &VideoPlayer::on_timeout));

	get_video_player()->get_signal_state_changed().connect(
			sigc::mem_fun(*this, &VideoPlayer::update_interface));

	get_video_player()->set_widget(m_drawingArea);

	Config::getInstance().get_value_bool("video-player", "display-translated-subtitle", m_config_display_translated_subtitle);
}


/*
 *
 */
VideoPlayer::~VideoPlayer()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	get_video_player()->close();
}

Player* VideoPlayer::get_video_player()
{
	return &m_player;
}

/*
 *
 */
void VideoPlayer::on_action_emit(const Glib::ustring &name)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(name == "video-player/open")
	{	
		DialogOpenVideo ui;
		ui.show();
		if(ui.run() == Gtk::RESPONSE_OK)
		{
			Glib::ustring uri = ui.get_uri();
			
			open(uri);
		}
		ui.hide();

		return;
	}

	if(!get_video_player()->is_valid())
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "Video Player is not valid.");
		return;
	}

	Document *doc = DocumentSystem::getInstance().getCurrentDocument();

	if(name == "video-player/play")
	{
		get_video_player()->seek(get_video_player()->get_position());
		get_video_player()->play();
	}
	else if(name == "video-player/pause")
	{
		get_video_player()->pause();
	}
	else if(name == "video-player/play-pause")
	{
		get_video_player()->is_playing() ? get_video_player()->pause() : get_video_player()->play();
	}
	else if(name == "video-player/very-short-backwards-jump")
	{
		backwards_jump("very-short");
	}
	else if(name == "video-player/short-backwards-jump")
	{
		backwards_jump("short");
	}
	else if(name == "video-player/medium-backwards-jump")
	{
		backwards_jump("medium");
	}
	else if(name == "video-player/long-backwards-jump")
	{
		backwards_jump("long");
	}
	else if(name == "video-player/very-short-forward-jump")
	{
		forward_jump("very-short");
	}
	else if(name == "video-player/short-forward-jump")
	{
		forward_jump("short");
	}
	else if(name == "video-player/medium-forward-jump")
	{
		forward_jump("medium");
	}
	else if(name == "video-player/long-forward-jump")
	{
		forward_jump("long");
	}


	if(doc == NULL)
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "Document = NULL");
		return;
	}

	if(name == "video-player/seek-to-selection")
	{
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
		{
			long start = selected.get_start().totalmsecs;
			get_video_player()->seek(start);
		}
	}
	else if(name == "video-player/play-previous-subtitle")
	{
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
		{
			Subtitle sub = doc->subtitles().get_previous(selected);
			if(sub)
			{
				doc->subtitles().select(sub);
				play_subtitle(sub);
			}
		}
	}
	else if(name == "video-player/play-current-subtitle")
	{
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
			play_subtitle(selected);
	}
	else if(name == "video-player/play-next-subtitle")
	{
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
		{
			Subtitle sub = doc->subtitles().get_next(selected);
			if(sub)
			{
				doc->subtitles().select(sub);
				play_subtitle(sub);
			}
		}
	}
	else if(name == "video-player/play-previous-second")
	{
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
			play_subtitle(selected.get_start() - SubtitleTime(0,0,1,0), selected.get_start());
	}
	else if(name == "video-player/play-first-second")
	{
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
			play_subtitle(selected.get_start(), selected.get_start() + SubtitleTime(0,0,1,0));
	}
	else if(name == "video-player/play-last-second")
	{
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
			play_subtitle(selected.get_end() - SubtitleTime(0,0,1,0), selected.get_end());
	}
	else if(name == "video-player/play-next-second")
	{
		Subtitle selected = doc->subtitles().get_first_selected();
		if(selected)
			play_subtitle(selected.get_end(), selected.get_end() + SubtitleTime(0,0,1,0));
	}
	else if(name == "video-player/set-subtitle-start")
	{
		Subtitle selected = doc->subtitles().get_first_selected();

		if(selected)
		{
			SubtitleTime time(get_video_player()->get_position());

			doc->start_command(_("Set subtitle start"));
			
			selected.set_start(time);
			
			doc->emit_signal("subtitle-time-changed");
			doc->finish_command();
		}
	}
	else if(name == "video-player/set-subtitle-end")
	{
		Subtitle selected = doc->subtitles().get_first_selected();

		if(selected)
		{
			SubtitleTime time(get_video_player()->get_position());

			doc->start_command(_("Set subtitle end"));
			
			selected.set_end(time);
			
			Subtitle next = doc->subtitles().get_next(selected);
			// si il n'existe pas de prochain on le crée
			if(!next)
			{
				next = doc->subtitles().append();
			}
			doc->subtitles().select(next);

			doc->emit_signal("subtitle-time-changed");
			doc->finish_command();
		}
	}
}

/*
 *
 */
bool VideoPlayer::open(const Glib::ustring &uri)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "%s", uri.c_str());
	
	if(is_visible() == false)
	{
#warning "TODO: Tester sans ça pour trouver ou cela bug"
		Config::getInstance().set_value_bool("interface", "display-video-player", true);
	}

	get_video_player()->set_widget(m_drawingArea);

	if(get_video_player()->open(uri))
	{
		return true;
	}
	return false;
}

/*
 *
 */
bool VideoPlayer::on_configure_event_video(GdkEventConfigure *ev)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	//Gtk::DrawingArea::on_configure_event(ev);

	Glib::RefPtr<Gdk::Window> window = m_drawingArea->get_window();

	{
		//gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(m_videosink), GDK_WINDOW_XWINDOW(window->gobj()));
		get_video_player()->widget_expose(m_drawingArea);
		//get_video_player()->set_widget(m_drawingArea);//GDK_WINDOW_XID(window->gobj()));
	}

	//Config::getInstance().set_value_int("video-player", "width", get_width());
	//Config::getInstance().set_value_int("video-player", "height", get_height());
	return true;
}

/*
 *
 */
bool VideoPlayer::on_expose_event_video(GdkEventExpose *ev)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	//Gtk::DrawingArea::on_expose_event(ev);
/*
	Glib::RefPtr<Gdk::Window> window = m_drawingArea->get_window();

	if(m_videosink && GST_IS_X_OVERLAY(m_videosink))
	{
		int xid = GDK_WINDOW_XID(window->gobj());
		get_video_player()->set_widget_pix(GDK_WINDOW_XWINDOW(window->gobj()));
		gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(m_videosink), xid);//GDK_WINDOW_XWINDOW(window->gobj()));
		gst_x_overlay_expose(GST_X_OVERLAY(m_videosink));
	}
*/
	Player *player = get_video_player();
	if(player)
		player->widget_expose(m_drawingArea);

	return true;
}

/*
 *
 */
bool VideoPlayer::on_visibility_notify_event_video(GdkEventVisibility *ev)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	Glib::RefPtr<Gdk::Window> win = m_drawingArea->get_window();
	if(win)
	{
		Gdk::Rectangle r(0, 0, m_drawingArea->get_allocation().get_width(),
				m_drawingArea->get_allocation().get_height());
		win->invalidate_rect(r, false);
	}
	return true;
}

/*
 *
 */
bool VideoPlayer::on_scroll_event_volume(GdkEventScroll *ev)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);
	/*

	if(ev->direction == GDK_SCROLL_UP)
	{
		double val = (m_progressbarVolume->get_fraction() + 0.1);

		utility::clamp(val, 0.0, 1.0);

		m_progressbarVolume->set_fraction(val);

		g_object_set(G_OBJECT(m_playbin), "volume", val, NULL);
	}
	else if(ev->direction == GDK_SCROLL_DOWN)
	{
		double val = (m_progressbarVolume->get_fraction() - 0.1);

		utility::clamp(val, 0.0, 1.0);

		m_progressbarVolume->set_fraction(val);

		g_object_set(G_OBJECT(m_playbin), "volume", val, NULL);
	}
	*/
	return true;
}

/*
 *
 */
void VideoPlayer::update_interface(Player::State state)
{	
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(m_interface_state == state)
		return;

	if(state == Player::DISABLE)
	{
		m_buttonPlay->show();
		m_buttonPause->hide();
		m_buttonPlay->set_sensitive(false);
		m_hscaleTime->set_sensitive(false);
		m_progressbarVolume->set_sensitive(false);
		m_labelTime->set_sensitive(false);

		m_hscaleTime->set_range(0,0.1);
		m_hscaleTime->set_value(0);
		m_labelTime->set_text(SubtitleTime::null());

		clear_subtitle();
	}
	else if(state == Player::READY)
	{
		m_hscaleTime->set_range(0, get_video_player()->get_duration());
	}
	else if(state == Player::PLAYING)
	{
		m_buttonPlay->hide();
		m_buttonPlay->set_sensitive(false);
		m_buttonPause->show();
		m_buttonPause->set_sensitive(true);
		m_hscaleTime->set_sensitive(true);
		m_progressbarVolume->set_sensitive(true);
		m_labelTime->set_sensitive(true);

		clear_subtitle();
	}
	else if(state == Player::PAUSED)
	{
		m_buttonPlay->show();
		m_buttonPlay->set_sensitive(true);
		m_buttonPause->hide();
		m_buttonPause->set_sensitive(false);
		m_hscaleTime->set_sensitive(true);
		m_progressbarVolume->set_sensitive(true);
		m_labelTime->set_sensitive(true);

		clear_subtitle();
	}

	m_interface_state = state;

	if(state == Player::DISABLE)
	{
		ActionSystem &as = ActionSystem::getInstance();

		as.set_sensitive("video-player/play", false);
		as.set_sensitive("video-player/pause", false);
		as.set_sensitive("video-player/play-pause", false);
		as.set_sensitive("video-player/seek-to-selection", false);
		as.set_sensitive("video-player/play-previous-subtitle", false);
		as.set_sensitive("video-player/play-next-subtitle", false);
		as.set_sensitive("video-player/play-current-subtitle", false);
		as.set_sensitive("video-player/play-previous-second", false);
		as.set_sensitive("video-player/play-first-second", false);
		as.set_sensitive("video-player/play-last-second", false);
		as.set_sensitive("video-player/play-next-second", false);
		as.set_sensitive("video-player/set-subtitle-start", false);
		as.set_sensitive("video-player/set-subtitle-end", false);
		as.set_sensitive("video-player/menu-backwards-jump", false);
		as.set_sensitive("video-player/menu-forward-jump", false);
	}
	else
	{
		ActionSystem &as = ActionSystem::getInstance();

		as.set_sensitive("video-player/play", true);
		as.set_sensitive("video-player/pause", true);
		as.set_sensitive("video-player/play-pause", true);
		as.set_sensitive("video-player/seek-to-selection", true);
		as.set_sensitive("video-player/play-previous-subtitle", true);
		as.set_sensitive("video-player/play-next-subtitle", true);
		as.set_sensitive("video-player/play-current-subtitle", true);
		as.set_sensitive("video-player/play-previous-second", true);
		as.set_sensitive("video-player/play-first-second", true);
		as.set_sensitive("video-player/play-last-second", true);
		as.set_sensitive("video-player/play-next-second", true);
		as.set_sensitive("video-player/set-subtitle-start", true);
		as.set_sensitive("video-player/set-subtitle-end", true);
		as.set_sensitive("video-player/menu-backwards-jump", true);
		as.set_sensitive("video-player/menu-forward-jump", true);

	}
}

/*
 *	fonction callback pour Glib::timeout()
 */
void VideoPlayer::on_timeout()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);
	
	find_subtitle();

	if(m_hscaleTime->is_lock() == false)
	{
		long position = get_video_player()->get_position();
		m_hscaleTime->set_value(position);

		SubtitleTime time(position);
		m_labelTime->set_text(time.str());
	}
}

/*
 *	fonction de rappel
 *	seulement pendant la lecture du media
 *	configure la frequence dans le constructeur "timeout"
 */
sigc::signal<void>& VideoPlayer::signal_timeout()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	return get_video_player()->get_signal_timeout();
}

/*
 *
 */
bool VideoPlayer::is_good_subtitle(const Subtitle &sub, const SubtitleTime &time)
{
	if(sub)
		if(time >= sub.get_start() && time <= sub.get_end())
			return true;

	return false;
}


/*
 *
 */
void VideoPlayer::clear_subtitle()
{
	m_subtitle = Subtitle();

	get_video_player()->show_text("");
}

/*
 *
 */
bool VideoPlayer::find_subtitle()
{
	if(get_video_player()->is_valid() == false)
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "Video Player is not valid.");
		return false;
	}

	Document *doc = DocumentSystem::getInstance().getCurrentDocument();
	
	if(doc == NULL)
	{
		clear_subtitle();
		return false;
	}

	SubtitleTime time(get_video_player()->get_position());

	if(m_subtitle)
	{
		
		if(is_good_subtitle(m_subtitle, time)) // sous titre dand le temps
		{
			show_subtitle_text();
		}
		else if(time < m_subtitle.get_start()) // il est le prochain sous-titre
		{
			get_video_player()->show_text("");
		}
		else if(time > m_subtitle.get_end()) // il est déjà passer au suivant
		{
			get_video_player()->show_text("");

			Subtitle next = doc->subtitles().get_next(m_subtitle);
			if(next)
			{
				m_subtitle = next;
			}
		}
	}
	else
	{
		m_subtitle = doc->subtitles().find(time);
	}

	return true;
}

/*
 *
 */
void VideoPlayer::show_subtitle_text()
{
	if(!m_subtitle)
		return;

	if(m_config_display_translated_subtitle && !m_subtitle.get_translation().empty())
		get_video_player()->show_text(m_subtitle.get_translation());
	else
		get_video_player()->show_text(m_subtitle.get_text());
}

/*
 *
 */
void VideoPlayer::on_button_play_clicked()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(get_video_player()->is_valid())
	{
		get_video_player()->seek(get_video_player()->get_position());
		get_video_player()->play();
	}
}

/*
 *
 */
void VideoPlayer::on_button_pause_clicked()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(get_video_player()->is_playing())
		get_video_player()->pause();
}

/*
 *	Slider Seek
 */
void VideoPlayer::on_slider_seek_value_changed()
{
	if(m_hscaleTime->is_lock() == false)
	{
		return;
	}
	Gtk::Adjustment *adj = m_hscaleTime->get_adjustment();

	long value = (long)adj->get_value();

	get_video_player()->seek(value);
}

/*
 *
 */
bool VideoPlayer::on_slider_seek_change_value(Gtk::ScrollType type, double newval)
{
	double upper = m_hscaleTime->get_adjustment()->get_upper();

	if(newval > upper)
		newval = upper;

	SubtitleTime time((long)newval);
	m_labelTime->set_text(time.str());
	return false;
}


/*
 *
 */
void VideoPlayer::play_subtitle(const Subtitle &sub)
{
	if(!get_video_player()->is_valid() || !sub)
		return;

	SubtitleTime start = sub.get_start();
	SubtitleTime end = sub.get_end();

	m_subtitle = sub;
	show_subtitle_text();

	play_subtitle(start, end);

}

/*
 *
 */
void VideoPlayer::play_subtitle(const SubtitleTime &start, const SubtitleTime &end)
{
	//get_video_player()->pause();
	get_video_player()->seek(start.totalmsecs, end.totalmsecs);
	get_video_player()->play();
}


/*
 *
 */
void VideoPlayer::on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value)
{
	if(key == "display-translated-subtitle")
	{
		bool state;
		if(from_string(value, state))
			m_config_display_translated_subtitle = state;
	}
}

/*
 *
 */
void VideoPlayer::on_current_document_changed(Document *doc)
{
	clear_subtitle();
	find_subtitle();
}

/*
 * very-short
 * short
 * medium
 * long
 */
void VideoPlayer::backwards_jump(const Glib::ustring &type)
{
	int value = 0;

	if(type == "very-short")
		Config::getInstance().get_value_int("video-player", "jump-very-short", value);
	else if(type == "short")
		Config::getInstance().get_value_int("video-player", "jump-short", value);
	else if(type == "medium")
		Config::getInstance().get_value_int("video-player", "jump-medium", value);
	else if(type == "long")
		Config::getInstance().get_value_int("video-player", "jump-long", value);

	long newpos = get_video_player()->get_position() - SubtitleTime(0,0, value, 0).totalmsecs;


	get_video_player()->seek(newpos, true);
}

/*
 * very-short
 * short
 * medium
 * long
 */
void VideoPlayer::forward_jump(const Glib::ustring &type)
{
	int value = 0;

	if(type == "very-short")
		Config::getInstance().get_value_int("video-player", "jump-very-short", value);
	else if(type == "short")
		Config::getInstance().get_value_int("video-player", "jump-short", value);
	else if(type == "medium")
		Config::getInstance().get_value_int("video-player", "jump-medium", value);
	else if(type == "long")
		Config::getInstance().get_value_int("video-player", "jump-long", value);

	long newpos = get_video_player()->get_position() + SubtitleTime(0,0, value, 0).totalmsecs;

	get_video_player()->seek(newpos, true);
}


