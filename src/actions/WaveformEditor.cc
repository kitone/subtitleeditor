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

#include "WaveformEditor.h"
#include "utility.h"
#include "DocumentSystem.h"
#include "SubtitleEditorWindow.h"

/*
 * TODO: Middle button play at cursor
 */

/*
 *	HACK!
 */
WaveformRenderer* create_waveform_renderer_gl();


/*
 *
 */
WaveformEditor::WaveformEditor()
:Gtk::HBox(false, 0), m_waveformRenderer(NULL), m_document(NULL), m_player(NULL)
{
	set_size_request(240, 240);

	// VBox for Renderer + scrollbar
	Gtk::VBox *vbox = manage(new Gtk::VBox(false, 0));
	pack_start(*vbox, true, true);

	m_frameDrawingArea = manage(new Gtk::Frame);
	m_frameDrawingArea->set_shadow_type(Gtk::SHADOW_IN);
	vbox->pack_start(*m_frameDrawingArea, true, true);

	m_hscrollbarWaveform = manage(new Gtk::HScrollbar);
	vbox->pack_start(*m_hscrollbarWaveform, false, false);
	
	vbox->show_all();


	// Zoom and Scale widget
	pack_start(*create_control_widget(), false, false);

	// connect signal
	
	// init the scrollbar with the size of widget
	m_frameDrawingArea->signal_configure_event().connect(
			sigc::mem_fun(*this, &WaveformEditor::on_configure_event_frame_waveform));

	m_hscrollbarWaveform->signal_value_changed().connect(
			sigc::mem_fun(*this, &WaveformEditor::on_scrollbar_value_changed));

	// create the renderer after the widget is realized.
	signal_realize().connect(
			sigc::mem_fun(*this, &WaveformEditor::on_create_renderer), true);

	DocumentSystem::getInstance().signal_current_document_changed().connect(
			sigc::mem_fun(*this, &WaveformEditor::init_document));

	// FIXME init
	{
		m_cfg_scrolling_with_player = true;
		m_cfg_scrolling_with_selection = true;
		//m_cfg_respect_min_display = true;
		//m_cfg_respect_gab_between_subtitles = true;
		m_cfg_respect_timing = true;
	}

	load_config();

	Config::getInstance().signal_changed("waveform").connect(
			sigc::mem_fun(*this, &WaveformEditor::on_config_waveform_changed));
	
	set_sensitive(false);
}

/*
 *
 */
WaveformEditor::~WaveformEditor()
{
	se_debug(SE_DEBUG_WAVEFORM);
}

/*
 *
 */
Gtk::Widget* WaveformEditor::create_control_widget()
{
	Gtk::Adjustment *adj_zoom = new Gtk::Adjustment(1, 1, 1000, 1, 10, 10);
	Gtk::Adjustment *adj_scale = new Gtk::Adjustment(1, 0.10, 10, 0.10, 0.10, 0.10);

	Gtk::HBox* box = manage(new Gtk::HBox(false, 3));

	// slider zoom
	m_sliderZoom = manage(new Gtk::VScale(*adj_zoom));
	m_sliderZoom->set_inverted(true);
	m_sliderZoom->set_draw_value(false);

	// slider scale
	m_sliderScale = manage(new Gtk::VScale(*adj_scale));
	m_sliderScale->set_inverted(true);
	m_sliderScale->set_draw_value(false);

	// signals
	m_sliderZoom->signal_value_changed().connect(
			sigc::mem_fun(*this, &WaveformEditor::on_zoom_changed));

	m_sliderScale->signal_value_changed().connect(
			sigc::mem_fun(*this, &WaveformEditor::on_scale_changed));

	// add widgets
	box->pack_start(*m_sliderZoom, false, false);
	box->pack_start(*m_sliderScale, false, false);

	box->show_all();
	return box;
	/*
	Gtk::VBox* box = manage(new Gtk::VBox(false, 3));

	// zoom
	std::list<Glib::ustring> zoom_icons;

	m_sliderZoom = manage(new Gtk::ScaleButton(Gtk::ICON_SIZE_BUTTON, 1, 1000, 1, zoom_icons));
	m_sliderZoom->set_adjustment(*adj_zoom);

	box->pack_start(*m_sliderZoom, false, false);

	m_sliderZoom->get_adjustment()->signal_value_changed().connect(
			sigc::mem_fun(*this, &WaveformEditor::on_zoom_changed));

	// scale
	std::list<Glib::ustring> scale_icons;
	scale_icons.push_back("gtk-zoom-fit");

	m_sliderScale = manage(new Gtk::ScaleButton(Gtk::ICON_SIZE_BUTTON, 1, 10, 1, scale_icons));
	m_sliderScale->set_adjustment(*adj_scale);

	box->pack_start(*m_sliderScale, false, false);

	m_sliderScale->get_adjustment()->signal_value_changed().connect(
			sigc::mem_fun(*this, &WaveformEditor::on_scale_changed));

	box->show_all();
	return box;
	*/
}

