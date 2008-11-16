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
#include <gdk/gdkx.h>
#include <gst/interfaces/xoverlay.h>
#include <gst/pbutils/missing-plugins.h>
#include "gstreamerplayer.h"
#include "utility.h"
#include "gstreamer_utility.h"

/*
 * Constructor
 * Init value
 */
GStreamerPlayer::GStreamerPlayer()
{
	m_pipeline = NULL;
	m_textoverlay = NULL;
	m_video_output = NULL;
	m_pipeline_rate = 1.0;
	m_pipeline_state = GST_STATE_NULL;
	m_pipeline_duration = GST_CLOCK_TIME_NONE;

	modify_bg(Gtk::STATE_NORMAL, Gdk::Color("black"));

	add_events(Gdk::EXPOSURE_MASK);

	show();

	m_loop_seek = Config::getInstance().get_value_bool("video-player", "repeat");

	Config::getInstance().signal_changed("video-player").connect(
			sigc::mem_fun(*this, &GStreamerPlayer::on_config_video_player_changed));
}

/*
 *
 */
GStreamerPlayer::~GStreamerPlayer()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(GST_IS_ELEMENT(m_pipeline))
	{
		gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_NULL);
		gst_object_unref(GST_OBJECT(m_pipeline));
		m_pipeline = NULL;
	}
}

/*
 * Create the pipeline en sets the uri. 
 */
bool GStreamerPlayer::open(const Glib::ustring &uri)
{
	// Widget need to be realized
	Config::getInstance().set_value_bool("video-player", "display", true);

	if(!create_pipeline())
		return false;

	m_uri = uri;

	set_pipeline_state(GST_STATE_READY);

	g_object_set(G_OBJECT(m_pipeline), "uri", uri.c_str(), NULL);

	bool ret = set_pipeline_state(GST_STATE_PAUSED);

	// TODO add gst_element_get_state and return the state...

	return ret;
}

/*
 * Close the pipeline.
 * The state is sets to null.
 */
void GStreamerPlayer::close()
{
	set_pipeline_null();
}

/*
 * Set the state of the pipeline.
 * The state change can be asynchronously.
 */
bool GStreamerPlayer::set_pipeline_state(GstState state)
{
	if(GST_IS_ELEMENT(m_pipeline) && m_pipeline_state != state)
	{
		GstStateChangeReturn ret = gst_element_set_state(m_pipeline, state);

		if(ret != GST_STATE_CHANGE_FAILURE)
			return true;
	}

	return false;
}

/*
 * Sets the pipeline state to playing.
 */
void GStreamerPlayer::play()
{
	set_pipeline_state(GST_STATE_PLAYING);
}

/*
 * Try to play the segment defined by the subtitle (start to end).
 * This function supports the looping.
 * The state is sets to playing.
 */
void GStreamerPlayer::play_subtitle(const Subtitle &sub)
{
	if(!GST_IS_ELEMENT(m_pipeline))
		return;

	// subtitle times
	long sub_start = CLAMP(sub.get_start().totalmsecs, 0, get_duration());
	long sub_end = CLAMP(sub.get_end().totalmsecs, 0, get_duration());

	//
	if(sub_start > sub_end)
		std::swap(sub_start, sub_end);
	
	// convert to gstreamer times
	gint64 start = (gint64)sub_start * GST_MSECOND;
	gint64 end = (gint64)sub_end * GST_MSECOND;

	// need to flush directly
	int flags = GST_SEEK_FLAG_FLUSH;

	//if(faster)
	//	flags |= GST_SEEK_FLAG_KEY_UNIT;
	//else
		flags |= GST_SEEK_FLAG_ACCURATE;

	if(m_loop_seek)
	{
		flags |= GST_SEEK_FLAG_SEGMENT;

		m_subtitle_play = sub;
	}

	bool ret = gst_element_seek(
		m_pipeline, 
		m_pipeline_rate, 
		GST_FORMAT_TIME,
		(GstSeekFlags)flags,
		GST_SEEK_TYPE_SET, start,
		GST_SEEK_TYPE_SET, end);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "result of seek %s", (ret) ? "true" : "false");

	gst_element_get_state(m_pipeline, NULL, NULL, 100 * GST_MSECOND);

	//on_timeout();

	set_pipeline_state(GST_STATE_PLAYING);
}

