#ifndef _VideoPlayer_h
#define _VideoPlayer_h

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
#include <libglademm.h>
#include <gst/gst.h>
#include "Document.h"
#include <iostream>
#include "Config.h"

#include "gui/Player.h"
#include "gui/GStreamerPlayer.h"

class HScale : public Gtk::HScale
{
public:
	HScale(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::HScale(cobject)
	{
		m_lock = false;
		add_events(Gdk::SCROLL_MASK);
	}

	bool on_button_press_event(GdkEventButton *ev)
	{
		m_lock = true;
		return Gtk::HScale::on_button_press_event(ev);
	}

	bool on_button_release_event(GdkEventButton *ev)
	{
		Gtk::HScale::on_button_release_event(ev);
		
		gint64 value = (gint64)get_adjustment()->get_value();

		set_value(value);
		
		m_lock = false;
		
		return false;
	}

	bool on_scroll_event(GdkEventScroll *ev)
	{
		Gtk::HScale::on_scroll_event(ev);

		long value = (long)get_adjustment()->get_value();
		long move = SubtitleTime(0,0,10,0).totalmsecs;

		m_lock = true;

		if(ev->direction == GDK_SCROLL_UP)
		{
			set_value(value + move);
		}
		else if(ev->direction == GDK_SCROLL_DOWN)
		{
			set_value(value - move);
		}

		m_lock = false;

		return true;
	}

	/*
	 *
	 */
	bool is_lock()
	{
		return m_lock;
	}
protected:
	bool m_lock;
};

/*
 *
 */
class VideoPlayer : public Gtk::Frame
{
public:

	/*
	 *
	 */
	VideoPlayer(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

	/*
	 *
	 */
	~VideoPlayer();

	/*
	 *
	 */
	Player* get_video_player();

	/*
	 *	permet d'ouvrir une video afin de la lire dans l'area
	 */
	bool open(const Glib::ustring &uri);


	/*
	 *	fonction de rappel
	 *	seulement pendant la lecture du media
	 *	configure la frequence avec m_timeout_time
	 */
	sigc::signal<void>&	signal_timeout();

	/*
	 *
	 */
	void on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value);

	/*
	 *
	 */
	void on_current_document_changed(Document *doc);
protected:

	/*
	 *
	 */
	void on_video_player_state_changed(Player::State state);

	/*
	 *
	 */
	void play_subtitle(const Subtitle &sub);
	void play_subtitle(const SubtitleTime &start, const SubtitleTime &end);

	/*
	 *
	 */
	void on_action_emit(const Glib::ustring &name);

	/*
	 * very-short
	 * short
	 * medium
	 * long
	 */
	void backwards_jump(const Glib::ustring &type);

	/*
	 * very-short
	 * short
	 * medium
	 * long
	 */
	void forward_jump(const Glib::ustring &type);
protected:

	/*
	 *
	 */
	bool on_configure_event_video(GdkEventConfigure *ev);
	
	/*
	 *	affiche la video dans l'area
	 */
	bool on_expose_event_video(GdkEventExpose *ev);

	/*
	 *
	 */
	bool on_visibility_notify_event_video(GdkEventVisibility *ev);

	/*
	 *	fonction callback pour Glib::timeout()
	 */
	void	on_timeout();

	/*
	 *
	 */
	bool is_good_subtitle(const Subtitle &sub, const SubtitleTime &time);

	/*
	 *
	 */
	void clear_subtitle();

	/*
	 *
	 */
	bool find_subtitle();

	/*
	 *
	 */
	void show_subtitle_text();

	/*
	 *
	 */
	void update_interface(Player::State state);

	/*
	 *
	 */
	void on_button_play_clicked();
	void on_button_pause_clicked();

	/*
	 *	Slider Seek
	 */
	void on_slider_seek_value_changed();
	bool on_slider_seek_change_value(Gtk::ScrollType type, double newval);

	/*
	 *
	 */
	bool on_scroll_event_volume(GdkEventScroll *ev);

protected:
	int m_interface_state;

	// Gtk interface
	Gtk::DrawingArea*	m_drawingArea;
	Gtk::Button*			m_buttonPlay;
	Gtk::Button*			m_buttonPause;
	HScale*						m_hscaleTime;
	Gtk::ProgressBar*	m_progressbarVolume;
	Gtk::Label*				m_labelTime;

	Glib::ustring	m_uri;

	//
	Subtitle	m_subtitle;

	GStreamerPlayer m_player;

	bool	m_config_display_translated_subtitle;
};


#endif//_VideoPlayer_h