/*
 *
 */
void WaveformEditor::load_config()
{
	se_debug(SE_DEBUG_WAVEFORM);

	Config &cfg = Config::getInstance();

	if(cfg.has_key("waveform", "scale"))
		m_sliderScale->set_value(cfg.get_value_int("waveform", "scale"));

	if(cfg.has_key("waveform", "zoom"))
		m_sliderZoom->set_value(cfg.get_value_int("waveform", "zoom"));

	cfg.get_value_bool("waveform", "scrolling-with-player", m_cfg_scrolling_with_player);
	cfg.get_value_bool("waveform", "scrolling-with-selection", m_cfg_scrolling_with_selection);
	cfg.get_value_bool("waveform", "respect-timing", m_cfg_respect_timing);

	if(cfg.get_value_bool("waveform", "display"))
		show();
	else
		hide();
}

/*
 * This callback is connected on the realize signal.
 * It's used to create the renderer because some need a realized parent.
 */
void WaveformEditor::on_create_renderer()
{
#warning "FIXME: load config for renderer"
	Glib::ustring renderer_name = Config::getInstance().get_value_string("waveform", "renderer");

	if(renderer_name == "gl")
		init_renderer(create_waveform_renderer_gl());
	else
		init_renderer(create_waveform_renderer_gl());
}

/*
 * Initialize the editor with the document.
 * This callback is also connected at 
 * "DocumentSystem::signal_current_document_changed"
 */
void WaveformEditor::init_document(Document *doc)
{
	m_document = NULL;
	for(unsigned int i=0; i<m_document_connection.size(); ++i)
		m_document_connection[i].disconnect();
	m_document_connection.clear();

	if(doc != NULL)
	{
		m_document = doc;

#define CONNECT(signal, callback) \
		m_document_connection.push_back( doc->get_signal(signal).connect( \
			sigc::mem_fun(*this, &WaveformEditor::callback)));

		CONNECT("subtitle-selection-changed", on_subtitle_selection_changed);
		CONNECT("subtitle-time-changed", on_subtitle_time_changed);

#undef CONNECT

		init_scrollbar();
	}

	if(has_renderer())
		renderer()->redraw_all();
}

/*
 * Initializes the signals of the renderer like the button pressed, released ...
 * Add events to the widget.
 */
void WaveformEditor::init_renderer(WaveformRenderer *renderer)
{
	if(m_waveformRenderer != NULL)
	{
		//remove old
#warning "FIXME: remove old renderer"
		m_waveformRenderer = NULL;
	}

	if(renderer)
	{
		m_waveformRenderer = renderer;

		renderer->signal_document().connect(
				sigc::mem_fun(*this, &WaveformEditor::document));

		renderer->signal_scale().connect(
				sigc::mem_fun(*this, &WaveformEditor::get_scale));

		renderer->signal_zoom().connect(
				sigc::mem_fun(*this, &WaveformEditor::get_zoom));

		renderer->player_time.connect(
				sigc::mem_fun(*this, &WaveformEditor::get_player_time));

		renderer->set_waveform(get_waveform());

		Gtk::Widget *widget = dynamic_cast<Gtk::Widget*>(renderer);
		
		m_frameDrawingArea->add(*widget);

		widget->add_events(
			Gdk::BUTTON_PRESS_MASK | 
			Gdk::BUTTON_RELEASE_MASK | 
			Gdk::BUTTON_MOTION_MASK | 
			Gdk::SCROLL_MASK);

		widget->signal_button_press_event().connect(
				sigc::mem_fun(*this, &WaveformEditor::on_button_press_event_renderer));

		widget->signal_button_release_event().connect(
				sigc::mem_fun(*this, &WaveformEditor::on_button_release_event_renderer));

		widget->signal_motion_notify_event().connect(
				sigc::mem_fun(*this, &WaveformEditor::on_motion_notify_event_renderer));

		widget->signal_scroll_event().connect(
				sigc::mem_fun(*this, &WaveformEditor::on_scroll_event_renderer));

		init_document(document());

		widget->show();
	}
}

