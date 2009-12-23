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

#include "gstplayer.h"
#include <debug.h>
#include <i18n.h>
#include <utility.h>
#include <gstreamer_utility.h>
#include <gst/pbutils/missing-plugins.h>

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#elif defined(GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#elif defined(GDK_WINDOWING_QUARTZ)
#include <gdk/gdkquartz.h>
#endif


/*
 * Constructor
 * Init values
 */
GstPlayer::GstPlayer()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	m_xWindowId = 0;
	m_watch_id = 0;
	m_pipeline_state = Gst::STATE_NULL;
	m_pipeline_duration = GST_CLOCK_TIME_NONE;
	m_pipeline_rate = 1.0;
	m_loop_seek = Config::getInstance().get_value_bool("video-player", "repeat");

	set_events(Gdk::ALL_EVENTS_MASK);
	/*
	set_events(
			get_events() | 
			Gdk::EXPOSURE_MASK |
			Gdk::STRUCTURE_MASK |
			Gdk::VISIBILITY_NOTIFY_MASK |
			Gdk::POINTER_MOTION_MASK | 
			Gdk::KEY_PRESS_MASK);
	*/
	set_flags(Gtk::CAN_FOCUS);
	unset_flags(Gtk::DOUBLE_BUFFERED);

	show();

	Config::getInstance().signal_changed("video-player").connect(
			sigc::mem_fun(*this, &GstPlayer::on_config_video_player_changed));
}

/*
 * Destructor
 * Set up pipeline to NULL.
 */
GstPlayer::~GstPlayer()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(m_pipeline)
	{
		set_pipeline_state(Gst::STATE_NULL);
		m_pipeline.clear();
	}
}

/*
 * Create the pipeline and sets the uri.
 */
bool GstPlayer::open(const Glib::ustring &uri)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "try to open uri '%s'", uri.c_str());
	// we need to make sure the widget is realized
	realize_if_needed();

	if(!create_pipeline())
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "could not open uri");
		return false;
	}
	// setup the uri property and init the player state to paused
	m_pipeline->property_uri() = uri;

	bool ret = set_pipeline_state(Gst::STATE_PAUSED);

	return ret;
}

/*
 * Set up the pipeline to NULL.
 */
void GstPlayer::close()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	set_pipeline_null();
}

/*
 * Return the uri of the current video.
 */
Glib::ustring GstPlayer::get_uri()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(!m_pipeline)
		return Glib::ustring();
	return m_pipeline->property_uri();
}

/*
 * Sets the pipeline state to playing.
 */
void GstPlayer::play()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	set_pipeline_state(Gst::STATE_PLAYING);
}

/*
 * Try to play the segment defined by the subtitle (from start to end).
 * This function supports the looping.
 * The state is sets to playing.
 */
void GstPlayer::play_subtitle(const Subtitle &sub)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(!m_pipeline)
		return;
	Gst::SeekFlags flags = Gst::SEEK_FLAG_FLUSH | Gst::SEEK_FLAG_ACCURATE;
	if(m_loop_seek)
	{
		flags |= Gst::SEEK_FLAG_SEGMENT;
		m_subtitle_play = sub;
	}
	// if the seek success, we update the timeout and 
	// swap the pipeline state to playing
	if(seek(sub.get_start().totalmsecs, sub.get_end().totalmsecs, flags))
	{
		update_pipeline_state_and_timeout();
		set_pipeline_state(Gst::STATE_PLAYING);
	}
}

/*
 * Try to play the segment defined (start to end).
 * This function don't support the mode looping.
 * The state is sets to playing.
 */
void GstPlayer::play_segment(const SubtitleTime &start, const SubtitleTime &end)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(!m_pipeline)
		return;
	
	Gst::SeekFlags flags = Gst::SEEK_FLAG_FLUSH | Gst::SEEK_FLAG_ACCURATE;

	if(seek(start.totalmsecs, end.totalmsecs, flags))
		update_pipeline_state_and_timeout();
}

/*
 * Sets the pipeline state to paused.
 */
void GstPlayer::pause()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	set_pipeline_state(Gst::STATE_PAUSED);
}

