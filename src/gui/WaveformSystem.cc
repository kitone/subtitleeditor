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
 

#include "WaveformSystem.h"
#include <cairomm/context.h>
#include <gst/gst.h>
#include "utility.h"
#include "DocumentSystem.h"

#include "ActionSystem.h"
#include "gui/OpenWaveformUI.h"
#include "gui/WaveformGeneratorUI.h"

#include <gtk/gtk.h>

bool new_method = false;

/*
 *	use directly gtk beacause gtkmm doesn't work!
 */
bool on_gtk_scroll_event(GtkWidget *w, GdkEventScroll *ev, gpointer data)
{
	if(ev->direction == GDK_SCROLL_UP)
		gtk_range_set_value(GTK_RANGE(w), gtk_range_get_value(GTK_RANGE(w)) + 1);
	else if(ev->direction == GDK_SCROLL_DOWN)
		gtk_range_set_value(GTK_RANGE(w), gtk_range_get_value(GTK_RANGE(w)) - 1);
	else
		return false;

	return true;
}

/*
 *
 */
WaveformSystem::WaveformSystem(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:Gtk::Frame(cobject)
{
	m_zoom = m_scale = 0;
	m_display_time_info = false;
	m_display_subtitle_text = true;

	se_debug(SE_DEBUG_WAVEFORM);

	m_scrolling_with_cursor = true;
	m_scrolling_with_selection = true;
	
	m_currentDocument = NULL;
	m_playerVideo = NULL;

	refGlade->get_widget("drawing-waveform", m_drawingWaveform);
	refGlade->get_widget("hscrollbar-waveform", m_hscrollbarWaveform);

	refGlade->get_widget("vscale-zoom", m_sliderZoom);
	refGlade->get_widget("vscale-scale", m_sliderScale);

	// waveform
	m_drawingWaveform->add_events(
			Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON_MOTION_MASK | Gdk::SCROLL_MASK);

	m_drawingWaveform->signal_expose_event().connect(
			sigc::mem_fun(*this, &WaveformSystem::on_expose_event_waveform));
	
	m_drawingWaveform->signal_configure_event().connect(
			sigc::mem_fun(*this, &WaveformSystem::on_configure_event_waveform));

	m_drawingWaveform->signal_button_press_event().connect(
			sigc::mem_fun(*this, &WaveformSystem::on_button_press_event_waveform));
	
	m_drawingWaveform->signal_button_release_event().connect(
			sigc::mem_fun(*this, &WaveformSystem::on_button_release_event_waveform));

	m_drawingWaveform->signal_motion_notify_event().connect(
			sigc::mem_fun(*this, &WaveformSystem::on_motion_notify_event_waveform));

	m_drawingWaveform->signal_scroll_event().connect(
			sigc::mem_fun(*this, &WaveformSystem::on_scroll_event_waveform));

	// scrollbar
	m_hscrollbarWaveform->signal_value_changed().connect(
			sigc::mem_fun(*this, &WaveformSystem::on_hscrollbar_waveform_value_changed));

	// slider
	m_sliderZoom->signal_value_changed().connect(
			sigc::mem_fun(*this, &WaveformSystem::on_slider_zoom_changed));
	m_sliderZoom->add_events(Gdk::SCROLL_MASK);
	//m_sliderZoom->signal_scroll_event().connect(
	//		sigc::bind(sigc::mem_fun(*this, &WaveformSystem::on_slider_scroll_event), m_sliderZoom));
	g_signal_connect(G_OBJECT(m_sliderZoom->gobj()), "scroll-event", G_CALLBACK(on_gtk_scroll_event), NULL);


	m_sliderScale->signal_value_changed().connect(
			sigc::mem_fun(*this, &WaveformSystem::on_slider_scale_changed));
	m_sliderScale->add_events(Gdk::SCROLL_MASK);
	//m_sliderScale->signal_scroll_event().connect(
	//		sigc::bind(sigc::mem_fun(*this, &WaveformSystem::on_slider_scroll_event), m_sliderScale));
	g_signal_connect(G_OBJECT(m_sliderScale->gobj()), "scroll-event", G_CALLBACK(on_gtk_scroll_event), NULL);


	//
	DocumentSystem::getInstance().signal_current_document_changed().connect(
			sigc::mem_fun(*this, &WaveformSystem::init_with_document));

	ActionSystem::getInstance().signal_emit().connect(
			sigc::mem_fun(*this, &WaveformSystem::on_action_emit));
	
	//
	set_sensitive(false);

	int zoom, scale;
	Config::getInstance().get_value_int("waveform", "zoom", zoom);
	Config::getInstance().get_value_int("waveform", "scale", scale);

	set_zoom(zoom);
	set_scale(scale);

	Config::getInstance().signal_changed("waveform").connect(
			sigc::mem_fun(*this, &WaveformSystem::on_config_waveform_changed));

	// on n'utilise pas width puisque le waveform 
	// doit prendre toute la place disponible
	//int height;
	//if(Config::getInstance().get_value_int("waveform", "height", height))
	//{
	//	set_size_request(-1, height);
	//}
}

/*
 *
 */
WaveformSystem::~WaveformSystem()
{
	se_debug(SE_DEBUG_WAVEFORM);
//	m_waveform.clear();
}

/*
 *
 */
Document* WaveformSystem::document()
{
	return m_currentDocument;
}

/*
 *
 */
void WaveformSystem::set_video_player(Player *player)
{
	se_debug(SE_DEBUG_WAVEFORM);

	m_playerVideo = player;

	m_playerVideo->get_signal_timeout().connect(
			sigc::mem_fun(*this, &WaveformSystem::on_video_timeout));
}

/*
 *
 */
void read_config_color_p(const Glib::ustring &name, float rgba[4])
{
	Color color;
	Config::getInstance().get_value_color("waveform", name, color);

	color.get_value(rgba, 1);
}

/*
 *
 */
void WaveformSystem::load_config()
{
	se_debug(SE_DEBUG_WAVEFORM);
	
	read_config_color_p("color-background", m_color_background);
	read_config_color_p("color-play-line", m_color_play_line);
	read_config_color_p("color-text", m_color_text);
	read_config_color_p("color-wave", m_color_wave);
	read_config_color_p("color-wave-fill", m_color_wave_fill);
	read_config_color_p("color-marker", m_color_marker);
	read_config_color_p("color-marker-hightlight", m_color_marker_hightlight);
	read_config_color_p("color-marker-invalid", m_color_marker_invalid);

	Config &cfg = Config::getInstance();
	
	cfg.get_value_bool("waveform", "display-waveform-fill", m_display_waveform_fill);
	cfg.get_value_bool("waveform", "display-background", m_display_background);
	cfg.get_value_bool("waveform", "display-subtitle-text", m_display_subtitle_text);
	cfg.get_value_bool("waveform", "scrolling-with-cursor", m_scrolling_with_cursor);
	cfg.get_value_bool("waveform", "scrolling-with-selection", m_scrolling_with_selection);
}

/*
 *
 */
bool WaveformSystem::open_media(const Glib::ustring &uri)
{
	se_debug_message(SE_DEBUG_WAVEFORM, "uri=%s", uri.c_str());

	if(m_waveform)
		m_waveform.clear();

	m_waveform = Glib::RefPtr<Waveform>(new Waveform);
	
	bool state = m_waveform->open(uri);

#warning "FIXME: show/hide Config/MenuBar"
	if(state)
	{
		set_sensitive(true);
		init_scrollbarWaveform();
		
		Config::getInstance().set_value_bool("interface", "display-waveform", true);
		
		show();
	}
	else
	{
		m_waveform.clear();

		set_sensitive(false);
		hide();
	}

	redraw_all();

	return state;
}

/*
 *
 */
void WaveformSystem::set_zoom(int zoom)
{
	se_debug_message(SE_DEBUG_WAVEFORM, "zoom=%d", zoom);

	utility::clamp(zoom, 1, 1000);

	if(m_zoom == zoom)
		return;

	m_zoom = zoom;

	Config::getInstance().set_value_int("waveform", "zoom", m_zoom);

	m_sliderZoom->set_value(zoom);

	init_scrollbarWaveform();

	redraw_all();
}

/*
 *
 */
void WaveformSystem::set_scale(int scale)
{
	se_debug_message(SE_DEBUG_WAVEFORM, "scale=%d", scale);

	utility::clamp(scale, 1, 10);

	if(m_scale == scale)
		return;

	m_scale = scale;

	m_sliderScale->set_value(scale);

	Config::getInstance().set_value_int("waveform", "scale", m_scale);

	redraw_waveform();
}

/*
 *
 */
void WaveformSystem::on_hscrollbar_waveform_value_changed()
{
	se_debug(SE_DEBUG_WAVEFORM);

	redraw_all();
}

/*
 *
 */
bool WaveformSystem::on_configure_event_waveform(GdkEventConfigure *ev)
{
	se_debug(SE_DEBUG_WAVEFORM);

	init_scrollbarWaveform();
	
	redraw_all();

	return false;
}

/*
 *	retourne le temps en msec (SubtitleTime.totalmsecs) 
 *	par rapport à la position dans la drawingarea
 */
long WaveformSystem::__get_time_by_pos(int pos)
{
	float width = (float)m_drawingWaveform->get_width() * get_zoom();
	float percent = ((float)pos / width);
	float gsttime = ((float)m_waveform->get_duration() * percent);
	return SubtitleTime(long(gsttime / GST_MSECOND)).totalmsecs;
}

/*
 *	return la position dans la drawingarea par rapport
 *	au temps en msec (SubtitleTime.totalmsecs)
 */
int WaveformSystem::__get_pos_by_time(long time)
{
	gint64 gsttime = time * GST_MSECOND;
	float percent = ((float)gsttime / (float)m_waveform->get_duration());
	float width = (float)m_drawingWaveform->get_width() * get_zoom();
	float pos = width * percent;
	pos = CLAMP(pos, 0, width);
	return (int)pos;
}

/*
 *
 */
void WaveformSystem::__draw_timeline(Cairo::RefPtr<Cairo::Context> &cr, const Gdk::Rectangle &rect)
{
	int width = rect.get_width();
	int height = rect.get_height();

	// background
	cr->set_source_rgb(1,1,1);
	cr->rectangle(0, 0, width, height);
	cr->fill();

	// line H
	cr->set_source_rgb(0,0,0);
	cr->move_to(0, height);
	cr->line_to(width, height);
	cr->stroke();

	long sec_1 = SubtitleTime(0,0,1,0).totalmsecs;
	long sec_5 = SubtitleTime(0,0,5,0).totalmsecs;
	long sec_10 = SubtitleTime(0,0,10,0).totalmsecs;

	while( __get_pos_by_time(sec_5) < 60)
	{
		sec_1  *= 2;
		sec_5  *= 2;
		sec_10 *= 2;
	}
	__draw_timeline_msecs(cr, sec_1, rect, height - 3);
	__draw_timeline_msecs(cr, sec_5, rect, height - 5);
	__draw_timeline_msecs(cr, sec_10, rect, height - 8);

	__draw_timeline_time(cr, sec_5, rect);

}

/*
 *
 */
void WaveformSystem::__draw_timeline_msecs(Cairo::RefPtr<Cairo::Context> &cr, long msec, const Gdk::Rectangle &rect, int upper)
{
	int height = rect.get_height();

	int start_area = get_start_area();
	int end_area = get_end_area();

	long start = __get_time_by_pos(start_area);
	long end = __get_time_by_pos(end_area);

	long diff = start % msec;

	start -= diff;

	// TODO: FIXME
	if(__get_pos_by_time(start) < start_area)
		start += msec;

	for(long t = start; t< end; t+=msec)
	{
		int x = __get_pos_by_time(t) - start_area;

		cr->move_to(x, height);
		cr->line_to(x, upper);
	}
	cr->stroke();
}

/*
 *
 */
void WaveformSystem::__draw_timeline_time(Cairo::RefPtr<Cairo::Context> &cr, long msec, const Gdk::Rectangle &rect)
{
	if(msec > 0)
	{
		cr->set_source_rgb(0,0,0);
		cr->set_font_size(13);

		int height = rect.get_height() - 15;

		int start_area = get_start_area();
		int end_area = get_end_area();

		long start = __get_time_by_pos(start_area);
		long end = __get_time_by_pos(end_area);

		long diff = start % msec;

		start -= diff;

		Cairo::TextExtents ex;
		cr->get_text_extents(Gst::time_to_string(0), ex);

		//int sub_width = start_area - ex.width * 0.5;

		for(long t = start; t< end; t+=msec)
		{
			int x = __get_pos_by_time(t) - start_area;

			cr->move_to(x - ex.width * 0.5, height);
	
			cr->show_text(SubtitleTime(t).str().substr(0,7));
		}
	}
}

/*
 *	Waveform
 */
bool WaveformSystem::on_expose_event_waveform(GdkEventExpose *ev)
{
	se_debug(SE_DEBUG_WAVEFORM);

	Glib::RefPtr<Gdk::Window> window = m_drawingWaveform->get_window();
	
	if(!window)
		return false;

	if(!m_waveform)
		return false;

	static bool loading = false;

	if(!loading)
	{
		loading = true;
		load_config();
	}

#warning "use cairo::push/pop ?"
	// widget size
	Gtk::Allocation allocation = m_drawingWaveform->get_allocation();
    
	int width = allocation.get_width();
  int height = allocation.get_height();

	Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

	//cr->set_antialias(Cairo::ANTIALIAS_NONE);

	if(ev)
	{
		cr->rectangle(ev->area.x, ev->area.y, ev->area.width, ev->area.height);
		cr->clip();
	}

	// waveform
	if(!m_waveform_surface)
	{
		//
		m_waveform_surface = Cairo::Surface::create(cr->get_target(), Cairo::CONTENT_COLOR_ALPHA, width, height);
		Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(m_waveform_surface);

		//context->set_antialias(Cairo::ANTIALIAS_NONE);

		if(m_display_background)
		{
			context->set_source_rgba(
					m_color_background[0], m_color_background[1], m_color_background[2], m_color_background[3]);

			context->paint();
		}

		__draw_timeline(context, Gdk::Rectangle(0,0, width, 30));

		height = height - 30;
	
		unsigned int n_channels = m_waveform->get_n_channels();

		for(unsigned int i=0; i<n_channels; ++i)
		{
			draw_channels(context, i, i*(height/n_channels) + 30, (height/n_channels));
		}
	}

	// marker
	if(!m_waveform_marker_surface)
	{
		m_waveform_marker_surface = Cairo::Surface::create(cr->get_target(), Cairo::CONTENT_COLOR_ALPHA, width, height);
		Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(m_waveform_marker_surface);
		
		context->set_antialias(Cairo::ANTIALIAS_NONE);

		if(document())
		{
			Gdk::Rectangle rec(0,0, width, height);
			draw_markers(context, rec, document());
		}
	}

	if(m_waveform_surface)
	{
		cr->set_source(m_waveform_surface, 0,0);
		cr->paint();
	}

	if(m_waveform_marker_surface)
	{
		cr->set_source(m_waveform_marker_surface, 0, 30);
		cr->paint();
	}

	if(m_playerVideo && m_playerVideo->is_valid())
	{
		// current play
		int position = __get_pos_by_time( m_playerVideo->get_position() ) - get_start_area();

		cr->set_source_rgba(m_color_play_line[0], m_color_play_line[1], m_color_play_line[2], m_color_play_line[3]);
		cr->move_to(position, 30);
		cr->line_to(position, height+30);
		cr->stroke();
	}

	// display time
	if(document() && m_display_time_info)
	{
		gint x=0, y=0;
		Gdk::ModifierType mod;
		Glib::RefPtr<Gdk::Window> win = window->get_pointer(x,y, mod);
		
		//if(	x > ev->area.x && x < ev->area.width &&
		//		y > ev->area.y && y < ev->area.height )
		{
			//if((mod & Gdk::BUTTON1_MASK) || (mod & Gdk::BUTTON3_MASK))
			{
				// current time is displayed 
				guint pos = (guint)x + get_start_area();

				SubtitleTime time ( __get_time_by_pos(pos) );

				cr->set_font_size(12);
				cr->set_source_rgb(m_color_text[0], m_color_text[1], m_color_text[2]);

				Cairo::TextExtents ex;
				cr->get_text_extents(SubtitleTime::null(), ex);

				cr->move_to(x - ex.width * 0.5, y - 5);
				cr->show_text(time.str());

				// duration is displayed in center of subtitle.
				Subtitle sub = document()->subtitles().get_first_selected();
				if(sub)
				{
					int center = __get_pos_by_time((sub.get_start() + (sub.get_end() - sub.get_start()) * 0.5).totalmsecs) - get_start_area();

					cr->move_to(center - ex.width * 0.5, y + 30);
					cr->show_text(sub.get_duration().str());
				}
			}
		}
	}
	return true;
}

/*
 *
 */
bool WaveformSystem::on_button_press_event_waveform(GdkEventButton *ev)
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(!m_waveform)
		return true;

	// on recupere la position et on transforme en temps dans la waveform

	// on prend en compte la hscrollbar
	guint position = (guint)ev->x + get_start_area();
	
	// position to time
	SubtitleTime time( __get_time_by_pos(position) );

	if(ev->button == 2  && !(ev->state & Gdk::CONTROL_MASK) && m_playerVideo)
	{
#warning "FIXME: RESTORE PLAYER"

		if(m_playerVideo->is_valid())
		{
			m_playerVideo->seek(time.totalmsecs);
			if(m_playerVideo->is_playing() == false)
				m_playerVideo->play();
		}
		
		//new_method = ! new_method;
		//redraw_waveform();
		return true;
	}
	else if(ev->button == 2 && ev->state & Gdk::CONTROL_MASK && document() != NULL)
	{
		Subtitle sub = document()->subtitles().find(time);
		if(sub)
		{
			document()->subtitles().select(sub);
		}
		return true;
	}

	if(document() == NULL)
		return true;

	Subtitle subtitle = document()->subtitles().get_first_selected();
	
	if(!subtitle)
		return true;

	if(ev->button == 1 && ev->state & Gdk::CONTROL_MASK)
	{
		// on décale la début et la fin du précédent sous-titre

		// on récupere l'espace entre les deux sous-titres
		SubtitleTime diff = time - subtitle.get_start();

		document()->start_command(_("Editing position"));
	
		subtitle.set_start(time);

		Subtitle previous = document()->subtitles().get_previous(subtitle);
		
		if(previous)
			previous.set_end(previous.get_end() + diff);
			
		document()->emit_signal("subtitle-time-changed");
	}
	else if(ev->button == 1)
	{
		document()->start_command(_("Editing position"));
		subtitle.set_start(time);
		document()->emit_signal("subtitle-time-changed");
	}
	else if(ev->button == 3 && ev->state & Gdk::CONTROL_MASK)
	{
		// on décale la fin et le début du prochain sous-titre

		// on récupere l'espace entre les deux sous-titres
		SubtitleTime diff = time - subtitle.get_end();

		document()->start_command(_("Editing position"));
		subtitle.set_end(time);

		Subtitle next = document()->subtitles().get_next(subtitle);
		if(next)
		{
			next.set_start(next.get_start() + diff);
		}
		document()->emit_signal("subtitle-time-changed");
	}
	else if(ev->button == 3)
	{
		document()->start_command(_("Editing position"));
		subtitle.set_end(time);
		document()->emit_signal("subtitle-time-changed");
	}

	m_display_time_info = true;

	return true;
}