/*
 *
 */
void WaveformEditor::set_player(Player *player)
{
	m_player = player;

	// init
	m_connection_timeout = m_player->get_signal_timeout().connect(
			sigc::mem_fun(*this, &WaveformEditor::on_player_timeout));
}

/*
 *
 */
Player* WaveformEditor::player()
{
	if(m_player == NULL)
	{
		Player *player = SubtitleEditorWindow::get_instance()->get_player();
		
		if(player != NULL)
			set_player(player);
	}
	return m_player;
}

/*
 * Enable the signal timeout (Player)
 */
void WaveformEditor::on_map()
{
	Gtk::HBox::on_map();

	if(m_connection_timeout)
	{
		if(m_connection_timeout.connected())
			m_connection_timeout.unblock();
	}

	if(has_renderer())
		renderer()->redraw_all();
}

/*
 * Disable the signal timeout (Player)
 */
void WaveformEditor::on_unmap()
{
	Gtk::HBox::on_unmap();

	if(m_connection_timeout)
	{
		if(m_connection_timeout.connected())
			m_connection_timeout.block();
	}
}

/*
 *
 */
void WaveformEditor::on_player_timeout()
{
	if(has_renderer() && player())
	{
		scroll_with_player();

		renderer()->redraw_all();
	}
}

/*
 * Return the current time of the player.
 */
long WaveformEditor::get_player_time()
{
	if(player())
	{
		if(player()->is_valid())
			return player()->get_position();
	}
	return 0;
}

/*
 * Set the value of the scale (widget).
 */
void WaveformEditor::set_scale(float value)
{
	m_sliderScale->set_value(value);
}

/*
 * Return the value of the scale (widget).
 */
float WaveformEditor::get_scale()
{
	return (float)m_sliderScale->get_value();
}

/*
 * Set the value of the zoom (widget).
 */
void WaveformEditor::set_zoom(int value)
{
	m_sliderZoom->set_value(value);
}

/*
 * Return the value of the zoom (widget).
 */
int WaveformEditor::get_zoom()
{
	return (int)m_sliderZoom->get_value();
}

/*
 * The scroll bar depend on the size of the waveform widget.
 * This callback is connected to the signal "configure" of the waveform frame (Gtk::Frame).
 * Every time this size changed, the scrollbar need to be recalculate.
 */
bool WaveformEditor::on_configure_event_frame_waveform(GdkEventConfigure *ev)
{
	se_debug(SE_DEBUG_WAVEFORM);

	// init scrollbar
	init_scrollbar();
	
	if(m_waveformRenderer)
		m_waveformRenderer->redraw_all();
	return true;
}

/*
 * Initialize the scrollbar depending 
 * on the size of the widget renderer (waveform) 
 * and the value of the zoom.
 */
void WaveformEditor::init_scrollbar()
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(!has_renderer())
		return;

	guint width = dynamic_cast<Gtk::Widget*>(renderer())->get_width();

	int zoom = (int)m_sliderZoom->get_value();

	double upper = m_hscrollbarWaveform->get_adjustment()->get_upper();
	double old_value = m_hscrollbarWaveform->get_value();

	Gtk::Adjustment *adj = m_hscrollbarWaveform->get_adjustment();

	adj->set_page_size((double)width);
	adj->set_page_increment(width);
	adj->set_step_increment(width);

	m_hscrollbarWaveform->set_range(0, width * zoom);

	if(upper > 0)
		m_hscrollbarWaveform->set_value(width * zoom * (old_value / upper));

	se_debug_message(SE_DEBUG_WAVEFORM, "width=%d zoom=%d", width, zoom);
}