/*
 * Return true if the state of the pipeline is playing.
 */
bool GstPlayer::is_playing()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	return (m_pipeline_state == Gst::STATE_PLAYING);
}

/*
 * Return the duration of the stream or 0.
 */
long GstPlayer::get_duration()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(!GST_CLOCK_TIME_IS_VALID(m_pipeline_duration) && m_pipeline)
		return 0;
	return m_pipeline_duration / GST_MSECOND;
}

/*
 * Return the current position in the stream.
 */
long GstPlayer::get_position()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(!m_pipeline)
		return 0;

	gint64 pos = 0;
	Gst::Format fmt = Gst::FORMAT_TIME;

	if(!m_pipeline->query_position(fmt, pos))
		return 0;
	return pos / GST_MSECOND;
}

/*
 */
bool GstPlayer::seek(long start, long end, const Gst::SeekFlags &flags)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, 
			"try to seek %s (%d) - %s (%d)",
			SubtitleTime(start).str().c_str(), start,
			SubtitleTime(end).str().c_str(), end);

	if(!m_pipeline)
		return false;
	// clamp
	start = CLAMP(start, 0, get_duration());
	end = CLAMP(end, 0, get_duration());
	// check the order
	if(start > end)
		std::swap(start, end);
	// convert to gstreamer time
	gint64 gstart = start * GST_MSECOND;
	gint64 gend = end * GST_MSECOND;

	bool ret = m_pipeline->seek(
			m_pipeline_rate,
			Gst::FORMAT_TIME,
			flags,
			Gst::SEEK_TYPE_SET, gstart,
			Gst::SEEK_TYPE_SET, gend);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "result of seek %s", (ret) ? "true" : "false");

	return ret;
}

/*
 * Seeking, the state of the pipeline is not modified.
 */
void GstPlayer::seek(long position)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(!m_pipeline)
		return;

	if(seek(position, get_duration(), Gst::SEEK_FLAG_FLUSH | Gst::SEEK_FLAG_ACCURATE))
		update_pipeline_state_and_timeout();
}

/*
 * Update the text overlay with this new text.
 */
void GstPlayer::set_subtitle_text(const Glib::ustring &text)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "text='%s'", text.c_str());

	if(m_textoverlay)
		m_textoverlay->property_text() = text;
}

/*
 * Sets the new playback rate. Used for slow or fast motion.
 * Default value : 1.0
 * Min : 0.1
 * Max : 1.5 
 */
void GstPlayer::set_playback_rate(double value)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "rate=%f", value);

	value = CLAMP(value, 0.1, 1.5); // FIXME

	m_pipeline_rate = value;

	if(seek(get_position(), get_duration(), Gst::SEEK_FLAG_FLUSH))
		update_pipeline_state_and_timeout();
}

/*
 * Return the playback rate.
 */
double GstPlayer::get_playback_rate()
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "pipeline_rate=%f", m_pipeline_rate);

	return m_pipeline_rate;
}

/*
 * Enable/Disable the repeat mode.
 * Works only with play_subtitle.
 */
void GstPlayer::set_repeat(bool state)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "state=%s", (state) ? "true" : "false");

	m_loop_seek = state;
	// FIXME flush pipeline ?
}

/*
 * Realize the widget and get the the xWindowId.
 */
void GstPlayer::on_realize()
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "try to realize...");

	Gtk::EventBox::on_realize();

	create_video_window();

	set_flags(Gtk::REALIZED);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "get xWindowId...");

	m_xWindowId = get_xwindow_id();

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "try to realize... ok");
}

/*
 * Create a Gdk::Window child used by the video player.
 */