/*
 *
 */
bool WaveformSystem::on_button_release_event_waveform(GdkEventButton *ev)
{
	m_display_time_info = false;
	m_drawingWaveform->queue_draw();

	if(document() != NULL)
		document()->finish_command();
	
	return true;
}

/*
 *
 */
bool WaveformSystem::on_motion_notify_event_waveform(GdkEventMotion *ev)
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(document() == NULL || !m_waveform)
		return true;

	Subtitle subtitle = document()->subtitles().get_first_selected();
	
	if(!subtitle)
		return true;

	// on recupere la position et on transforme en temps dans la waveform

	// on prend en compte la hscrollbar
	guint position = CLAMP((guint)ev->x, 0, (guint)m_drawingWaveform->get_width()) + get_start_area();

	// position to time
	SubtitleTime time( __get_time_by_pos(position) );

	if(ev->state & Gdk::BUTTON1_MASK && ev->state & Gdk::CONTROL_MASK)
	{
		// on décale la début et la fin du précédent sous-titre

		// on récupere l'espace entre les deux sous-titres
		SubtitleTime diff = time - subtitle.get_start();

		subtitle.set_start(time);

		Subtitle previous = document()->subtitles().get_previous(subtitle);
		if(previous)
		{
			previous.set_end(previous.get_end() + diff);
		}
		document()->emit_signal("subtitle-time-changed");
	}
	else if(ev->state & Gdk::BUTTON1_MASK && ev->state & Gdk::SHIFT_MASK)
	{
		subtitle.set_end(time);
		document()->emit_signal("subtitle-time-changed");
	}
	else if(ev->state & Gdk::BUTTON1_MASK)
	{
		subtitle.set_start(time);
		document()->emit_signal("subtitle-time-changed");
	}
	else if(ev->state & Gdk::BUTTON3_MASK && ev->state & Gdk::CONTROL_MASK)
	{
		// on décale la fin et le début du prochain sous-titre

		// on récupere l'espace entre les deux sous-titres
		SubtitleTime diff = time - subtitle.get_end();

		subtitle.set_end(time);

		Subtitle next = document()->subtitles().get_next(subtitle);
		if(next)
		{
			next.set_start(next.get_start() + diff);
		}
		document()->emit_signal("subtitle-time-changed");
	}
	else if(ev->state & Gdk::BUTTON3_MASK)
	{
		subtitle.set_end(time);
		document()->emit_signal("subtitle-time-changed");
	}
	return true;
}