/*
 * Try to play the segment defined (start to end).
 * This function don't support the mode looping.
 * The state is sets to playing.
 */
void GStreamerPlayer::play_segment(const SubtitleTime &tstart, const SubtitleTime &tend)
{
	if(!GST_IS_ELEMENT(m_pipeline))
		return;

	// subtitle times
	long sub_start = CLAMP(tstart.totalmsecs, 0, get_duration());
	long sub_end = CLAMP(tend.totalmsecs, 0, get_duration());

	//
	if(sub_start > sub_end)
		std::swap(sub_start, sub_end);
	
	// convert to gstreamer times
	gint64 start = (gint64)sub_start * GST_MSECOND;
	gint64 end = (gint64)sub_end * GST_MSECOND;

	// need to flush directly
	int flags = GST_SEEK_FLAG_FLUSH;

	//if(faster)
	//	flags |= GST_SEEK_FLAG_KEY_UNIT;
	//else
		flags |= GST_SEEK_FLAG_ACCURATE;

	bool ret = gst_element_seek(
		m_pipeline, 
		m_pipeline_rate, 
		GST_FORMAT_TIME,
		(GstSeekFlags)flags,
		GST_SEEK_TYPE_SET, start,
		GST_SEEK_TYPE_SET, end);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "result of seek %s", (ret) ? "true" : "false");

	gst_element_get_state(m_pipeline, NULL, NULL, 100 * GST_MSECOND);

	set_pipeline_state(GST_STATE_PLAYING);

	on_timeout();
}

/*
 * Sets the pipeline state to paused.
 */
void GStreamerPlayer::pause()
{
	set_pipeline_state(GST_STATE_PAUSED);
}

/*
 * Return true if the state of the pipeline is playing.
 */
bool GStreamerPlayer::is_playing()
{
	return (m_pipeline_state == GST_STATE_PLAYING);
}

/*
 * Return the duration of the stream or 0.
 */
long GStreamerPlayer::get_duration()
{
	if(!GST_CLOCK_TIME_IS_VALID(m_pipeline_duration) && GST_IS_ELEMENT(m_pipeline))
		return 0;
	
	return m_pipeline_duration / GST_MSECOND;
}

/*
 * Return the current position in the stream.
 */
long GStreamerPlayer::get_position()
{
	if(!GST_IS_ELEMENT(m_pipeline))
		return 0;

	gint64 pos;
	GstFormat fmt = GST_FORMAT_TIME;

	if(!gst_element_query_position(GST_ELEMENT(m_pipeline), &fmt, &pos))
		return 0;
	
	return pos / GST_MSECOND;
}

/*
 * Seeking, the state of the pipeline is not modified.
 */
void GStreamerPlayer::seek(long position)
{
	if(!GST_IS_ELEMENT(m_pipeline))
		return;

	position = CLAMP(position, 0, get_duration());

	// convert to gstreamer time
	gint64 pos = position * GST_MSECOND;

	gint64 dur = m_pipeline_duration;

	// need to flush directly
	int flags = GST_SEEK_FLAG_FLUSH;

	//if(faster)
	//	flags |= GST_SEEK_FLAG_KEY_UNIT;
	//else
		flags |= GST_SEEK_FLAG_ACCURATE;

	gst_element_seek(
			m_pipeline, 
			m_pipeline_rate, 
			GST_FORMAT_TIME,
			(GstSeekFlags)flags,
			GST_SEEK_TYPE_SET, pos,
			GST_SEEK_TYPE_SET, dur); // check me

	gst_element_get_state(m_pipeline, NULL, NULL, 100 * GST_MSECOND);

	// FIXME : used new segment ? new time ?... 
	on_timeout();
}

/*
 * Sets the new playback rate. Used for slow or fast motion.
 * Default value : 1.0
 * Min : 0.1
 * Max : 1.5 
 */