Glib::RefPtr<Gdk::Window> GstPlayer::create_video_window()
{
	if(m_video_window)
		return m_video_window;

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "setup event_mask");
	
	realize_if_needed();

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "try to create video window");

	Glib::RefPtr<Gdk::Window> m_window = get_window();
	if(!m_window)
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "get_window return invalid Glib::RefPtr");
		g_warning("GstPlayer can get window, get_window return invalid Glib::RefPtr");

		return Glib::RefPtr<Gdk::Window>(NULL);
	}

	GdkWindowAttr attributes;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = 0;
	attributes.y = 0;
	attributes.width = get_width();
	attributes.height = get_height();
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = get_visual()->gobj();
	attributes.colormap = get_colormap()->gobj();
	attributes.event_mask = get_events() | GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK;

	gint attributes_mask = GDK_WA_X | GDK_WA_Y;

	m_video_window = Gdk::Window::create(m_window, &attributes, attributes_mask);
	if(!m_video_window)
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "Failed to create Gdk::Window");
		g_warning("GstPlayer could not create m_video_window");
		return Glib::RefPtr<Gdk::Window>(NULL);
	}

	get_style()->attach(m_video_window);
	get_style()->set_background(m_video_window, Gtk::STATE_NORMAL);

	// Set up background to black
	Gdk::Color color("black");
	get_colormap()->alloc_color(color, true, true);
	m_video_window->set_background(color);

	set_flags(Gtk::REALIZED);

	// Connect to configure event on the top level window
	get_toplevel()->signal_configure_event().connect_notify(
			sigc::mem_fun(*this, &GstPlayer::toplevel_win_configure_event));

	show_all_children();
	show();
	return m_video_window;
}

/*
 * Return the Gdk::Window video.
 */
Glib::RefPtr<Gdk::Window> GstPlayer::get_video_window()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	Glib::RefPtr<Gdk::Window> win = create_video_window();
	if(win)
		return win;

	g_warning("get_video_window return an NULL RefPtr");

	return Glib::RefPtr<Gdk::Window>();
}

/*
 * Show the widget and child (video window).
 */
void GstPlayer::show()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(Glib::RefPtr<Gdk::Window> window = get_window())
		window->show();
	if(m_video_window)
		m_video_window->show();

	Gtk::EventBox::show();
}

/*
 * Hide the widget and child (video window).
 */
void GstPlayer::hide()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(Glib::RefPtr<Gdk::Window> window = get_window())
		window->hide();
	if(m_video_window)
		m_video_window->hide();

	Gtk::EventBox::hide();
}

/*
 * The widget size has changed, need to resize the Gdk::Window.
 */
bool GstPlayer::on_configure_event(GdkEventConfigure *ev)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	Gtk::EventBox::on_configure_event(ev);

	if(Glib::RefPtr<Gdk::Window> window = get_window())
		window->resize(ev->width, ev->height);
	if(m_video_window)
		m_video_window->resize(ev->width, ev->height);

	if(m_xoverlay)
		m_xoverlay->expose();

	return false;
}

/*
 * The parent changed, we need to re-expose the overlay.
 */
void GstPlayer::toplevel_win_configure_event(GdkEventConfigure *ev)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(m_xoverlay)
		m_xoverlay->expose();
}

/*
 * Display an black rectangle or expose the xoverlay.
 */
bool GstPlayer::on_expose_event(GdkEventExpose *ev)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(ev && ev->count > 0)
		return true;
	if(!m_video_window)
		return true;
	
	if(!m_xoverlay)
		m_video_window->draw_rectangle(get_style()->get_black_gc(), true, 0, 0, get_width(), get_height());
	else
	{
		set_xoverlay_window_id();
		m_xoverlay->expose();
	}
	return true;
}

/*
 */
void GstPlayer::on_size_allocate(Gtk::Allocation& rect)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, 
			"move_resize videowindow (%d, %d, %d, %d)", 
			rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height());

	Gtk::EventBox::on_size_allocate(rect);

	if(!is_realized())
		return;

	Glib::RefPtr<Gdk::Window> videowindow = get_video_window();
	if(!videowindow)
		return;

	videowindow->move_resize(0,0, rect.get_width(), rect.get_height());
}

/*
 * Refresh the video area.
 */
bool GstPlayer::on_visibility_notify_event(GdkEventVisibility* ev)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	Gtk::EventBox::on_visibility_notify_event(ev);

	Glib::RefPtr<Gdk::Window> videowindow = get_video_window();
	if(videowindow && !m_xoverlay)
		videowindow->clear();
	queue_draw();
	return false;
}

