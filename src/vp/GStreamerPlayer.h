#ifndef _GStreamerPlayer_h
#define _GStreamerPlayer_h

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

#include "Player.h"
#include "Document.h"
#include <gst/gst.h>

/*
 *
 */
class GStreamerPlayer : public Gtk::DrawingArea, public Player
{
	/*
	 *
	 */
	static gboolean static_bus_message(GstBus *bus, GstMessage *msg, gpointer data)
	{
		return ((GStreamerPlayer*)data)->on_bus_message(bus, msg);
	}

	/*
	 *
	 */
	static GstBusSyncReply static_element_msg_sync(GstBus *bus, GstMessage *msg, gpointer data)
	{
		return ((GStreamerPlayer*)data)->on_element_msg_sync(bus, msg);
	}

public:

	/*
	 *
	 */
	GStreamerPlayer();

	/*
	 *
	 */
	~GStreamerPlayer();

	/*
	 * Create the pipeline en sets the uri. 
	 */
	bool open(const Glib::ustring &uri);

	/*
	 * Close the pipeline.
	 * The state is sets to null.
	 */
	void close();

	/*
	 * Sets the pipeline state to playing.
	 */
	void play();

	/*
	 * Try to play the segment defined by the subtitle (start to end).
	 * This function supports the looping.
	 * The state is sets to playing.
	 */
	void play_subtitle(const Subtitle &sub);

	/*
	 * Try to play the segment defined (start to end).
	 * This function don't support the mode looping.
	 * The state is sets to playing.
	 */
	void play_segment(const SubtitleTime &start, const SubtitleTime &end);

	/*
	 * Sets the pipeline state to paused.
	 */
	void pause();

	/*
	 * Return true if the state of the pipeline is playing.
	 */
	bool is_playing();

	/*
	 * Return the state of the pipeline.
	 * NONE can be considerate as NULL, the pipeline is not create.
	 */
	Player::State get_state();

	/*
	 * The signal is emited when the state is changed.
	 */
	sigc::signal<void, Player::State>& signal_state_changed();

	/*
	 * Callback emited during the playing.
	 */
	sigc::signal<void>& signal_timeout();

	/*
	 * Return the duration of the stream or 0.
	 */
	long get_duration();

	/*
	 * Return the current position in the stream.
	 */
	long get_position();

	/*
	 * Seeking, the state of the pipeline is not modified.
	 */
	void seek(long position);

	/*
	 * Sets the new playback rate. Used for slow or fast motion.
	 * Default value : 1.0
	 * Min : 0.1
	 * Max : 1.5 
	 */
	void set_playback_rate(double value);

	/*
	 * Return the playback rate.
	 */
	double get_playback_rate();

	/*
	 * Enable/Disable the repeat mode.
	 * Works only with play_subtitle.
	 */
	void set_repeat(bool state);

	/*
	 *
	 */
	void set_volume(double value);

	/*
	 *
	 */
	void set_subtitle_text(const Glib::ustring &text);

	/*
	 *
	 */
	void on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value);

protected:

	/*
	 *
	 */
	bool on_configure_event(GdkEventConfigure *ev);

	/*
	 *
	 */
	bool on_expose_event(GdkEventExpose *ev);

	/*
	 * Connect the XOverlay with the Gdk::Window.
	 * Need a realized widget.
	 */
	void set_x_overlay();

	/*
	 * In case of failure, create a gstreamer error.
	 */
	GstElement* create_element(const Glib::ustring &element, const Glib::ustring &name, const Glib::ustring &msg_error = Glib::ustring());

	/*
	 * Create a GStreamer pipeline.
	 */
	GstElement* create_pipeline();

	/*
	 * Create a GStreamer video output.
	 */
	GstElement* gen_video_element();

	/*
	 * Create a GStreamer audio output.
	 */
	GstElement* gen_audio_element();

	/*
	 * Set the state of the pipeline.
	 * The state change can be asynchronously.
	 */
	bool set_pipeline_state(GstState state);

	/*
	 * Dispatch the gstreamer message.
	 */
	bool on_bus_message(GstBus *bus, GstMessage *msg);

	/*
	 * An error is detected. 
	 * Detroy the pipeline and show the error message in a dialog.
	 */
	void on_bus_message_error(GstBus *bus, GstMessage *msg);

	/*
	 * An warning message is detected.
	 */
	void on_bus_message_warning(GstBus *bus, GstMessage *msg);

	/*
	 * The state of the pipeline has changed.
	 * Update the player state.
	 */
	void on_bus_message_state_changed(GstBus *bus, GstMessage *msg);

	/*
	 * End-of-stream (segment or stream) has been detected, 
	 * update the pipeline state to PAUSED.
	 * Seek to the begining if it's the end of the stream.
	 */
	void on_bus_message_eos(GstBus *bus, GstMessage *msg);

	/*
	 * The pipeline completed playback of a segment.
	 * If the looping is activated send new seek event.
	 * Works only with play_subtitle.
	 */
	void on_bus_message_segment_done(GstBus *bus, GstMessage *msg);

	/*
	 * Wait the message "prepare-xwindow-id" and 
	 * connect the XOverlay to the window.
	 */
	GstBusSyncReply on_element_msg_sync(GstBus *bus, GstMessage *msg);

	/*
	 * Call the timeout signal.
	 */
	bool on_timeout();

	/*
	 * Sets the current state of the pipeline.
	 * Block or unblock the timeout signal and emit the signal state_changed.
	 */
	void set_player_state(Player::State state);

	/*
	 * Sets the state of the pipeline to NULL.
	 */
	void set_pipeline_null();

	/*
	 * Check if are missing plugin, if it's true display a message.
	 * Return true if missing.
	 */
	bool check_missing_plugins();

protected:
	Glib::ustring m_uri;
	GstElement *m_pipeline;
	GstElement *m_textoverlay;
	GstElement *m_video_output;
	
	double m_pipeline_rate;
	GstState m_pipeline_state;
	GstClockTime m_pipeline_duration;

	sigc::signal<void, Player::State> m_signal_state_changed;

	sigc::connection m_timeout_connection;
	sigc::signal<void> m_timeout_signal;

	bool m_loop_seek;
	Subtitle m_subtitle_play;

	std::list<Glib::ustring> m_missing_plugins;
};

#endif//_GStreamerPlayer_h