void GStreamerPlayer::set_playback_rate(double value)
{
	value = CLAMP(value, 0.1, 1.5);

	m_pipeline_rate = value;

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "New rate: %f", m_pipeline_rate);

	gint64 pos = get_position() * GST_MSECOND;
	gint64 dur = m_pipeline_duration;

	// need to flush directly
	int flags = GST_SEEK_FLAG_FLUSH;

	//if(faster)
	//	flags |= GST_SEEK_FLAG_KEY_UNIT;
	//else
	//	flags |= GST_SEEK_FLAG_ACCURATE;

	gst_element_seek(
			m_pipeline, 
			m_pipeline_rate, 
			GST_FORMAT_TIME,
			(GstSeekFlags)flags,
			GST_SEEK_TYPE_SET, pos,
			GST_SEEK_TYPE_SET, dur); // check me

	gst_element_get_state(m_pipeline, NULL, NULL, 100 * GST_MSECOND);

	on_timeout();
}

/*
 * Return the playback rate.
 */
double GStreamerPlayer::get_playback_rate()
{
	return m_pipeline_rate;
}

/*
 * Return the state of the pipeline.
 * NONE can be considerate as NULL, the pipeline is not create.
 */
Player::State GStreamerPlayer::get_state()
{
	if(m_pipeline_state == GST_STATE_PLAYING)
		return PLAYING;
	else if(m_pipeline_state == GST_STATE_PAUSED)
		return PAUSED;
	else if(m_pipeline_state == GST_STATE_READY)
		return READY;
	return NONE;
}

/*
 * The signal is emited when the state is changed.
 */
sigc::signal<void, Player::State>& GStreamerPlayer::signal_state_changed()
{
	return m_signal_state_changed;
}

/*
 * Call the timeout signal.
 */
bool GStreamerPlayer::on_timeout()
{
	//se_debug(SE_DEBUG_VIDEO_PLAYER);

	m_timeout_signal();
	
	return (m_pipeline_state == GST_STATE_PLAYING);
}

/*
 * Callback emited during the playing.
 */
sigc::signal<void>& GStreamerPlayer::signal_timeout()
{
	return m_timeout_signal;
}

/*
 * Sets the current state of the pipeline.
 * Block or unblock the timeout signal and emit the signal state_changed.
 */
void GStreamerPlayer::set_player_state(Player::State state)
{
	// create the timeout callback
	// the signal is directly blocked
	if(!m_timeout_connection)
	{
		int msec = 100;
		m_timeout_connection = Glib::signal_timeout().connect(
				sigc::mem_fun(*this, &GStreamerPlayer::on_timeout), msec);
		m_timeout_connection.block();
	}

	if(state == PLAYING)
	{
		m_timeout_connection.unblock();
		m_timeout_signal();
	}
	else
	{
		m_timeout_signal(); // update with last position
		m_timeout_connection.block();
	}
	m_signal_state_changed(state);
}

/*
 * Connect the XOverlay with the Gdk::Window.
 * Need a realized widget.
 */
void GStreamerPlayer::set_x_overlay()
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "set_x_overlay...");

	realize_if_needed();

	if(GST_IS_ELEMENT(m_video_output) && GST_IS_X_OVERLAY(m_video_output))
	{
		Glib::RefPtr<Gdk::Window> window = get_window();
		if(window)
		{
			se_debug_message(SE_DEBUG_VIDEO_PLAYER, "try to gst_x_overlay_set_xwindow_id...");
			gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(m_video_output), GDK_WINDOW_XID(window->gobj()));
			
			se_debug_message(SE_DEBUG_VIDEO_PLAYER, "try to gst_x_overlay_expose...");
			gst_x_overlay_expose(GST_X_OVERLAY(m_video_output));

			se_debug_message(SE_DEBUG_VIDEO_PLAYER, "set_x_overlay...ok");
		}
		else
		{
			se_debug_message(SE_DEBUG_VIDEO_PLAYER, "window is NULL");
		}
	}

}

/*
 *
 */
bool GStreamerPlayer::on_configure_event(GdkEventConfigure *ev)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	//set_x_overlay();
	if(m_video_output && GST_IS_X_OVERLAY(m_video_output))
		gst_x_overlay_expose(GST_X_OVERLAY(m_video_output));

	return false;
}

/*
 *
 */