/*
 * Create a gstreamer pipeline (Gst::PlayBin2), initialize the
 * audio and video sink with the configuration. 
 * Connect the bug message to the player.
 */
bool GstPlayer::create_pipeline()
{
	// Clean or destroy the old pipeline
	set_pipeline_null();

	m_pipeline = Gst::PlayBin2::create("pipeline");

	m_pipeline->property_audio_sink() = gen_audio_element();
	m_pipeline->property_video_sink() = gen_video_element();

	Glib::RefPtr<Gst::Bus> bus = m_pipeline->get_bus();

	// Enable synchronous msg emission to set up video
	bus->enable_sync_message_emission();

	// Connect synchronous msg to set up xoverlay with the widget
	bus->signal_sync_message().connect(
			sigc::mem_fun(*this, &GstPlayer::on_bus_message_sync));

	m_watch_id = bus->add_watch(
			sigc::mem_fun(*this, &GstPlayer::on_bus_message));
	return true;
}

/*
 * Return a gstreamer audio sink from the configuration option.
 */
Glib::RefPtr<Gst::Element> GstPlayer::gen_audio_element()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	Config &cfg = Config::getInstance();

	Glib::ustring cfg_audiosink = cfg.get_value_string("video-player", "audio-sink");

	try
	{
		Glib::RefPtr<Gst::Element> sink = Gst::ElementFactory::create_element(cfg_audiosink, "audiosink");
		if(!sink)
		{
			throw std::runtime_error(
					build_message(
						_("Failed to create a GStreamer audio output (%s). "
							"Please check your GStreamer installation."), cfg_audiosink.c_str()));
		}
		return sink;
	}
	catch(std::runtime_error &ex)
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "failed to gen_audio_element '%s'", ex.what());
		GST_ELEMENT_WARNING(m_pipeline->gobj(), RESOURCE, NOT_FOUND, (ex.what()), (NULL));
	}
	// Return an NULL ptr
	return Glib::RefPtr<Gst::Element>();
}

/*
 * Return a gstreamer video sink from the configuration option.
 */
Glib::RefPtr<Gst::Element> GstPlayer::gen_video_element()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	Config &cfg = Config::getInstance();

	Glib::ustring cfg_videosink = cfg.get_value_string("video-player", "video-sink");
	Glib::ustring cfg_font_desc = cfg.get_value_string("video-player", "font-desc");
	bool cfg_shaded_background = cfg.get_value_bool("video-player", "shaded-background");
	bool cfg_force_aspect_ratio = cfg.get_value_bool("video-player", "force-aspect-ratio");

	try
	{
		Glib::RefPtr<Gst::Element> conv, textoverlay, sink;

		// ffmpegcolorspace
		conv = Gst::ElementFactory::create_element("ffmpegcolorspace", "conv");
		if(!conv)
		{
			throw std::runtime_error(
					build_message(
						_("Failed to create a GStreamer converts video (%s). "
							"Please check your GStreamer installation."), "ffmpegcolorspace"));
		}
		// textoverlay
		textoverlay = Gst::ElementFactory::create_element("textoverlay", "overlay");
		if(!textoverlay)
		{
			throw std::runtime_error(
					build_message(
						_("Failed to create a GStreamer textoverlay (%s). "
							"Please check your GStreamer installation."), "textoverlay"));
		}
		// ffmpegcolorspace ! videoscale ! %s videosink
		sink = Gst::Parse::create_bin(
			Glib::ustring::compose(
				"ffmpegcolorspace name=videocsp ! "
				"videoscale name=videoscale ! "
				"%1 name=videosink", cfg_videosink), true);
		if(!sink)
		{
			throw std::runtime_error(
					build_message(
						_("Failed to create a GStreamer sink (%s). "
							"Please check your GStreamer installation."), cfg_videosink.c_str()));
		}

		Glib::RefPtr<Gst::Bin> bin = Gst::Bin::create("videobin");

		// Add in the videobin and link
		bin->add(conv)->add(textoverlay)->add(sink);

		conv->link_pads("src", textoverlay, "video_sink");
		textoverlay->link_pads("src", sink, "sink");

		// Add sink pad to bin element
		Glib::RefPtr<Gst::Pad> pad = conv->get_static_pad("sink");
		bin->add_pad(	Gst::GhostPad::create("sink", pad));

		// configure text overlay
		m_textoverlay = Glib::RefPtr<Gst::TextOverlay>::cast_dynamic(textoverlay);
		if(m_textoverlay)
		{
			m_textoverlay->property_halign() = "center";
			m_textoverlay->property_valign() = "bottom";
			m_textoverlay->property_shaded_background() = cfg_shaded_background;
			m_textoverlay->property_font_desc() = cfg_font_desc;
		}
		else
			g_warning("could not get the textoverlay");

		// Configure video output
		Glib::RefPtr<Gst::Element> videosink = bin->get_element("videosink");
		if(videosink)
		{
			g_object_set(G_OBJECT(videosink->gobj()),
					"force-aspect-ratio", cfg_force_aspect_ratio, NULL);
		}
		return bin;
	}
	catch(std::runtime_error &ex)
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "failed to gen_video_element '%s'", ex.what());
		GST_ELEMENT_ERROR(m_pipeline->gobj(), RESOURCE, NOT_FOUND, (ex.what()), (NULL));
	}
	// Return an NULL ptr
	return Glib::RefPtr<Gst::Element>();
}

