#ifndef _gstplayer_h
#define _gstplayer_h

/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2010, kitone
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

#include <gstreamermm.h>
#include <gstreamermm/playbin2.h>
#include <player.h>
#include <gtkmm.h>

class GstPlayer : public Gtk::EventBox, public Player
{
public:

	/*
	 * Constructor
	 * Init values
	 */
	GstPlayer();

	/*
	 * Destructor
	 * Set up pipeline to NULL.
	 */
	virtual ~GstPlayer();

	/*
	 * Create the pipeline and sets the uri.
	 */
	bool open(const Glib::ustring &uri);

	/*
	 * Set up the pipeline to NULL.
	 */
	void close();

	/*
	 * Return the uri of the current video.
	 */
	Glib::ustring get_uri();

	/*
	 * Sets the pipeline state to playing.
	 */
	void play();

	/*
	 * Try to play the segment defined by the subtitle (from start to end).
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
	 * Return the duration of the stream or 0.
	 */
	long get_duration();

	/*
	 * Return the current position in the stream.
	 */
	long get_position();

	/*
	 */
	bool seek(long start, long end, const Gst::SeekFlags &flags);

	/*
	 * Seeking, the state of the pipeline is not modified.
	 */
	void seek(long position);

	/*
	 * Update the text overlay with this new text.
	 */
	void set_subtitle_text(const Glib::ustring &text);

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
	 * Return the framerate of the video.
	 */
	virtual float get_framerate();

protected:

	/*
	 * Create a Gdk::Window child used by the video player.
	 */
	Glib::RefPtr<Gdk::Window> create_video_window();

	/*
	 * Return the Gdk::Window video.
	 */
	Glib::RefPtr<Gdk::Window> get_video_window();

	/*
	 * Show the widget and child (video window).
	 */
	void show();

	/*
	 * Hide the widget and child (video window).
	 */
	void hide();

	/*
	 * Realize the widget and get the the xWindowId.
	 */
	void on_realize();

	/*
	 * The widget size has changed, need to resize the Gdk::Window.
	 */
	bool on_configure_event(GdkEventConfigure *ev);

	/*
	 * The parent changed, we need to re-expose the overlay.
	 */
	void toplevel_win_configure_event(GdkEventConfigure *ev);

	/*
	 * Display an black rectangle or expose the xoverlay.
	 */
	bool on_expose_event(GdkEventExpose *ev);

	/*
	 */
	void on_size_allocate(Gtk::Allocation& rect);

	/*
	 * Refresh the video area.
	 */
	bool on_visibility_notify_event(GdkEventVisibility* ev);

	/*
	 * Create a gstreamer pipeline (Gst::PlayBin2), initialize the
	 * audio and video sink with the configuration. 
	 * Connect the bug message to the player.
	 */
	bool create_pipeline();

	/*
	 * Return a gstreamer audio sink from the configuration option.
	 */
	Glib::RefPtr<Gst::Element> gen_audio_element();

	/*
	 * Return a gstreamer video sink from the configuration option.
	 */
	Glib::RefPtr<Gst::Element> gen_video_element();

	/*
	 * Check if are missing plugin, if it's true display a message.
	 * Return true if missing.
	 */
	bool check_missing_plugins();

	/*
	 * Check if it's a Missing Plugin Message.
	 * Add the description of the missing plugin in the list.
	 */
	bool is_missing_plugin_message(const Glib::RefPtr<Gst::MessageElement> &msg);

	/*
	 * Receive synchronous message emission to set up video. 
	 */
	void on_bus_message_sync(const Glib::RefPtr<Gst::Message> &msg);

	/*
	 * Dispatch the gstreamer message.
	 */
	bool on_bus_message(const Glib::RefPtr<Gst::Bus> &bus, const Glib::RefPtr<Gst::Message> &msg);

	/*
	 * The state of the pipeline has changed.
	 * Update the player state.
	 */
	void on_bus_message_state_changed(const Glib::RefPtr<Gst::MessageStateChanged> &msg);

	/*
	 * Check the missing plugin.
	 * If is missing add in the list of missing plugins.
	 * This list should be show later.
	 */
	void on_bus_message_element(const Glib::RefPtr<Gst::MessageElement> &msg);

	/*
	 * An error is detected. 
	 * Detroy the pipeline and show the error message in a dialog.
	 */
	void on_bus_message_error(const Glib::RefPtr<Gst::MessageError> &msg);

	/*
	 * An error is detected. 
	 */
	void on_bus_message_warning(const Glib::RefPtr<Gst::MessageWarning> &msg);

	/*
	 * End-of-stream (segment or stream) has been detected, 
	 * update the pipeline state to PAUSED.
	 * Seek to the begining if it's the end of the stream.
	 */
	void on_bus_message_eos(const Glib::RefPtr<Gst::MessageEos> &msg);
	
	/*
	 * The pipeline completed playback of a segment.
	 * If the looping is activated send new seek event.
	 * Works only with play_subtitle.
	 */
	void on_bus_message_segment_done(const Glib::RefPtr<Gst::MessageSegmentDone> &msg);
	
	/*
	 * Set the state of the pipeline.
	 * The state change can be asynchronously.
	 */
	bool set_pipeline_state(Gst::State state);

	/*
	 * Sets the state of the pipeline to NULL.
	 */
	void set_pipeline_null();

	/*
	 * The video-player configuration has changed, update the player.
	 */
	void on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value);

	/*
	 * Return the xwindow ID. (Support X11, WIN32 and QUARTZ)
	 * Do not call this function in a gstreamer thread, this cause crash/segfault.
	 * Caused by the merge of the Client-Side Windows in GTK+.
	 */
	gulong get_xwindow_id();

	/*
	 * Set up the XWindowID to the XOverlay.
	 */
	void set_xoverlay_window_id();

	/*
	 */
	void update_pipeline_state_and_timeout();

	/*
	 * Set up the duration value of the stream if need.
	 */
	bool update_pipeline_duration();

	/*
	 * Return the number of audio track.
	 */
	gint get_n_audio();

	/*
	 * Sets the current audio track. (-1 = auto)
	 */
	void set_current_audio(gint track);

	/*
	 * Return the current audio track.
	 */
	gint get_current_audio();

protected:

	Glib::RefPtr<Gdk::Window> m_video_window;
	gulong m_xWindowId;

	guint m_watch_id;
	// Gstreamer Elements
	Glib::RefPtr<Gst::PlayBin2> m_pipeline;
	Glib::RefPtr< Gst::ElementInterfaced<Gst::XOverlay> > m_xoverlay;
	Glib::RefPtr<Gst::TextOverlay> m_textoverlay;

	bool m_pipeline_async_done;
	Gst::State m_pipeline_state;
	gint64 m_pipeline_duration;
	double m_pipeline_rate;
	bool m_loop_seek;
	Subtitle m_subtitle_play;

	std::list<Glib::ustring> m_missing_plugins;
};

#endif//_gstplayer_h
