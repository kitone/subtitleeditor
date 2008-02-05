#ifndef _WaveformGeneratorUI_h
#define _WaveformGeneratorUI_h

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
#include <gst/gst.h>
#include "Waveform.h"

class WaveformGeneratorUI : public Gtk::Dialog
{
public:
	WaveformGeneratorUI(Glib::RefPtr<Waveform> &waveform, const Glib::ustring &uri);
	~WaveformGeneratorUI();

protected:
	
	/*
	 *
	 */
	bool bus_message(GstBus *bus, GstMessage *message);
	
	void newpad(GstElement *decodebin, GstPad *pad, gboolean last);
	/*
	 *
	 */
	static gboolean __static_bus_message(GstBus *bus, GstMessage *message, gpointer data);
	static void __static_newpad(GstElement* decodebin, GstPad *pad, gboolean last, gpointer data);

	/*
	 *
	 */
	bool on_timeout();
protected:
	
	Gtk::ProgressBar	m_progressbar;
	GstElement* m_pipeline;
	GstBus* m_bus;
	Glib::RefPtr<Waveform> m_waveform;
	gint64	m_duration;
	GstClockTime	endtime;

	GstElement *m_audio;
	GstElement *m_src;
	GstElement *m_dec;
	GstElement *m_conv;
	GstElement *m_lev;
	GstElement *m_sink;
};

#endif//_WaveformGeneratorUI_h