/*
 * Set the state of the pipeline.
 * The state change can be asynchronously.
 */
bool GstPlayer::set_pipeline_state(Gst::State state)
{
	if(m_pipeline && m_pipeline_state != state)
	{
		Gst::StateChangeReturn ret = m_pipeline->set_state(state);
		if(ret != Gst::STATE_CHANGE_FAILURE)
			return true;
	}
	return false;
}

/*
 * Sets the state of the pipeline to NULL.
 */
void GstPlayer::set_pipeline_null()
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "set up pipeline to null...");

	if(!m_pipeline)
		return;

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "set up pipeline to NULL");

	set_pipeline_state(Gst::STATE_NULL);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "remove watch id");

	m_pipeline->get_bus()->remove_watch(m_watch_id);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "set up all values to NULL");

	m_watch_id = 0;
	m_pipeline_state = Gst::STATE_NULL;
	m_pipeline_duration = GST_CLOCK_TIME_NONE;

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "clear RefPtr");

	m_pipeline.clear();
	m_textoverlay.clear();
	m_xoverlay.clear();
	m_pipeline_rate = 1.0;

	set_player_state(NONE);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "set up pipeline to null... ok");
}

/*
 * Check if are missing plugin, if it's true display a message.
 * Return true if missing.
 */
bool GstPlayer::check_missing_plugins()
{
	if(m_missing_plugins.empty())
		return false;

	gstreamer_utility::dialog_missing_plugins(m_missing_plugins);
	m_missing_plugins.clear();
	return true;
}

/*
 * Receive synchronous message emission to set up video. 
 */
void GstPlayer::on_bus_message_sync( const Glib::RefPtr<Gst::Message> &msg)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	// Ignore anything but 'prepare-xwindow-id' element messages
	if(msg->get_message_type() != Gst::MESSAGE_ELEMENT)
		return;
	if(!msg->get_structure().has_name("prepare-xwindow-id"))
		return;

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "prepare-xwindow-id");

	// Get the gstreamer element source
	Glib::RefPtr<Gst::Element> el_src = 
		Glib::RefPtr<Gst::Element>::cast_dynamic(msg->get_source());

	// Has an XOverlay
	Glib::RefPtr< Gst::ElementInterfaced<Gst::XOverlay> > xoverlay =
		Gst::Interface::cast<Gst::XOverlay>(el_src);

	if(xoverlay)
	{
		m_xoverlay = xoverlay;
		set_xoverlay_window_id();
	}
	else
	{
		g_warning("Failed to get xoverlay");
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "failed to get xoverlay");
	}
	// We don't need to keep sync message
	Glib::RefPtr<Gst::Bus> bus = m_pipeline->get_bus();
	bus->disable_sync_message_emission();
}