/*
 * The value of the scrollbar has changed.
 * Update the waveform renderer with the new value.
 */
void WaveformEditor::on_scrollbar_value_changed()
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(has_renderer())
	{
		m_waveformRenderer->set_start_area((int)m_hscrollbarWaveform->get_value());
		m_waveformRenderer->redraw_all();
	}
}

/*
 * The value of the zoom has changed.
 * Call init_scrollbar and updates the config.
 * Redraw the waveform.
 */
void WaveformEditor::on_zoom_changed()
{
	int value = (int)m_sliderZoom->get_value();

	//utility::clamp(value, 1, 1000);

	se_debug_message(SE_DEBUG_WAVEFORM, "zoom=%d", value);

	init_scrollbar();

	if(Config::getInstance().get_value_int("waveform", "zoom") != value)
		Config::getInstance().set_value_int("waveform", "zoom", value);

	if(has_renderer())
		renderer()->redraw_all();
}

/*
 * The value of the scale has changed.
 * Redraw the waveform.
 */
void WaveformEditor::on_scale_changed()
{
	double value = m_sliderScale->get_value();

	se_debug_message(SE_DEBUG_WAVEFORM, "scale=%f", value);

	if(has_renderer())
		renderer()->redraw_all();
}

/*
 * Try to open a waveform file and show or hide the editor.
 */
bool WaveformEditor::open_waveform(const Glib::ustring &uri)
{
	if(m_waveform)
	{
		// FIXME
		// clear
	}

	Glib::RefPtr<Waveform> wf = Waveform::create_from_file(uri);

	set_waveform(wf);
	
	return (bool)wf;
}

/*
 *
 */
void WaveformEditor::set_waveform(const Glib::RefPtr<Waveform> &wf)
{
	m_waveform = wf;

	if(has_renderer())
		renderer()->set_waveform(wf);
	else
		std::cerr << "You need a WaveformRenderer!!" << std::endl;

	set_sensitive((bool)wf && has_renderer());

	Config::getInstance().set_value_bool("waveform", "display", (bool)wf);

	m_signal_waveform_changed.emit();
}

/*
 * Return the state of waveform. Can be NULL.
 */
bool WaveformEditor::has_waveform()
{
	return (bool)get_waveform();
}

/*
 * Return a pointer to the waveform. Can be NULL.
 */
Glib::RefPtr<Waveform> WaveformEditor::get_waveform()
{
	return m_waveform;
}

/*
 *
 */
sigc::signal<void>& WaveformEditor::signal_waveform_changed()
{
	return m_signal_waveform_changed;
}

/*
 * The editor has a renderer ?
 */
bool WaveformEditor::has_renderer()
{
	return renderer() != NULL;
}

/*
 * Return the renderer. Can be NULL.
 */
WaveformRenderer* WaveformEditor::renderer()
{
	return m_waveformRenderer;
}


/*
 * Return the state of current document.
 */
bool WaveformEditor::has_document()
{
	return document() != NULL;
}

/*
 * Return a pointer to the current document. Can be NULL.
 */
Document* WaveformEditor::document()
{
	return m_document;
}

/*
 * This callback is connected at the current document. 
 * It's call when the selection of the subtitles has changed.
 * The view is centered with the new selection if the option is enable.
 * It's need to redraw the view.
 */
void WaveformEditor::on_subtitle_selection_changed()
{
	if((has_renderer() && has_waveform() && has_document()) == false)
		return;

	bool player_playing = (player() ? player()->is_playing() : false);
	
	if(m_cfg_scrolling_with_selection  && player_playing == false)
		center_with_selected_subtitle();

	renderer()->redraw_all();
}

/*
 * This callback is connected at the current document. 
 * The time of subtitle has changed, it's need to redraw the view.
 */
void WaveformEditor::on_subtitle_time_changed()
{
	if((has_renderer() && has_waveform()) == false)
		return;
	
	renderer()->redraw_all();
}

/*
 *
 */
void WaveformEditor::scroll_to_position(int position)
{
	if(!has_renderer())
		return;

	int start_area = renderer()->get_start_area();
	int end_area = renderer()->get_end_area();

	int margin = 20;

	if((position - margin) < start_area)
		m_hscrollbarWaveform->set_value(position - margin);
	else if((position + margin) > end_area)
		m_hscrollbarWaveform->set_value(position + margin);
}