/*
 *
 */
bool WaveformSystem::on_scroll_event_waveform(GdkEventScroll *ev)
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(!m_waveform)
		return true;

	// scale
	if(ev->direction == GDK_SCROLL_UP && ev->state & Gdk::SHIFT_MASK)
	{
		set_scale(m_scale + 1);
	}
	else if(ev->direction == GDK_SCROLL_DOWN && ev->state & Gdk::SHIFT_MASK)
	{
		set_scale(m_scale - 1);
	}
	// zoom
	else if(ev->direction == GDK_SCROLL_UP && ev->state & Gdk::CONTROL_MASK)
	{
		int center = get_start_area() + (int)((get_end_area() - get_start_area()) * 0.5);
		long time = __get_time_by_pos(center);

		zoom_in();
		
		scroll_to_position_and_center(__get_pos_by_time(time));
	}
	else if(ev->direction == GDK_SCROLL_DOWN && ev->state & Gdk::CONTROL_MASK)
	{
		int center = get_start_area() + (int)((get_end_area() - get_start_area()) * 0.5);
		long time = __get_time_by_pos(center);

		zoom_out();
		
		scroll_to_position_and_center(__get_pos_by_time(time));
	}
	// scroll
	else if(ev->direction == GDK_SCROLL_UP)
	{
		double page_size = m_hscrollbarWaveform->get_adjustment()->get_page_size();
		double delta = pow(page_size, 2.0 / 3.0);
		m_hscrollbarWaveform->set_value(m_hscrollbarWaveform->get_value() - delta);
	}
	else if(ev->direction == GDK_SCROLL_DOWN)
	{
		double page_size = m_hscrollbarWaveform->get_adjustment()->get_page_size();
		double delta = pow(page_size, 2.0 / 3.0);
		m_hscrollbarWaveform->set_value(m_hscrollbarWaveform->get_value() + delta);
	}
	
	return true;
}

