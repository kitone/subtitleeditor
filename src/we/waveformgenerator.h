#ifndef _WaveformGenerator_h
#define _WaveformGenerator_h

/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2009, kitone
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
#include "waveform.h"

/*
 *
 */
class WaveformGenerator : public Gtk::Dialog
{
private:
	
	/*
	 * Warp gstreamer to c++
	 */
	static gboolean static_bus_message(GstBus *bus, GstMessage *msg, gpointer data);

	/*
	 * Warp gstreamer to c++
	 */
	static void static_newpad(GstElement *decodebin, GstPad *pad, gboolean last, gpointer data);

public:
	
	/*
	 * Open the dialog and try to generate the waveform.
	 * Return the new waveform.
	 */
	static Glib::RefPtr<Waveform> create(const Glib::ustring &uri);

protected:

	/*
	 *
	 */
	WaveformGenerator(const Glib::ustring &uri, Glib::RefPtr<Waveform> &wf);
	
	/*
	 * Destroy the pipeline
	 */
	~WaveformGenerator();

	/*
	 * Create the GStreamer pipeline.
	 *
	 * filesrc uri ! decodebin ! audioconvert ! level ! fakesink
	 */
	void create_pipeline(const Glib::ustring &uri);

	/*
	 * Destroy the GStreamer pipeline.
	 */
	void destroy_pipeline();

	/*
	 * Create a GStreamer audio container.
	 *
	 * audioconvert ! level ! fakesink.
	 */
	GstElement* create_audio_bin();

	/*
	 * Check if are missing plugins, if it's true display a message.
	 * Return true if missing.
	 */
	bool check_missing_plugins();

	/*
	 * Dispatch a GStreamer message.
	 */
	bool on_bus_message(GstBus *bus, GstMessage *msg);

	/*
	 * A GStreamer internal error. 
	 * Display a dialog message.
	 */
	bool on_bus_message_error(GstBus *bus, GstMessage *msg);

	/*
	 * A GStreamer warning. Log the message.
	 */
	bool on_bus_message_warning(GstBus *bus, GstMessage *msg);

	/*
	 * The pipeline state has changed.
	 * Update with only the pipeline message.
	 */
	bool on_bus_message_state_changed(GstBus *bus, GstMessage *msg);

	/*
	 * End of the stream. Send RESPONSE_OK.
	 */
	bool on_bus_message_eos(GstBus *bus, GstMessage *msg);

	/*
	 *
	 */
	bool on_bus_message_tag(GstBus *bus, GstMessage *msg);

	/*
	 * Generate the waveform with the level message.
	 */
	bool on_bus_message_element_level(GstBus *bus, GstMessage *msg);

	/*
	 * Only used the audio pad. Connect the the AudioBin.
	 */
	void on_newpad(GstElement *decodebin, GstPad *pad, bool last);

	/*
	 * Return the current position in the pipeline.
	 */
	gint64 get_position();

	/*
	 * Return the duration of the pipeline.
	 */
	gint64 get_duration();

	/*
	 * Update the progress bar with the position in the pipeline.
	 */
	bool update_progress_bar();

protected:

	Gtk::ProgressBar* m_progressbar;
	sigc::connection m_connection_timeout;

	GstElement* m_pipeline;
	GstClockTime m_position;
	GstClockTime m_duration;

	std::list<Glib::ustring> m_missing_plugins;

	Glib::RefPtr<Waveform> m_waveform;
};				

#endif//_WaveformGenerator_h

