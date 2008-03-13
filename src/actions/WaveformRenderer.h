#ifndef _WaveformRenderer_h
#define _WaveformRenderer_h

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
#include "Waveform.h"
#include <gst/gst.h>

/*
 *
 */
class WaveformRenderer
{
public:
	
	/*
	 *
	 */
	WaveformRenderer(Gtk::DrawingArea *area);

	/*
	 *
	 */
	virtual ~WaveformRenderer();

	/*
	 *
	 */
	virtual bool on_expose_event(GdkEventExpose *ev);

	/*
	 *
	 */
	virtual void redraw_all();

	/*
	 *
	 */
	void set_start_area(int value);

	/*
	 *
	 */
	int get_start_area();

	/*
	 *
	 */
	int get_end_area();

	/*
	 * return the time of the position in the area
	 * time is in msec (SubtitleTime.totalmsecs)
	 */
	long get_time_by_pos(int pos);

	/*
	 * return the position of the time in the area
	 */
	int get_pos_by_time(long msec);

	/*
	 *	return the position in the area with scrolling support
	 */
	int get_mouse_coords(int x);

	/*
	 *
	 */
	long get_mouse_time(int x);

	/*
	 *
	 */
	sigc::signal<Document*>& signal_document();

	/*
	 *
	 */
	sigc::signal<int>& signal_zoom();

	/*
	 *
	 */
	sigc::signal<float>& signal_scale();

	/*
	 *
	 */
	void set_waveform(const Glib::RefPtr<Waveform> &wf);

	//protected:
	Gtk::DrawingArea* m_drawingArea;
	int m_start_area;
	// when is true display the time of the mouse
	bool m_display_time_info;
	
	Glib::RefPtr<Waveform> m_waveform;

	sigc::signal<Document*> document;
	sigc::signal<int> zoom;
	sigc::signal<float> scale;
	sigc::signal<long> player_time; // the current time of the player
};


#endif//_WaveformRenderer_h