/*
 *   Glib::signal_timeout().connect( sigc::mem_fun(*this, &WaveformSystem::onSecondElapsed), 1000);
 */
void WaveformSystem::redraw_waveform()
{
	se_debug(SE_DEBUG_WAVEFORM);

	m_waveform_surface.clear();

	m_drawingWaveform->queue_draw();
}

/*
 *	efface simplement la surface (cairo)
 *	puis on demande au widget (drawing area)
 *	de rafraichir l'affichage
 *	pour dessiner la surface
 */
void WaveformSystem::redraw_marker()
{
	se_debug(SE_DEBUG_WAVEFORM);

	m_waveform_marker_surface.clear();

	m_drawingWaveform->queue_draw();
}

/*
 *	efface simplement les surfaces (cairo)
 *	puis on demande au widget (drawing area)
 *	de rafraichir l'affichage
 *	pour dessiner la surface
 */
void WaveformSystem::redraw_all()
{
	se_debug(SE_DEBUG_WAVEFORM);

	m_waveform_surface.clear();
	m_waveform_marker_surface.clear();

	m_drawingWaveform->queue_draw();
}


/*
 *
 */
int WaveformSystem::get_zoom()
{
	//se_debug_message(SE_DEBUG_WAVEFORM, "zoom=%f", m_zoom);

	return m_zoom;
}