bool GStreamerPlayer::on_expose_event(GdkEventExpose *ev)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(ev && ev->count > 0)
		return true;

	//set_x_overlay();
	if(m_video_output && GST_IS_X_OVERLAY(m_video_output))
	{
		Glib::RefPtr<Gdk::Window> window = get_window();
		if(window)
		{
			//gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(m_video_output), GDK_WINDOW_XID(window->gobj()));
			gst_x_overlay_expose(GST_X_OVERLAY(m_video_output));
		}
	}
	
	return true;
}

/*
 * In case of failure, create a gstreamer error.
 */
GstElement* GStreamerPlayer::create_element(const Glib::ustring &element, const Glib::ustring &name, const Glib::ustring &msg_error)
{
	GstElement *el = gst_element_factory_make(element.c_str(), name.c_str());
	if(el)
		return el;

	// Error. Could not create element
	GST_ELEMENT_ERROR(m_pipeline, RESOURCE, NOT_FOUND, (msg_error.c_str()), (NULL));
	
	return NULL;
}

/*
 * Sets the state of the pipeline to NULL.
 */
void GStreamerPlayer::set_pipeline_null()
{
	if(!GST_IS_ELEMENT(m_pipeline))
		return;

	m_uri = Glib::ustring();

	GstMessage *msg = NULL;
	GstBus *bus = NULL;

	gst_element_set_state(m_pipeline, GST_STATE_NULL);

	bus =  gst_element_get_bus(m_pipeline);
	while((msg = gst_bus_poll(bus, GST_MESSAGE_STATE_CHANGED, 0)))
	{
		gst_bus_async_signal_func(bus, msg, NULL);
		gst_message_unref(msg);
	}
	gst_object_unref(bus);

#define OBJECT_UNREF(obj)	\
	if(GST_IS_OBJECT(obj)) { \
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "unref object '%s'", #obj); \
		gst_element_set_state(GST_ELEMENT(obj), GST_STATE_NULL); \
		g_object_unref(G_OBJECT(obj)); \
		obj = NULL; \
	} else \
		obj = NULL;

	OBJECT_UNREF(m_pipeline);
	OBJECT_UNREF(m_textoverlay);
	OBJECT_UNREF(m_video_output);

#undef OBJECT_UNREF

	m_pipeline_state = GST_STATE_NULL;
	m_pipeline_duration = GST_CLOCK_TIME_NONE;
	//m_pipeline_rate = 1.0; // ?
	
	set_player_state(NONE);
}

/*
 * Create a GStreamer pipeline.
 */
GstElement* GStreamerPlayer::create_pipeline()
{
	// clean or destroy the pipeline
	set_pipeline_null();

	if(gstreamer_utility::check_registry("playbin", 0, 10, 0) == false)
		return NULL;

	m_pipeline = create_element("playbin", "pipeline",
			build_message(_("Failed to create a GStreamer pipeline (%s). "
					"Please check your GStreamer installation."), "playbin"));

	if(m_pipeline == NULL)
		return NULL;

	// create the video bin
	GstElement* video = gen_video_element();
	if(video)
	{
		g_object_set(G_OBJECT(m_pipeline), "video-sink", video, NULL);
	}

	// create the audio bin
	GstElement* audio = gen_audio_element();
	if(audio)
	{
		g_object_set(G_OBJECT(m_pipeline), "audio-sink", audio, NULL);
	}

	// create bus
	GstBus *m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
	
	gst_bus_set_sync_handler(m_bus, static_element_msg_sync, this); 
	gst_bus_add_watch(m_bus, static_bus_message, this);
	
	gst_object_unref(m_bus);
	
	return m_pipeline;
}

/*
 * Create a GStreamer video output.
 */