/*
 * Go at the position on the scrollbar and 
 * try to place at the center of the view.
 */
void WaveformEditor::scroll_to_position_and_center(int position)
{
	if(!has_renderer())
		return;

	int start_area = renderer()->get_start_area();
	int end_area = renderer()->get_end_area();

	double center_area = start_area + (end_area - start_area) * 0.5;

	double diff = position - center_area;

	m_hscrollbarWaveform->set_value(start_area + diff);
}

/*
 * If scrolling with player is enabled, 
 * scroll with the current time of the player.
 */
void WaveformEditor::scroll_with_player()
{
	if(player() && has_renderer() && m_cfg_scrolling_with_player)
	{
		int position = renderer()->get_pos_by_time(player()->get_position());

		scroll_to_position(position);
	}
}

/*
 * Try to display the current subtitle at the center of the view.
 */
void WaveformEditor::center_with_selected_subtitle()
{
	if(!document())
		return;

	Subtitle subtitle = document()->subtitles().get_first_selected();
	if(!subtitle)
		return;

	int start = renderer()->get_pos_by_time(subtitle.get_start().totalmsecs);
	int end = renderer()->get_pos_by_time(subtitle.get_end().totalmsecs);

	if(start != 0 && end != 0)
	{
		int middle = start + int((end - start) * 0.5);

		scroll_to_position_and_center(middle);
	}
}

/*
 * Increment the zoom
 */
void WaveformEditor::zoom_in()
{
	set_zoom( get_zoom() + 1);
}

/*
 * Decrement the zoom
 */
void WaveformEditor::zoom_out()
{
	set_zoom( get_zoom() - 1);
}

/*
 * Décrément completely the zoom
 */
void WaveformEditor::zoom_all()
{
	set_zoom(1);
}

/*
 * Zooming on the current subtitle.
 */
void WaveformEditor::zoom_selection()
{
	zoom_in();

	center_with_selected_subtitle();
}


/*
 * Edit the position of the current subtitle.
 * Start the recorder command.
 */
bool WaveformEditor::on_button_press_event_renderer(GdkEventButton *ev)
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(!has_renderer())
		return true;

	// the time of the mouse in the area
	SubtitleTime time(m_waveformRenderer->get_mouse_time((gint)ev->x));
/*
	if(ev->button == 2 && !(ev->state & Gdk::CONTROL_MASK) && player())
	{
		if(player()->is_valid())
		{
			player()->seek(time);
			if(player()->is_playing() == false)
				player()->play();
		}
		return;
	}
*/
	if(!has_document())
		return true;

	// try to select subtitle
	if(ev->button == 2 && ev->state & Gdk::CONTROL_MASK)
	{
		Subtitle sub = document()->subtitles().find(time);
		if(sub)
			document()->subtitles().select(sub);

		m_waveformRenderer->redraw_all();
		return true;
	}

	// now actions with document and subtitle
	if(!has_document())
		return true;

	Subtitle subtitle = document()->subtitles().get_first_selected();
	if(!subtitle)
		return true;

	if(ev->button == 1)
	{
		document()->start_command(_("Editing position"));
		move_subtitle_start(time, (ev->state & Gdk::SHIFT_MASK), (ev->state & Gdk::CONTROL_MASK));
	}
	else if(ev->button == 3)
	{
		document()->start_command(_("Editing position"));
		move_subtitle_end(time, (ev->state & Gdk::SHIFT_MASK),  (ev->state & Gdk::CONTROL_MASK));
	}

	renderer()->m_display_time_info = true;
	
	return false;
}

/*
 * Finish the editing of the current subtitle. 
 * Stop the recorder command.
 */
bool WaveformEditor::on_button_release_event_renderer(GdkEventButton *ev)
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(!(has_renderer() && has_document()))
		return true;

	document()->finish_command();

	renderer()->m_display_time_info = false;
	renderer()->redraw_all();

	return true;
}

/*
 * Adjust the position of the current subtitle.
 */