/*
 *	passe d'une position dans l'area à une position dans le channel
 *	channel != len
 */
int WaveformSystem::area_to_waveform_channel(int pos)
{
	//se_debug(SE_DEBUG_WAVEFORM);

	// taille total du widget
	float width = (float)m_drawingWaveform->get_width() * get_zoom();

	//on bascule en % a partir de l'espace area
	float percent = (float)pos / width;

	// du % on rebascule sur l'espace channel (waveform)
	return (int)((float)m_waveform->get_size() * percent);
}

/*
 *
 */
double WaveformSystem::area_to_waveform_channel_2(int pos)
{
	//se_debug(SE_DEBUG_WAVEFORM);

	// taille total du widget
	double width = (double)m_drawingWaveform->get_width() * get_zoom();

	//on bascule en % a partir de l'espace area
	double percent = (double)pos / width;

	// du % on rebascule sur l'espace channel (waveform)
	return (double)m_waveform->get_size() * percent;
}

/*
 *	init la scrollbar selon ça taille et le zoom
 */
void WaveformSystem::init_scrollbarWaveform()
{
	se_debug(SE_DEBUG_WAVEFORM);

	double upper = m_hscrollbarWaveform->get_adjustment()->get_upper();
	double old_value = m_hscrollbarWaveform->get_value();

	guint width = m_drawingWaveform->get_width();

	Gtk::Adjustment *adj = m_hscrollbarWaveform->get_adjustment();

	adj->set_page_size((double)width);
	adj->set_page_increment(width);
	adj->set_step_increment(width);

	m_hscrollbarWaveform->set_range(0, width * get_zoom());

	if(upper > 0)
		m_hscrollbarWaveform->set_value(width * get_zoom() * (old_value / upper));
}

/*
 *	retourne la position du debut de l'affichage
 *	(prise en compte du decalage avec la scrollbar)
 */
int WaveformSystem::get_start_area()
{
	//se_debug(SE_DEBUG_WAVEFORM);

	return (int)m_hscrollbarWaveform->get_value();
}

/*
 *	retourne la position de la fin de l'affichage
 *	(prise en compte du decalage avec la scrollbar)
 *	return get_start_area() + width
 */