GstElement* GStreamerPlayer::gen_video_element()
{
	// config value
	Config &cfg = Config::getInstance();

	Glib::ustring cfg_videosink = cfg.get_value_string("video-player", "video-sink");
	Glib::ustring cfg_font_desc = cfg.get_value_string("video-player", "font-desc");
	bool cfg_shaded_background = cfg.get_value_bool("video-player", "shaded-background");
	bool cfg_force_aspect_ratio = cfg.get_value_bool("video-player", "force-aspect-ratio");

	GstElement *bin = NULL;
	GstElement *conv = NULL;
	GstElement *textoverlay = NULL;
	GstElement *sink = NULL;

	// TODO check element
	

	//
	conv = create_element("ffmpegcolorspace", "conv", 
			build_message(_("Failed to create a GStreamer converts video (%s). "
				"Please check your GStreamer installation."), "ffmpegcolorspace"));

	textoverlay = create_element("textoverlay", "overlay",
			build_message(_("Failed to create a GStreamer text overlay (%s). "
				"Please check your GStreamer installation."), "textoverlay"));

	sink = gst_parse_bin_from_description(
			build_message("ffmpegcolorspace name=videocsp ! videoscale name=videoscale ! %s name=videosink", cfg_videosink.c_str()).c_str(),
			true, NULL);

	bin = gst_bin_new("videobin");

	// add and link
	gst_bin_add_many(GST_BIN(bin), conv, textoverlay, sink, NULL);

	gst_element_link_pads(conv, "src", textoverlay, "video_sink");
	gst_element_link_pads(textoverlay, "src", sink, "sink");

	// add sink pad to bin
	GstPad *pad = gst_element_get_pad(conv, "sink");
	gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad));
	gst_object_unref(pad);

	// configure text overlay
	g_object_set(G_OBJECT(textoverlay),
			"halign", "center",
			"valign", "bottom",
			"shaded-background", cfg_shaded_background, 
			NULL);

	if(!cfg_font_desc.empty())
		g_object_set(G_OBJECT(textoverlay), "font-desc", cfg_font_desc.c_str(), NULL);

	GstElement *videosink = gst_bin_get_by_name(GST_BIN(bin), "videosink");
	
	// configure video output
	//if(g_object_has_property(G_OBJECT(m_videosink), "force-aspect-ratio"))
	{
		g_object_set(G_OBJECT(videosink), 
				"force-aspect-ratio", cfg_force_aspect_ratio, 
				NULL);
	}

	m_textoverlay = textoverlay;
	m_video_output = videosink;

	return bin;
}

/*
 * Create a GStreamer audio output.
 */
GstElement* GStreamerPlayer::gen_audio_element()
{
	return NULL;
}


/*
 * Dispatch the gstreamer message.
 */
bool GStreamerPlayer::on_bus_message(GstBus *bus, GstMessage *msg)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, 
			"type='%s' name='%s'", 
			GST_MESSAGE_TYPE_NAME(msg), GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)));

	// check the missing plugin.
	// If is missing add in the list of missing plugins.
	// This list should be show later.
	if(gst_is_missing_plugin_message(msg))
	{
		gchar *description = gst_missing_plugin_message_get_description(msg);
		if(description)
		{
			m_missing_plugins.push_back(description);

			g_free(description);
		}
	}

	GstMessageType type = GST_MESSAGE_TYPE(msg);

	if(type == GST_MESSAGE_ERROR)
	{
		on_bus_message_error(bus, msg);
	}
	else if(type == GST_MESSAGE_WARNING)
	{
		on_bus_message_warning(bus, msg);
	}
	else if(type == GST_MESSAGE_STATE_CHANGED)
	{
		on_bus_message_state_changed(bus, msg);
	}
	else if(type == GST_MESSAGE_SEGMENT_DONE)
	{
		on_bus_message_segment_done(bus, msg);
	}
	else if(type == GST_MESSAGE_EOS)
	{
		on_bus_message_eos(bus, msg);
	}
	else
	{
		/*
		// prepare-xwindow-id
		Glib::ustring type_name = GST_MESSAGE_TYPE_NAME(msg);
		
		if(type_name == "element")
		{
			if(type_name == "prepare-xwindow-id" || type_name == "have-xwindow-id")
			{
				se_debug_message(SE_DEBUG_VIDEO_PLAYER, "set_xwindow_id (%s)", type_name.c_str());

				realize_if_needed();
				//if(!is_realized())
				//	realize();
				
				Glib::RefPtr<Gdk::Window> window = get_window();
				if(window)
				{
					GstXOverlay *xoverlay = GST_X_OVERLAY( GST_MESSAGE_SRC(msg) );
					gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(xoverlay), GDK_WINDOW_XWINDOW(window->gobj()));
				}
				else
				{
					se_debug_message(SE_DEBUG_VIDEO_PLAYER, "window is NULL");
				}
			}
		}
		*/
	}

	// we want to be notified again, so returning true.
	return true;
}

