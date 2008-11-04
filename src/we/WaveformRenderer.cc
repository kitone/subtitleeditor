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

#include "WaveformRenderer.h"
#include "utility.h"
#include <gst/gst.h>

/*
 *
 */
WaveformRenderer::WaveformRenderer()
{
	init_default_config();
	load_config();

	Config::getInstance().signal_changed("waveform-renderer").connect(
			sigc::mem_fun(*this, &WaveformRenderer::on_config_waveform_renderer_changed));
}

/*
 *
 */
WaveformRenderer::~WaveformRenderer()
{
}

/*
 * Initialize the default value like colors.
 */
void WaveformRenderer::init_default_config()
{
#define SET_COLOR(x, r,g,b,a) x[0]=r; x[1]=g; x[2]=b; x[3]=a;

	SET_COLOR(m_color_player_position, 1, 1, 1, 1);
	SET_COLOR(m_color_background, 0.3, 0.3, 0.3, 1);
	SET_COLOR(m_color_wave, 0.6, 0.8, 0.3, 1);
	SET_COLOR(m_color_wave_fill, 1, 1, 1, 1);
	SET_COLOR(m_color_subtitle, .6, 0.3, 0.1, 0.6);
	SET_COLOR(m_color_subtitle_selected, 0.9, 0.5, 0.3, 0.6);
	SET_COLOR(m_color_subtitle_invalid, 1, 1, 0.0, 0.8); // invalid time start > end
	SET_COLOR(m_color_text, 1, 1, 1, 1);

#undef SET_COLOR

	m_display_time_info = false;
	m_display_subtitle_text = true;

#warning "TODO: FIXME with ConfigChecker"
	Config &cfg = Config::getInstance();

#define check_color(key, rgba) if(!cfg.has_key("waveform-renderer", key)) { Color col; col.set_value(rgba, 1); cfg.set_value_string("waveform-renderer", key, col.to_string()); }
#define check_bool(key, value) if(!cfg.has_key("waveform-renderer", key)) cfg.set_value_bool("waveform-renderer", key, value);
	
	check_bool("display-subtitle-text", m_display_subtitle_text);

	check_color("color-background", m_color_background);
	check_color("color-wave", m_color_wave);
	check_color("color-wave-fill", m_color_wave_fill);
	check_color("color-subtitle", m_color_subtitle);
	check_color("color-subtitle-selected", m_color_subtitle_selected);
	check_color("color-subtitle-invalid", m_color_subtitle_invalid);
	check_color("color-text", m_color_text);
	check_color("color-player-position", m_color_player_position);


#undef check_color
#undef check_bool
}

/*
 *
 */
void WaveformRenderer::load_config()
{
	Config &cfg = Config::getInstance();

	m_display_subtitle_text = cfg.get_value_bool("waveform-renderer", "display-subtitle-text");

#define get_color(key, col) cfg.get_value_color("waveform-renderer", key).get_value(col, 1)

	get_color("color-background", m_color_background);
	get_color("color-wave", m_color_wave);
	get_color("color-wave-fill", m_color_wave_fill);
	get_color("color-subtitle", m_color_subtitle);
	get_color("color-subtitle-selected", m_color_subtitle_selected);
	get_color("color-subtitle-invalid", m_color_subtitle_invalid);
	get_color("color-text", m_color_text);
	get_color("color-player-position", m_color_player_position);

#undef get_color
}

/*
 *
 */
void WaveformRenderer::set_waveform(const Glib::RefPtr<Waveform> &wf)
{
	m_waveform = wf;

	waveform_changed();
}

/*
 * This function is call when the waveform is changed.
 * Like a new Waveform.
 */
void WaveformRenderer::waveform_changed()
{
}

/*
 *
 */
void WaveformRenderer::redraw_all()
{
}

/*
 *
 */
void WaveformRenderer::force_redraw_all()
{
}

/*
 *
 */
int WaveformRenderer::get_start_area()
{
	return scrolling();
}

/*
 *
 */
int WaveformRenderer::get_end_area()
{
	return get_start_area() + widget()->get_width();
}

/*
 * return the time of the position in the area
 * time is in msec (SubtitleTime.totalmsecs)
 */
long WaveformRenderer::get_time_by_pos(int pos)
{
	float width = (float)widget()->get_width() * zoom();
	float percent = ((float)pos / width);
	float gsttime = ((float)m_waveform->get_duration() * percent);
	return SubtitleTime(long(gsttime / GST_MSECOND)).totalmsecs;
}

/*
 * return the position of the time in the area
 */
int WaveformRenderer::get_pos_by_time(long msec)
{
	gint64 gsttime = msec * GST_MSECOND;
	float percent = ((float)gsttime / (float)m_waveform->get_duration());
	float width = (float)widget()->get_width() * zoom();
	float pos = width * percent;
	pos = CLAMP(pos, 0, width);
	return (int)pos;
}

/*
 *	return the position in the area with scrolling support
 */
int WaveformRenderer::get_mouse_coords(int x)
{
	return get_start_area() + x;
}

/*
 *
 */
long WaveformRenderer::get_mouse_time(int x)
{
	return get_time_by_pos(get_mouse_coords(x));
}

/*
 *
 */
sigc::signal<Document*>& WaveformRenderer::signal_document()
{
	return document;
}

/*
 *
 */
sigc::signal<int>& WaveformRenderer::signal_zoom()
{
	return zoom;
}

/*
 *
 */
sigc::signal<float>& WaveformRenderer::signal_scale()
{
	return scale;
}

/*
 *
 */
sigc::signal<int>& WaveformRenderer::signal_scrolling()
{
	return scrolling;
}

/*
 *
 */
void WaveformRenderer::on_config_waveform_renderer_changed(const Glib::ustring &key, const Glib::ustring &value)
{
#define string_to_rgba(string, col) Color(string).get_value(col, 1)
	
	if("display-subtitle-text" == key)
	{
		m_display_subtitle_text = utility::string_to_bool(value);
	}
	else if("color-background" == key)
	{
		string_to_rgba(value, m_color_background);
	}
	else if("color-wave" == key)
	{
		string_to_rgba(value, m_color_wave);
	}
	else if("color-wave-fill" == key)
	{
		string_to_rgba(value, m_color_wave_fill);
	}
	else if("color-subtitle" == key)
	{
		string_to_rgba(value, m_color_subtitle);
	}
	else if("color-subtitle-selected" == key)
	{
		string_to_rgba(value, m_color_subtitle_selected);
	}
	else if("color-subtitle-invalid" == key)
	{
		string_to_rgba(value, m_color_subtitle_invalid);
	}
	else if("color-text" == key)
	{
		string_to_rgba(value, m_color_text);
	}
	else if("color-player-position" == key)
	{
		string_to_rgba(value, m_color_player_position);
	}

#undef string_to_rgba

	force_redraw_all();
}