int WaveformSystem::get_end_area()
{
	//se_debug(SE_DEBUG_WAVEFORM);

	return get_start_area() + m_drawingWaveform->get_width();
}

/*
 *	retourne la position dans l'area par rapport au temps (du waveform)
 */
int WaveformSystem::get_pos_by_time(gint64 time)
{
	//se_debug(SE_DEBUG_WAVEFORM);

	// passage ne % a partir du waveform
	float percent = ((float)time / (float)m_waveform->get_duration());


	float width = (float)m_drawingWaveform->get_width() * get_zoom();

	// du % on passe a une position dans le widget
	float pos = width * percent;
 
	if(pos < 0) 
		pos = 0;
	else if(pos > width) 
		pos = width;

	return (int)pos;
}

/*
 *	retourne le temps (gstreamer) par rapport a la position (pixel)
 *	sur le waveform
 */
gint64 WaveformSystem::get_time_by_pos(gint64 pos)
{
	//se_debug(SE_DEBUG_WAVEFORM);

	float width = (float)m_drawingWaveform->get_width() * get_zoom();

	float percent = ((float)pos / width);
	
	float time = ((float)m_waveform->get_duration() * percent);
	return (gint64)time;
}


/*
 *
 */
void WaveformSystem::draw_channels(Cairo::RefPtr<Cairo::Context> &cr, unsigned int channel, int y, int height)
{
	se_debug(SE_DEBUG_WAVEFORM);

	cr->set_source_rgba(m_color_wave[0], m_color_wave[1], m_color_wave[2], m_color_wave[3]);
	
	guint count = 4;
	
	double peak, scale_peak;
	float scale = m_scale * height;

	int y_height = y + height;
/*
	cr->move_to(0, y + height);

	gint t=0;

	for(t=startx; t<endx; t+=count)
	{
		peak = m_waveform->get_channel(channel, area_to_waveform_channel(t));

		scale_peak = peak * scale;

		if(scale_peak < 0) scale_peak = - scale_peak;
		if(scale_peak > height) scale_peak = height;

		cr->line_to(t - startx, y_height - scale_peak);
	}
	cr->line_to(endx, y+height);
*/
	if(!new_method)
	{
	// optimisation
	{
		// place le point de départ
		cr->move_to(0, y_height);
		
		// recherche le point de départ par rapport au waveform
		double begin = area_to_waveform_channel_2(get_start_area());

		// recherche le prochain point dans le waveform
		// par rapport au décalage (4 pixel)
		// ce qui permet une constante qu'importe le zoom
		double move = area_to_waveform_channel_2(count);

		// la longueur de la surface afin de ne rendre seulement
		// ce qui est visible
		gint lenght = m_drawingWaveform->get_width();
		
		double x=begin;

		for(gint t=0; t<lenght; t+=count, x+=move)
		{
			peak = m_waveform->get_channel(channel, (int)x);
	
			scale_peak = peak * scale;

			if(scale_peak < 0) scale_peak = - scale_peak;
			if(scale_peak > height) scale_peak = height;

			cr->line_to(t, y_height - scale_peak);
		}

		cr->line_to(lenght, y+height);
	}
	// 

	if(m_display_waveform_fill)
	{
		cr->fill_preserve();
		cr->set_source_rgba(m_color_wave_fill[0], m_color_wave_fill[1], m_color_wave_fill[2], m_color_wave_fill[3]);
		cr->stroke_preserve();
	}
	else
		cr->fill();

	}//new_method
	else{

	//cr->set_line_width(0.5);

	int width = m_drawingWaveform->get_width();

	double skip = (double)(width * m_zoom) / m_waveform->get_size();

	double x = - get_start_area();

	cr->move_to(0, y_height);
	for(int i=0; i<m_waveform->get_size(); ++i, x+=skip)
	{
		double peak = m_waveform->get_channel(channel, i);

		double peakOnScreen = (peak * scale);

		if(peakOnScreen < 0) peakOnScreen = - peakOnScreen;
		if(peakOnScreen > height) peakOnScreen = height;

		//cr->move_to(x, y_height);
		cr->line_to(x, y_height - peakOnScreen);

		if(x > width)
			break;
	}
	//cr->stroke();

	cr->line_to(x, y_height);
	
	if(m_display_waveform_fill)
	{
		cr->fill_preserve();
		cr->set_source_rgba(m_color_wave_fill[0], m_color_wave_fill[1], m_color_wave_fill[2], m_color_wave_fill[3]);
		cr->stroke_preserve();
	}
	else
		cr->fill();

	}//new_method

}

/*
 *
 */