/*
 * Wait the message "prepare-xwindow-id" and 
 * connect the XOverlay to the window.
 */
GstBusSyncReply GStreamerPlayer::on_element_msg_sync(GstBus *bus, GstMessage *msg)
{
	if(GST_MESSAGE_TYPE(msg) != GST_MESSAGE_ELEMENT)
		return GST_BUS_PASS;

	if(!gst_structure_has_name(msg->structure, "prepare-xwindow-id"))
		return GST_BUS_PASS;

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "prepare-xwindow-id...");

	realize_if_needed();
	
	Glib::RefPtr<Gdk::Window> window = get_window();
	if(window)
	{
		GstXOverlay *xoverlay = GST_X_OVERLAY( GST_MESSAGE_SRC(msg) );
	
		gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(xoverlay), GDK_WINDOW_XWINDOW(window->gobj()));

		// Clear hanlder, don't need to call this function in the future.
		GstBus *m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
		
		gst_bus_set_sync_handler(m_bus, NULL, NULL); 
		
		gst_object_unref(m_bus);
	}
	else
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "window is NULL");
	}

	gst_message_unref (msg);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "prepare-xwindow-id...OK");

	return GST_BUS_DROP;	
}

/*
 * An error is detected. 
 * Detroy the pipeline and show the error message in a dialog.
 */
void GStreamerPlayer::on_bus_message_error(GstBus *bus, GstMessage *msg)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	check_missing_plugins();

	GError *error = NULL;
	gchar* debug = NULL;

	gst_message_parse_error(msg, &error, &debug);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "GST_MESSAGE_ERROR : %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));

	dialog_error(build_message(_("Media file could not be played.\n%s"), m_uri.c_str()), GST_STR_NULL(error->message));

	g_error_free(error);
	g_free(debug);
	
	set_pipeline_null();
}

/*
 * An warning message is detected.
 */
void GStreamerPlayer::on_bus_message_warning(GstBus *bus, GstMessage *msg)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	check_missing_plugins();

	GError *error = NULL;
	gchar* debug = NULL;

	gst_message_parse_warning(msg, &error, &debug);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "GST_MESSAGE_WARNING : %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));

	g_warning ("%s [%s]", GST_STR_NULL (error->message), GST_STR_NULL (debug));

	g_error_free(error);
	g_free(debug);
}

/*
 * The state of the pipeline has changed.
 * Update the player state.
 */
void GStreamerPlayer::on_bus_message_state_changed(GstBus *bus, GstMessage *msg)
{
	// only the pipeline message
	if(GST_MESSAGE_SRC(msg) != GST_OBJECT(m_pipeline))
		return;

	GstState old_state, new_state;

	gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);

	if(old_state == new_state)
		return;

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "translation %s -> %s", gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
		
	GstStateChange transition = (GstStateChange)GST_STATE_TRANSITION(old_state, new_state);

	m_pipeline_state = new_state;

	switch(transition)
	{
	case GST_STATE_CHANGE_NULL_TO_READY:
		{
			set_player_state(NONE);
		}break;
	case GST_STATE_CHANGE_READY_TO_PAUSED:
		{
			// get the duration
			{
				gint64 dur;
				GstFormat fmt = GST_FORMAT_TIME;
				if(gst_element_query_duration(GST_ELEMENT(m_pipeline), &fmt, &dur))
				{
					m_pipeline_duration = dur;
				}
			}

			set_player_state(READY);
			set_player_state(PAUSED);

			check_missing_plugins();

		}break;
	case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
		{
			set_player_state(PLAYING);
		}break;
	case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
		{
			set_player_state(PAUSED);

		}break;
	case GST_STATE_CHANGE_PAUSED_TO_READY:
		{
			set_player_state(NONE);
		}break;
	case GST_STATE_CHANGE_READY_TO_NULL:
		{
			set_player_state(NONE);
		}break;
	}
}

/*
 * The pipeline completed playback of a segment.
 * If the looping is activated send new seek event.
 * Works only with play_subtitle.
 */