/*
 * Dispatch the gstreamer message.
 */
bool GstPlayer::on_bus_message(const Glib::RefPtr<Gst::Bus> &bus, const Glib::RefPtr<Gst::Message> &msg)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, 
			"type='%s' name='%s'", 
			GST_MESSAGE_TYPE_NAME(msg->gobj()), GST_OBJECT_NAME(GST_MESSAGE_SRC(msg->gobj())));

	// check the missing plugin.
	// If is missing add in the list of missing plugins.
	// This list should be show later.
	if(gst_is_missing_plugin_message(msg->gobj()))
	{
		gchar *description = gst_missing_plugin_message_get_description(msg->gobj());
		if(description)
		{
			m_missing_plugins.push_back(description);
			g_free(description);
		}
	}

	switch(msg->get_message_type())
	{
	case Gst::MESSAGE_EOS: 
			on_bus_message_eos( Glib::RefPtr<Gst::MessageEos>::cast_dynamic(msg) );
			break;
	case Gst::MESSAGE_ERROR:
			on_bus_message_error( Glib::RefPtr<Gst::MessageError>::cast_dynamic(msg) );
			break;
	case Gst::MESSAGE_WARNING:
			on_bus_message_warning( Glib::RefPtr<Gst::MessageWarning>::cast_dynamic(msg) );
			break;
	case Gst::MESSAGE_STATE_CHANGED:
			on_bus_message_state_changed( Glib::RefPtr<Gst::MessageStateChanged>::cast_dynamic(msg) );
			break;
	case Gst::MESSAGE_SEGMENT_DONE:
			on_bus_message_segment_done( Glib::RefPtr<Gst::MessageSegmentDone>::cast_dynamic(msg) );
			break;
	default:
			break;
	}
	return true;
}

/*
 * An error is detected. 
 * Detroy the pipeline and show the error message in a dialog.
 */
void GstPlayer::on_bus_message_error(const Glib::RefPtr<Gst::MessageError> &msg)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	check_missing_plugins();

	Glib::Error err;
	std::string err_dbg;
	msg->parse(err, err_dbg);
	
	se_debug_message(SE_DEBUG_VIDEO_PLAYER,	"GST_MESSAGE_ERROR : %s [%s]", 
			err.what().c_str(), err_dbg.c_str());

	dialog_error(build_message(_("Media file could not be played.\n%s"), get_uri().c_str()), err.what().c_str());

	set_pipeline_null();
}

/*
 * An warning message is detected.
 */
void GstPlayer::on_bus_message_warning(const Glib::RefPtr<Gst::MessageWarning> &msg)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	check_missing_plugins();

	Glib::Error err;
	std::string err_dbg;
	msg->parse(err, err_dbg);
	
	se_debug_message(SE_DEBUG_VIDEO_PLAYER,	"GST_MESSAGE_WARNING : %s [%s]", 
			err.what().c_str(), err_dbg.c_str());

	g_warning("%s [%s]", err.what().c_str(), err_dbg.c_str());
}

/*
 * The state of the pipeline has changed.
 * Update the player state.
 */
void GstPlayer::on_bus_message_state_changed(const Glib::RefPtr<Gst::MessageStateChanged> &msg)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	// We only update when it's the pipeline object
	if(msg->get_source()->get_name() != "pipeline")
		return;

	Gst::State old_state, new_state, pending;

	msg->parse(old_state, new_state, pending);

	// Update the current state of the pipeline
	m_pipeline_state = new_state;

	if(old_state == Gst::STATE_NULL && new_state == Gst::STATE_READY)
	{
		set_player_state(NONE);
	}
	else if(old_state == Gst::STATE_READY && new_state == Gst::STATE_PAUSED)
	{
		// get the duration of the pipeline
		gint64 dur;
		Gst::Format fmt = Gst::FORMAT_TIME;
		if(m_pipeline->query_duration(fmt, dur))
			m_pipeline_duration = dur;

		set_player_state(READY);
		set_player_state(PAUSED);
		
		check_missing_plugins();
	}
	else if(old_state == Gst::STATE_PAUSED && new_state == Gst::STATE_PLAYING)
	{
		set_player_state(PLAYING);
	}
	else if(old_state == Gst::STATE_PLAYING && new_state == Gst::STATE_PAUSED)
	{
		set_player_state(PAUSED);
	}
	else if(old_state == Gst::STATE_PAUSED && new_state == Gst::STATE_READY)
	{
		set_player_state(NONE);
	}
	else if(old_state == Gst::STATE_READY && new_state == Gst::STATE_NULL)
	{
		set_player_state(NONE);
	}
}