void WaveformSystem::draw_marker(Cairo::RefPtr<Cairo::Context> &cr, const Gdk::Rectangle &rec, Subtitle &subtitle, float color[4])
{
//	se_debug(SE_DEBUG_WAVEFORM);

	long start = subtitle.get_start().totalmsecs;
	long end = subtitle.get_end().totalmsecs;

	int y = rec.get_y();
	int width = rec.get_width();
	int height = rec.get_height();

	gint64 x = __get_pos_by_time(start) - get_start_area();
	gint64 w = __get_pos_by_time(end) - get_start_area();

	if(((x >= 0 && x <= width) || (w >=0 && w <= width) || (x <=0 && w >= width)) == false)
		return;

	if(start == end)
		return;

	cr->move_to(0,0);

	if(x > w)
		cr->set_source_rgba(m_color_marker_invalid[0], m_color_marker_invalid[1], m_color_marker_invalid[2], m_color_marker_invalid[3]);
	else
		cr->set_source_rgba(color[0], color[1], color[2], color[3]);

	cr->rectangle(x, y, w-x, height);
	cr->fill();
/*
	cr->set_source_rgba(0.3, 0.3, 0.3, 0.5);
	
	cr->move_to(x, y);
	cr->line_to(x, height);

	cr->move_to(w, y);
	cr->line_to(w, height);

	cr->stroke();
*/
	// display subtitle text
	if(m_display_subtitle_text)
	{
		cr->save();

		cr->rectangle(x, y, w-x, height);
		cr->clip();

		cr->set_source_rgba(m_color_text[0], m_color_text[1], m_color_text[2], m_color_text[3]);
		
		std::vector<std::string> texts;
		
		utility::split(subtitle.get_text(), '\n', texts);

		for(unsigned int i=0; i<texts.size(); ++i)
		{
			cr->move_to(x, y + 20 * (1+ i));
			cr->show_text(texts[i]);
		}

		cr->restore();
	}
}

/*
 *
 */
void WaveformSystem::draw_markers(Cairo::RefPtr<Cairo::Context> &cr, const Gdk::Rectangle &rec, Document *doc)
{
	se_debug(SE_DEBUG_WAVEFORM);

	g_return_if_fail(doc);

	// on recupere le temps visible pour ne dessiner que les markers visibles
	//gint64 t_start = get_time_by_pos(get_start_area()) / GST_MSECOND;

	// recuperation des iter qui sont compris dans ce temps
	Subtitle selected = doc->subtitles().get_first_selected();
	if(selected)
	{
		for(Subtitle sub = doc->subtitles().get_first(); sub; ++sub)
		{
			//long s = sub.get_start().totalmsecs;
			//long e = sub.get_end().totalmsecs;

			if(sub == selected)
				draw_marker(cr, rec, sub, m_color_marker_hightlight);
			else
				draw_marker(cr, rec, sub, m_color_marker);
		}
	}
	else
	{
		for(Subtitle sub = doc->subtitles().get_first(); sub; ++sub)
		{
			//long s = sub.get_start().totalmsecs;
			//long e = sub.get_end().totalmsecs;

			draw_marker(cr, rec, sub, m_color_marker);
		}
	}
}

/*
 *
 */
void WaveformSystem::init_with_document(Document *doc)
{
	se_debug(SE_DEBUG_WAVEFORM);

	m_currentDocument = NULL;

	// disconnect previous document
	std::map<std::string, sigc::connection>::iterator it;
	
	for(it = m_connection.begin(); it!=m_connection.end(); ++it)
		it->second.disconnect();

	m_connection.clear();
	

	if(doc != NULL)
	{
		m_currentDocument = doc;

#define CONNECT(signal, callback) m_connection[signal] = doc->get_signal(signal).connect( \
			sigc::mem_fun(*this, &WaveformSystem::callback));

		CONNECT("subtitle-selection-changed", on_subtitle_selection_changed);
		CONNECT("subtitle-time-changed", on_subtitle_time_changed);

#undef CONNECT

		init_scrollbarWaveform();
	}

	redraw_marker();
}

/*
 *
 */
void WaveformSystem::on_subtitle_selection_changed()
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(!m_waveform)
		return;

	if(m_scrolling_with_selection && m_playerVideo && document())
	{
		if(!m_playerVideo->is_playing())
		{
			center_with_selected_subtitle();
		}
		else
			redraw_marker();
	}
	else
		redraw_marker();
}

/*
 *
 */
void WaveformSystem::on_subtitle_time_changed()
{
	se_debug(SE_DEBUG_WAVEFORM);

	redraw_marker();
}

/*
 *
 */
void WaveformSystem::on_video_timeout()
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(m_waveform)
	{
		update_view_from_cursor();

		m_drawingWaveform->queue_draw();
	}
}

void WaveformSystem::scroll_to_position(gint position)
{
	se_debug(SE_DEBUG_WAVEFORM);
/*
	if(position < get_start_area())
		m_hscrollbarWaveform->set_value(position);
	else if(position > get_end_area())
		m_hscrollbarWaveform->set_value(position);
*/
	if((position - 20) < get_start_area())
		m_hscrollbarWaveform->set_value(position - 20);
	else if((position + 20) > get_end_area())
		m_hscrollbarWaveform->set_value(position + 20);
}

/*
 *
 */
void WaveformSystem::scroll_to_position_and_center(gint position)
{
	se_debug_message(SE_DEBUG_WAVEFORM, "position=%d", position);

	int start_area = get_start_area();
	int end_area = get_end_area();

	double center_area = start_area + (end_area - start_area) * 0.5;

	double diff = position - center_area;

	m_hscrollbarWaveform->set_value(start_area + diff);
}

/*
 *	place la position de la scrollbar par rapport 
 *	a la lecture
 */
void WaveformSystem::update_view_from_cursor()
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(m_playerVideo == NULL || m_scrolling_with_cursor == false)
		return;

	gint position = __get_pos_by_time(m_playerVideo->get_position());

	scroll_to_position(position);
}


/*
 *
 */
void WaveformSystem::on_action_emit(const Glib::ustring &name)
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(name == "waveform/open")
		on_open_waveform();

	if(!m_waveform)
		return;

	if(name == "waveform/save")
		on_save_waveform();
	else if(name == "waveform/zoom-in")
		zoom_in();
	else if(name == "waveform/zoom-out")
		zoom_out();
	else if(name == "waveform/zoom-selection")
		zoom_selection();
	else if(name == "waveform/zoom-all")
		zoom_all();
	else if(name == "waveform/center-with-selected-subtitle")
		center_with_selected_subtitle();
}