void GStreamerPlayer::on_bus_message_segment_done(GstBus *bus, GstMessage *msg)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(m_loop_seek && m_subtitle_play)
	{
		{
			GstFormat fmt;
			gint64 position;

			gst_message_parse_segment_done(msg, &fmt, &position);

			se_debug_message(SE_DEBUG_VIDEO_PLAYER, "end of segment at %"GST_TIME_FORMAT, GST_TIME_ARGS(position));
		}

		// subtitle times
		long sub_start = m_subtitle_play.get_start().totalmsecs;
		long sub_end = m_subtitle_play.get_end().totalmsecs;

		// clamp
		sub_start = CLAMP(sub_start, 0, get_duration());
		sub_end = CLAMP(sub_end, 0, get_duration());

		// check the order
		if(sub_start > sub_end)
			std::swap(sub_start, sub_end);
	
		// convert to gstreamer times
		gint64 start = (gint64)sub_start * GST_MSECOND;
		gint64 end = (gint64)sub_end * GST_MSECOND;

		int flags = GST_SEEK_FLAG_SEGMENT | GST_SEEK_FLAG_ACCURATE;

		bool ret = gst_element_seek(
				m_pipeline,
				m_pipeline_rate,
				GST_FORMAT_TIME,
				(GstSeekFlags)flags,
				GST_SEEK_TYPE_SET, start,
				GST_SEEK_TYPE_SET, end);

		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "result of seek %s", (ret) ? "true" : "false");
	}
}

/*
 * End-of-stream (segment or stream) has been detected, 
 * update the pipeline state to PAUSED.
 * Seek to the begining if it's the end of the stream.
 */
void GStreamerPlayer::on_bus_message_eos(GstBus *bus, GstMessage *msg)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	// FIXME with seek_loop
	set_pipeline_state(GST_STATE_PAUSED);

	if(get_position() == get_duration())
	{
		seek(0);
	}
}

/*
 * Check if are missing plugin, if it's true display a message.
 * Return true if missing.
 */
bool GStreamerPlayer::check_missing_plugins()
{
	if(m_missing_plugins.empty())
		return false;

	gstreamer_utility::dialog_missing_plugins(m_missing_plugins);

	m_missing_plugins.clear();

	return true;
}

/*
 * Enable/Disable the repeat mode.
 * Works only with play_subtitle.
 */
void GStreamerPlayer::set_repeat(bool state)
{
	m_loop_seek = state;

	// FIXME flush pipeline ?
}

/*
 *
 */
void GStreamerPlayer::set_volume(double value)
{
	if(GST_IS_ELEMENT(m_pipeline))
		g_object_set(G_OBJECT(m_pipeline), "volume", value, NULL);
}

/*
 *
 */
void GStreamerPlayer::set_subtitle_text(const Glib::ustring &text)
{
	if(GST_IS_ELEMENT(m_textoverlay))
		g_object_set(G_OBJECT(m_textoverlay), "text", text.c_str(), NULL);
}

/*
 *
 */
void GStreamerPlayer::on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value)
{
	if(key == "repeat")
	{
		set_repeat(utility::string_to_bool(value));
	}

	if((GST_IS_OBJECT(m_pipeline) && GST_IS_ELEMENT(m_pipeline)) == false)
		return;

	if(key == "force-aspect-ratio" && G_IS_OBJECT(m_video_output))
	{
		if(g_object_class_find_property(G_OBJECT_GET_CLASS(m_video_output), "force-aspect-ratio") != NULL)
		{
			g_object_set(	
					G_OBJECT(m_video_output),
					"force-aspect-ratio", utility::string_to_bool(value),
					NULL);

			queue_draw();
		}
	}
	else if(key == "shaded-background" && G_IS_OBJECT(m_textoverlay))
	{
		g_object_set(	
				G_OBJECT(m_textoverlay),
				"shaded-background", utility::string_to_bool(value),
				NULL);
	}
	else if(key == "font-desc" && G_IS_OBJECT(m_textoverlay))
	{
		g_object_set(	
				G_OBJECT(m_textoverlay),
				"font-desc", value.c_str(),
				NULL);
	}
}