bool WaveformEditor::on_motion_notify_event_renderer(GdkEventMotion *ev)
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(!(has_renderer() && has_document()))
		return true;

	Subtitle subtitle = document()->subtitles().get_first_selected();
	if(!subtitle)
		return true;

	SubtitleTime time = m_waveformRenderer->get_mouse_time((int)ev->x);


	if((ev->state & Gdk::BUTTON1_MASK))
	{
		move_subtitle_start(time, (ev->state & Gdk::SHIFT_MASK),  (ev->state & Gdk::CONTROL_MASK));
	}	
	else if((ev->state & Gdk::BUTTON3_MASK))
	{
		move_subtitle_end(time, (ev->state & Gdk::SHIFT_MASK),  (ev->state & Gdk::CONTROL_MASK));
	}	

	renderer()->redraw_all();
	return true;
}

/*
 * Manage the scrolling like the current position in the view (like scrollbar), 
 * the scale or the zoom.
 */
bool WaveformEditor::on_scroll_event_renderer(GdkEventScroll *ev)
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(!(has_waveform() && has_renderer()))
		return true;
	
	int value = 0;

	if(ev->direction == GDK_SCROLL_UP)
		value = 1;
	else if(ev->direction == GDK_SCROLL_DOWN)
		value = -1;
	else
		return true;

	if(ev->state & Gdk::SHIFT_MASK)	// Scale
	{
		set_scale(get_scale() + value);
	}
	else if(ev->state & Gdk::CONTROL_MASK) // Zoom
	{
		int center_area = renderer()->get_start_area() + (int)((renderer()->get_end_area() - renderer()->get_start_area()) * 0.5);

		long time = renderer()->get_time_by_pos(center_area);

		set_zoom(get_zoom() + value);

		scroll_to_position_and_center(renderer()->get_pos_by_time(time));
	}
	else // Scrolling like scrollbar
	{
		double page_size = m_hscrollbarWaveform->get_adjustment()->get_page_size();
		double delta = pow(page_size, 2.0 / 3.0);
		m_hscrollbarWaveform->set_value(m_hscrollbarWaveform->get_value() - delta * value);
	}

	return true;
}

/*
 * Try to move the beginning of the current subtitle.
 * If the option 'respect-timing' is enabled, 
 * try to respect the timing preferences.
 *
 * disable_respect:
 *	No test is doing if 'disable_respect' is enabled.
 *
 * around:
 *	If is true, the end of the previous subtitle can be moved 
 *	if necessary with respect of timing preferences.
 */
bool WaveformEditor::move_subtitle_start(const SubtitleTime &_time, bool disable_respect, bool around)
{
	if(!(has_renderer() && has_document()))
		return false;

	Subtitle subtitle = document()->subtitles().get_first_selected();

	if(!subtitle)
		return false;

	SubtitleTime min_gap( Config::getInstance().get_value_int("timing", "min-gap-between-subtitles") );
	SubtitleTime min_display ( Config::getInstance().get_value_int("timing", "min-display") );

	SubtitleTime time = _time;

	// clamp [0:]
	if(time.totalmsecs < 0)
		time = SubtitleTime();

	SubtitleTime diff = time - subtitle.get_start();

	// this is the start of the current subtitle
	SubtitleTime subtitle_start = time;

	if(!disable_respect)
	{
		// Check the min display respect of the current subtitle
		// Clamped if need
		//if(m_cfg_respect_min_display)
		if(m_cfg_respect_timing)
		{
			SubtitleTime duration = subtitle.get_end() - subtitle_start;
			if(duration < min_display)
				subtitle_start = subtitle.get_end() - min_display;
		}

		// check the timing respect with the previous subtitle
		// Respect just the gap between subtitle
		// the start of the current subtitle is clamped if need
		//if(m_cfg_respect_gab_between_subtitles)
		if(m_cfg_respect_timing)
		{
			Subtitle previous = document()->subtitles().get_previous(subtitle);
			if(previous)
			{
				SubtitleTime end_of_previous_sub = previous.get_end();

				if(subtitle_start < end_of_previous_sub + min_gap)
				{
					// if around is enable
					// try to move the end of the previous subtitle
					if(around)
					{
						SubtitleTime new_previous_end = subtitle_start - min_gap;
							
						// try to move the end of the previous
						{
							// check first if after move is respect the min display
							if(new_previous_end - previous.get_start() > min_display) 
							{
								previous.set_end(new_previous_end); 
							}
							else // I can, move with gap respect
							{
								new_previous_end = previous.get_start() + min_display;

								previous.set_end(new_previous_end);

								subtitle_start = new_previous_end + min_gap;
							}
						}
					}
					else // around is disable, clamp at the end of the previous with gap respect
						subtitle_start = end_of_previous_sub + min_gap;
				}
			}
		}//m_cfg_respect_timing && gab between subtitles
	}//!disable_respect

	if(subtitle.get_start() != subtitle_start)
		subtitle.set_start(subtitle_start);
	
	document()->emit_signal("subtitle-time-changed");
	return true;
}


