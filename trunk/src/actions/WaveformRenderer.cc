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

/*
 *
 */
WaveformRenderer::WaveformRenderer(Gtk::DrawingArea *area)
:m_drawingArea(area)
{
	m_start_area = 0;
	m_display_time_info = false;
}

/*
 *
 */
WaveformRenderer::~WaveformRenderer()
{
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
bool WaveformRenderer::on_expose_event(GdkEventExpose *ev)
{
	return false;
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
void WaveformRenderer::set_start_area(int value)
{
	m_start_area = value;
}

/*
 *
 */
int WaveformRenderer::get_start_area()
{
	return m_start_area;
}

/*
 *
 */
int WaveformRenderer::get_end_area()
{
	return get_start_area() + m_drawingArea->get_width();
}

/*
 * return the time of the position in the area
 * time is in msec (SubtitleTime.totalmsecs)
 */
long WaveformRenderer::get_time_by_pos(int pos)
{
	float width = (float)m_drawingArea->get_width() * zoom();
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
	float width = (float)m_drawingArea->get_width() * zoom();
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