/*
 * End-of-stream (segment or stream) has been detected, 
 * update the pipeline state to PAUSED.
 * Seek to the begining if it's the end of the stream.
 */
void GstPlayer::on_bus_message_eos(const Glib::RefPtr<Gst::MessageEos> &msg)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	// FIXME with seek_loop
	set_pipeline_state(Gst::STATE_PAUSED);

	if(get_position() == get_duration())
		seek(0);
}

/*
 * The pipeline completed playback of a segment.
 * If the looping is activated send new seek event.
 * Works only with play_subtitle.
 */
void GstPlayer::on_bus_message_segment_done(const Glib::RefPtr<Gst::MessageSegmentDone> &msg)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(!(m_loop_seek && m_subtitle_play))
		return;

	// TODO debug information of MessageSegmentDone

	seek(
		m_subtitle_play.get_start().totalmsecs, 
		m_subtitle_play.get_end().totalmsecs, 
		Gst::SEEK_FLAG_ACCURATE | Gst::SEEK_FLAG_SEGMENT);
}

/*
 * The video-player configuration has changed, update the player.
 */
void GstPlayer::on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "%s %s", key.c_str(), value.c_str());

	if(key == "repeat")
		set_repeat(utility::string_to_bool(value));
	else if(m_pipeline)
	{
		if(key == "force-aspect-ratio" && m_xoverlay)
		{
			g_object_set(
					G_OBJECT(m_xoverlay->gobj()), "force-aspect-ratio", utility::string_to_bool(value),
					NULL);
			queue_draw();
		}
		else if(key == "shaded-background" && m_textoverlay)
		{
			m_textoverlay->property_shaded_background() = utility::string_to_bool(value);
		}
		else if(key == "font-desc" && m_textoverlay)
		{
			m_textoverlay->property_font_desc() = value;
		}
	}
}

/*
 * Return the xwindow ID. (Support X11, WIN32 and QUARTZ)
 * Do not call this function in a gstreamer thread, this cause crash/segfault.
 * Caused by the merge of the Client-Side Windows in GTK+.
 */
gulong GstPlayer::get_xwindow_id()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

#ifdef GDK_WINDOWING_X11
	const gulong xWindowId = GDK_WINDOW_XWINDOW(m_video_window->gobj());
#elif defined(GDK_WINDOWING_WIN32)
	const gulong xWindowId = gdk_win32_drawable_get_handle(m_video_window->gobj());
#elif defined(GDK_WINDOWING_QUARTZ)
	const gulong xWindowId = gdk_quartz_window_get_nswindow(m_video_window->gobj());
#else
	#error unimplemented GTK backend
#endif

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "xWindowId=%d", xWindowId);

	return xWindowId;
}

/*
 * Set up the XWindowID to the XOverlay.
 */
void GstPlayer::set_xoverlay_window_id()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	// FIXME
	if(m_xWindowId == 0)
		g_warning("m_xWindowId is not initialized");
	else
	{
		//const gulong xWindowId = get_xwindow_id();
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "xoverlay->set_xwindow_id...");
		
		m_xoverlay->set_xwindow_id(m_xWindowId);

		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "xoverlay->set_xwindow_id... ok");
	}
}

/*
 */
void GstPlayer::update_pipeline_state_and_timeout()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(!m_pipeline)
		return;
	Gst::State old_st, new_st;
	m_pipeline->get_state(old_st, new_st, 100 * GST_MSECOND);
	on_timeout();
}