/*
 * Try to move the end of the current subtitle.
 * If the option 'respect-timing' is enabled, 
 * try to respect the timing preferences.
 *
 * disable_respect:
 *	No test is doing if 'disable_respect' is enabled.
 *
 * around:
 *	If is true, the beginning of the next subtitle can be moved 
 *	if necessary with respect of timing preferences.
 */
bool WaveformEditor::move_subtitle_end(const SubtitleTime &_time, bool disable_respect, bool around)
{
	if(!(has_renderer() && has_document()))
		return false;

	Subtitle subtitle = document()->subtitles().get_first_selected();

	if(!subtitle)
		return false;

	SubtitleTime min_gap( Config::getInstance().get_value_int("timing", "min-gap-between-subtitles") );
	SubtitleTime min_display ( Config::getInstance().get_value_int("timing", "min-display") );

	SubtitleTime time = _time;

	// clamp [0:]
	if(time.totalmsecs < 0)
		time = SubtitleTime();

	SubtitleTime diff = time - subtitle.get_end();

	// this is the end of the current subtitle
	SubtitleTime subtitle_end = time;

	if(!disable_respect)
	{
		// Check the min display respect of the current subtitle
		// Clamped if need
		//if(m_cfg_respect_min_display)
		if(m_cfg_respect_timing)
		{
			SubtitleTime duration = subtitle_end  - subtitle.get_start();
			if(duration < min_display)
				subtitle_end = subtitle.get_start() + min_display;
		}

		// check the timing respect with the next subtitle
		// Respect just the gap between subtitle
		// the end of the current subtitle is clamped if need
		//if(m_cfg_respect_gab_between_subtitles)
		if(m_cfg_respect_timing)
		{
			Subtitle next = document()->subtitles().get_next(subtitle);
			if(next)
			{
				SubtitleTime start_of_next_sub = next.get_start();

				if(subtitle_end + min_gap > start_of_next_sub)
				{
					// if around is enable
					// try to move the start of the next subtitle
					if(around)
					{
						SubtitleTime new_next_start = subtitle_end + min_gap;
							
						// try to move the start of the next
						{
							if(next.get_end() - new_next_start > min_display) 
							{
								next.set_start(new_next_start); 
							}
							else // I can, move with gap respect
							{
								new_next_start = next.get_end() - min_display;

								next.set_start(new_next_start);

								subtitle_end = new_next_start - min_gap;
							}
						}
					}
					else // around is disable, clamp at the start of the next with gap respect
						subtitle_end = start_of_next_sub - min_gap;
				}
			}
		}
	}

	if(subtitle.get_end() != subtitle_end)
		subtitle.set_end(subtitle_end);
	
	document()->emit_signal("subtitle-time-changed");
	return true;
}


/*
 *
 */
void WaveformEditor::on_config_waveform_changed(const Glib::ustring &key, const Glib::ustring &value)
{
	if(key == "scrolling-with-player")
		m_cfg_scrolling_with_player = utility::string_to_bool(value);
	else if(key == "scrolling-with-selection")
		m_cfg_scrolling_with_selection = utility::string_to_bool(value);
	else if(key == "respect-timing")
		m_cfg_respect_timing = utility::string_to_bool(value);
	//else if(key == "respect-min-display")
	//	m_cfg_respect_min_display = utility::string_to_bool(value);
	//else if(key == "respect-gap-between-subtitles")
	//	m_cfg_respect_gab_between_subtitles = utility::string_to_bool(value);
	else if(key == "display")
	{
		bool state = utility::string_to_bool(value);
		
		if(state)
			show();
		else
			hide();
	}
}