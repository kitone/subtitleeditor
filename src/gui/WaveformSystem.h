#ifndef _WaveformSystem_h
#define _WaveformSystem_h

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
#include "Waveform.h"
#include "Document.h"
#include "VideoPlayer.h"
#include "Player.h"

/*
 *
 */
class WaveformSystem : public Gtk::Frame
{
public:
	WaveformSystem(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~WaveformSystem();

	/*
	 *
	 */
	bool open(const Glib::ustring &uri);
	
	/*
	 *
	 */
	void set_video_player(Player *player);

	/*
	 *
	 */
	void on_action_emit(const Glib::ustring &name);

protected:
	/*
	 *
	 */
	bool open_media(const Glib::ustring &uri);
	
	void init_with_document(Document *doc);

	/*
	 *
	 */
	void on_subtitle_selection_changed();
	void on_subtitle_time_changed();

	/*
	 *
	 */
	void on_video_timeout();

	/*
	 *
	 */
	void on_open_waveform();
	void on_save_waveform();

	void on_config_waveform_changed(const Glib::ustring &key, const Glib::ustring &value);

	void zoom_in();
	void zoom_out();
	void zoom_selection();
	void zoom_all();

	void center_with_selected_subtitle();
protected:

	/*
	 *
	 */
	void init_scrollbarWaveform();

	/*
	 *
	 */
	int get_zoom();
	
	/*
	 *	passe d'une position dans l'area a une position dans le channel
	 *	channel != len
	 */
	int area_to_waveform_channel(int pos);
	double area_to_waveform_channel_2(int pos);

	/*
	 *	retourne la position dans l'area par rapport au temps (du waveform)
	 */
	int get_pos_by_time(gint64 time);

	/*
	 *	retourne le temps (gstreamer) par rapport a la position (pixel)
	 *	sur le waveform
	 */
	gint64 get_time_by_pos(gint64 pos);

	/*
	 *	retourne la position dans l'area par rapport au temps (SubtitleTime.totalmsec)
	 */
	int __get_pos_by_time(long time);

	/*
	 *	retourne le temps (SubtitleTime.totalmsec) par rapport à la position 
	 *	dans l'area
	 */
	long __get_time_by_pos(int pos);

	/*
	 *
	 */
	void __draw_timeline(Cairo::RefPtr<Cairo::Context> &cr, const Gdk::Rectangle &rect);
	void __draw_timeline_msecs(Cairo::RefPtr<Cairo::Context> &cr, long msec, const Gdk::Rectangle &rect, int upper);
	void __draw_timeline_time(Cairo::RefPtr<Cairo::Context> &cr, long msec, const Gdk::Rectangle &rect);

	/*
	 *	retourne la position du debut de l'affichage
	 *	(prise en compte du decalage avec la scrollbar)
	 */
	int get_start_area();

	/*
	 *	retourne la position de la fin de l'affichage
	 *	(prise en compte du decalage avec la scrollbar)
	 *	return get_start_area() + width
	 */
	int get_end_area();

	/*
	 *
	 */
	void draw_channels(Cairo::RefPtr<Cairo::Context> &cr, unsigned int channel, int y, int height);
	
	/*
	 *
	 */
	void draw_marker(Cairo::RefPtr<Cairo::Context> &cr, const Gdk::Rectangle &rec, Subtitle &subtitle, float color[4]);

	/*
	 *
	 */
	void draw_markers(Cairo::RefPtr<Cairo::Context> &cr, const Gdk::Rectangle &rec, Document *doc);

	/*
	 *
	 */
	bool on_expose_event_video(GdkEventExpose *ev);

	/*
	 *
	 */
	bool on_expose_event_waveform(GdkEventExpose *ev);


	/*
	 *
	 */
	bool on_button_press_event_waveform(GdkEventButton *ev);
	bool on_button_release_event_waveform(GdkEventButton *ev);

	/*
	 *
	 */
	bool on_motion_notify_event_waveform(GdkEventMotion *ev);

	/*
	 *
	 */
	bool on_configure_event_waveform(GdkEventConfigure *ev);

	/*
	 *
	 */
	bool on_scroll_event_waveform(GdkEventScroll *ev);

	/*
	 *
	 */
	void on_hscrollbar_waveform_value_changed();

	/*
	 *
	 */
	void on_slider_zoom_changed();
	void on_slider_scale_changed();
	bool on_slider_scroll_event(GdkEventScroll *ev, Gtk::Scale *scale);
	
	/*
	 *
	 */
	void redraw_waveform();
	void redraw_marker();
	void redraw_all();

	/*
	 *	place la position de la scrollbar par rapport 
	 *	a la lecture
	 */
	void update_view_from_cursor();

	void set_zoom(int zoom);
	void set_scale(int scale);

	void load_config();

	void scroll_to_position(gint position);
	void scroll_to_position_and_center(gint position);

	Document* document();
protected:

	bool m_display_time_info;

	Player*			m_playerVideo;

	Gtk::DrawingArea* m_drawingWaveform;
	Gtk::HScrollbar*	m_hscrollbarWaveform;
	Gtk::VScale*			m_sliderZoom;
	Gtk::VScale*			m_sliderScale;

	int m_scale;
	int m_zoom;

	Glib::RefPtr<Waveform> m_waveform;

	Cairo::RefPtr<Cairo::Surface> m_waveform_surface;
	Cairo::RefPtr<Cairo::Surface> m_waveform_marker_surface;
	//
	Document* m_currentDocument;
	std::map<std::string, sigc::connection> m_connection;

	// config
	bool m_display_waveform_fill;
	bool m_display_background;
	bool m_scrolling_with_cursor;
	bool m_scrolling_with_selection;
	bool m_display_subtitle_text;

	float m_color_background[4];
	float m_color_play_line[4];
	float m_color_text[4];
	float m_color_wave[4];
	float m_color_wave_fill[4];
	float m_color_marker[4];
	float m_color_marker_hightlight[4];
	float m_color_marker_invalid[4];
};

#endif//_WaveformSystem_h