/*
 *
 */
bool WaveformSystem::open(const Glib::ustring &uri)
{
	if(open_media(uri))
	{
		if(m_playerVideo)
		{
			// teste si la video est presente
			//if(Glib::file_test(Glib::filename_from_uri(m_waveform->get_video_uri()), Glib::FILE_TEST_EXISTS))
			{
				m_playerVideo->open(m_waveform->get_video_uri());
				return true;
			}
			//else
			{
#warning "FIXME: find the video for waveform"
			}
				//
		}
	}
	else
	{
		// generate waveform
		WaveformGeneratorUI ui(m_waveform, uri);
			
		if(m_waveform)
		{
			show();
			set_sensitive(true);
			m_hscrollbarWaveform->set_value(0);
			init_scrollbarWaveform();
			m_playerVideo->open(uri);
			return true;
		}
	}
	return false;
}
/*
 *
 */
void WaveformSystem::on_open_waveform()
{
	se_debug(SE_DEBUG_WAVEFORM);

	OpenWaveformUI ui;
	if(ui.run() == Gtk::RESPONSE_OK)
	{
		ui.hide();
		Glib::ustring uri = ui.get_uri();

		open(uri);

	}
}

void WaveformSystem::on_save_waveform()
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(m_waveform)
	{
		Gtk::FileChooserDialog ui(_("Save Waveform"), Gtk::FILE_CHOOSER_ACTION_SAVE);
		ui.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		ui.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
		ui.set_default_response(Gtk::RESPONSE_OK);

		if(ui.run() == Gtk::RESPONSE_OK)
		{
			Glib::ustring uri = ui.get_uri();

			m_waveform->save(uri);
		}
	}
}

void string_to_color_1(const Glib::ustring &value, float rgba[4])
{
	Color color(value);
	color.get_value(rgba, 1);
}

void WaveformSystem::on_config_waveform_changed(const Glib::ustring &key, const Glib::ustring &value)
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(key == "color-background")
		string_to_color_1(value, m_color_background);
	else if(key == "color-play-line")
		string_to_color_1(value, m_color_play_line);
	else if(key == "color-text")
		string_to_color_1(value, m_color_text);
	else if(key == "color-wave")
		string_to_color_1(value, m_color_wave);
	else if(key == "color-wave-fill")
		string_to_color_1(value, m_color_wave_fill);
	else if(key == "color-marker")
		string_to_color_1(value, m_color_marker);
	else if(key == "color-marker-hightlight")
		string_to_color_1(value, m_color_marker_hightlight);
	else if(key == "color-marker-invalid")
		string_to_color_1(value, m_color_marker_invalid);
	else if(key == "display-waveform-fill")
	{
		m_display_waveform_fill = utility::string_to_bool(value);
	}
	else if(key == "display-background")
	{
		m_display_background = utility::string_to_bool(value);
	}
	else if(key == "display-subtitle-text")
	{
		m_display_subtitle_text = utility::string_to_bool(value);
	}
	else if(key == "scrolling-with-cursor")
	{
		m_scrolling_with_cursor = utility::string_to_bool(value);
	}
	else if(key == "scrolling-with-selection")
	{
		m_scrolling_with_selection = utility::string_to_bool(value);
	}

	redraw_all();
}

/*
 *
 */
void WaveformSystem::zoom_in()
{
	set_zoom(m_zoom + 1);
}

/*
 *
 */
void WaveformSystem::zoom_out()
{
	set_zoom(m_zoom - 1);
}

/*
 *
 */
void WaveformSystem::zoom_selection()
{
	if(Document *doc = DocumentSystem::getInstance().getCurrentDocument())
	{
		Subtitle subtitle = doc->subtitles().get_first_selected();
		if(subtitle)
		{
			zoom_in();

			int start = __get_pos_by_time(subtitle.get_start().totalmsecs);
			int end   = __get_pos_by_time(subtitle.get_end().totalmsecs);

			// centre du sous-titre
			int subtitle_center = start + int((end - start) * 0.5);

			scroll_to_position_and_center(subtitle_center);
		}
	}
}

/*
 *
 */
void WaveformSystem::zoom_all()
{
	set_zoom(1);
}

/*
 *	centre l'area sur le sous-titre selectionner
 */
void WaveformSystem::center_with_selected_subtitle()
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(!document())
		return;

	Subtitle subtitle = document()->subtitles().get_first_selected();
	if(!subtitle)
		return;

	int start	= __get_pos_by_time(subtitle.get_start().totalmsecs);
	int end		= __get_pos_by_time(subtitle.get_end().totalmsecs);

	int center = start + int((end - start) * 0.5);

	if(start != 0 && end != 0)
		scroll_to_position_and_center(center);
	
	redraw_marker();
}

/*
 *
 */
void WaveformSystem::on_slider_zoom_changed()
{
	double value = m_sliderZoom->get_value();

	set_zoom((int)value);
}

void WaveformSystem::on_slider_scale_changed()
{
	double value = m_sliderScale->get_value();

	set_scale((int)value);
}

/*
 *
 */
bool WaveformSystem::on_slider_scroll_event(GdkEventScroll *ev, Gtk::Scale *scale)
{
	double value = scale->get_value();
	if(ev->direction == GDK_SCROLL_UP)
		scale->set_value(value + 1);
	else if(ev->direction == GDK_SCROLL_DOWN)
		scale->set_value(value - 1);

	return true;
}


