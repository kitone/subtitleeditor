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
 
#include "GStreamerPlayer.h"
#include <gst/interfaces/xoverlay.h>
#include "debug.h"
#include "utility.h"
#include "Config.h"
#include "SubtitleTime.h"
#include <map>
#include <gdk/gdkx.h>

/*
 *	Video player Architecture
 *
 *  +--------------------------------------------+
 *  | pipeline                                   |
 *  |	    +-------+    +---------+    +--------+ |
 *  |     |filesrc|    |decodebin|    |audiobin| |
 *  |     |      src-sink       src-sink       | |
 *  |   	+-------+    |         |    +--------+ |
 *  |                  |         |    |videobin| |
 *  |                  |        src-sink       | |
 *  |                  +---------+    +--------+ |
 *  +--------------------------------------------+
 *
 *	VIDEO:
 *
 *  +---------------------------------------------------+
 *  | video output                                      |
 *  |	      +----------+    +----------+    +---------+ |
 *  |       |colorspace|    |videoscale|    |videosink| |
 *  |    +-sink       src-sink        src-sink        | |
 *  |    |	+----------+    +----------+    +---------+ |
 * sink--+                                              |
 *  +---------------------------------------------------+
 *
 *
 *  +-------------------------------------------------------+
 *  | videobin                                              |
 *  |	      +----------+    +-----------+    +------------+ |
 *  |       |colorspace|    |textoverlay|    |video output| |
 *  |    +-sink       src-sink         src-sink           | |
 *  |    |	+----------+    +-----------+    +------------+ |
 * sink--+                                                  |
 *  +-------------------------------------------------------+
 *
 *  AUDIO:
 *
 *  +-------------------------------------+
 *  | audiobin                            |
 *  |	      +------------+    +---------+ |
 *  |       |audioconvert|    |audiosink| |
 *  |    +-sink         src-sink        | |
 *  |    |	+------------+    +---------+ |
 * sink--+                                |
 *  +-------------------------------------+
 *
 *
 *
 */

/*
 *
 */
bool g_object_has_property(GObject *obj, const char *name)
{
	if(g_object_class_find_property(G_OBJECT_GET_CLASS(obj), name) != NULL)
		return true;
	return false;
}

/*
 *
 */
gboolean GStreamerPlayer::__static_bus_message(GstBus *bus, GstMessage *message, gpointer data)
{
	return ((GStreamerPlayer*)data)->bus_message(bus, message);
}

GstBusSyncReply GStreamerPlayer::__static_element_msg_sync(GstBus *bus, GstMessage *msg, gpointer data)
{
	return ((GStreamerPlayer*)data)->on_element_msg_sync(bus, msg);
}

/*
 *
 */
GStreamerPlayer::GStreamerPlayer()
:m_pipeline(NULL), m_filesrc(NULL), m_textoverlay(NULL), m_audiosink(NULL), m_videosink(NULL), m_pipeline_state(GST_STATE_NULL)
{
	Config::getInstance().signal_changed("video-player").connect(
			sigc::mem_fun(*this, &GStreamerPlayer::on_config_video_player_changed));
}

/*
 *
 */
GStreamerPlayer::~GStreamerPlayer()
{
	close();
}

/*
 *	crée un element grace à gst_element_factory_make
 *	s'il y a une erreur affiche un message d'erreur dans un dialog (msg_error)
 */
GstElement* GStreamerPlayer::create_element(const std::string &element, const std::string &name, const Glib::ustring &msg_error)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "element='%s' name='%s'", element.c_str(), name.c_str());

	GstElement *el = gst_element_factory_make(element.c_str(), name.c_str());

	if(el)
		return el;

	// Error. Could not create element
	GST_ELEMENT_ERROR(m_pipeline, RESOURCE, NOT_FOUND, (msg_error.c_str()), (NULL));

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "create element '%s' failed", element.c_str());
	return NULL;
}

/*
 *
 */
bool GStreamerPlayer::create_pipeline()
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "create pipeline");

	GstElement *decodebin = NULL;
	
	m_pipeline = gst_pipeline_new("pipeline");

	m_filesrc = gst_element_factory_make("filesrc", "filesrc");
	g_return_val_if_fail(m_filesrc, false);

#warning "USE_DECODEBIN2 ??"
	
	if(g_getenv("USE_DECODEBIN2"))
	{
		decodebin = gst_element_factory_make("decodebin2", "decodebin");
		std::cout << "USE_DECODEBIN2" << std::endl;
	}
	else
	{
		decodebin = gst_element_factory_make("decodebin", "decodebin");
	}

	if(decodebin == NULL)
	{
		dialog_error(
				_("Failed to create a GStreamer 'decodebin'"),
				_("Please check your GStreamer installation"));
		
		return false;
	}

	g_signal_connect(G_OBJECT(decodebin), "new-decoded-pad",
			G_CALLBACK(__static_newpad), this);

	gst_bin_add_many(GST_BIN(m_pipeline), m_filesrc, decodebin, NULL);
	gst_element_link(m_filesrc, decodebin);


	// create bus...
	GstBus *m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));

	gst_bus_set_sync_handler(m_bus,  __static_element_msg_sync, this);
	gst_bus_add_watch(GST_BUS(m_bus), __static_bus_message, this);
	gst_object_unref(m_bus);

	return true;
}

/*
 *
 */
GstElement* GStreamerPlayer::gen_video_element()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	// config value
	Glib::ustring cfg_videosink;
	Glib::ustring cfg_font_desc;
	bool cfg_shaded_background;
	bool cfg_force_aspect_ratio;

	// load config
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "load config");

	Config &cfg = Config::getInstance();

	cfg.get_value_bool("video-player", "force-aspect-ratio", cfg_force_aspect_ratio);
	cfg.get_value_string("video-player", "video-sink", cfg_videosink);
	cfg.get_value_bool("video-player", "shaded-background", cfg_shaded_background);
	cfg.get_value_string("video-player", "font-desc", cfg_font_desc);
	
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "create elements");

	// elements
	GstElement *bin = NULL;
	GstElement *conv = NULL;
	GstElement *textoverlay = NULL;
	GstElement *sink = NULL;
	GstPad *pad = NULL;


	conv = create_element("ffmpegcolorspace", "conv", 
			build_message(_("Failed to create a GStreamer converts video (%s). "
				"Please check your GStreamer installation."), "ffmpegcolorspace"));

	textoverlay = create_element("textoverlay", "overlay",
			build_message(_("Failed to create a GStreamer text overlay (%s). "
				"Please check your GStreamer installation."), "textoverlay"));

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "create video sink");

	Glib::ustring description = build_message(
			"ffmpegcolorspace name=videocsp ! videoscale name=videoscale ! %s name=videosink", 
			cfg_videosink.c_str());

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "description:'%s'", description.c_str());

	GError *error = NULL;
	sink = gst_parse_bin_from_description(description.c_str(), true, &error);
			
	if(error)
	{
		std::cerr << GST_STR_NULL(error->message) << std::endl;
		g_error_free(error);
	}

	// check result
	if(conv == NULL || textoverlay == NULL || sink == NULL)
	{
		if(GST_IS_OBJECT(conv))
			g_object_unref(G_OBJECT(conv));

		if(GST_IS_OBJECT(textoverlay))
			g_object_unref(G_OBJECT(textoverlay));

		if(GST_IS_OBJECT(sink))
			g_object_unref(G_OBJECT(sink));

		return NULL;
	}

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "create videobin");

	bin = gst_bin_new("videobin");

	// add and link
	gst_bin_add_many(GST_BIN(bin), conv, textoverlay, sink, NULL);
	
	gst_element_link_pads(conv, "src", textoverlay, "video_sink");
	gst_element_link_pads(textoverlay, "src", sink, "sink");

	// add sink pad to bin
	pad = gst_element_get_pad(conv, "sink");
	gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad));
	gst_object_unref(pad);


	// setup text overlay
	g_object_set(G_OBJECT(textoverlay),
			"halign", "center",
			"valign", "bottom",
			"shaded-background", cfg_shaded_background, 
			NULL);

	if(!cfg_font_desc.empty())
	{
		g_object_set(G_OBJECT(textoverlay),
				"font-desc", cfg_font_desc.c_str(),
				NULL);
	}

	// setup ptr
	m_textoverlay = textoverlay;
	m_videosink = gst_bin_get_by_name(GST_BIN(sink), "videosink");

	// setup video output
	if(g_object_has_property(G_OBJECT(m_videosink), "force-aspect-ratio"))
	{
		g_object_set(G_OBJECT(m_videosink), 
				"force-aspect-ratio", cfg_force_aspect_ratio, 
				NULL);
	}

	return bin;
}

/*
 *
 */
GstElement* GStreamerPlayer::gen_audio_element()
{
	Config &cfg = Config::getInstance();

	Glib::ustring cfg_audiosink;

	cfg.get_value_string("video-player", "audio-sink", cfg_audiosink);

	GstElement *bin = NULL;
	GstElement *conv = NULL;
	GstElement *sink = NULL;
	GstPad *pad = NULL;

	conv = create_element("audioconvert", "conv",
			build_message(_("Failed to create a GStreamer converts video (%s). "
				"Please check your GStreamer installation."), "audioconvert"));

	sink = create_element(cfg_audiosink, "sink",
			build_message(_("Failed to create a GStreamer audio output (%s). "
				"Please select another audio output or check your GStreamer installation."), cfg_audiosink.c_str()));

	// check result
	if(conv == NULL || sink == NULL)
	{
		if(GST_IS_OBJECT(conv))
			g_object_unref(G_OBJECT(conv));

		if(GST_IS_OBJECT(sink))
			g_object_unref(G_OBJECT(sink));

		return NULL;
	}

	bin = gst_bin_new("audiobin");

	// add and link
	gst_bin_add_many(GST_BIN(bin), conv, sink, NULL);
	gst_element_link_pads(conv, "src", sink, "sink");

	// add sink pad to bin
	pad = gst_element_get_pad(conv, "sink");
	gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad));
	gst_object_unref(pad);

	m_audiosink = bin;

	return bin;
}

	

/*
 *	open movie
 */
bool GStreamerPlayer::open(const Glib::ustring &uri)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "%s", uri.c_str());

	// clear old uri
	m_uri = Glib::ustring();

	Config::getInstance().set_value_bool("interface", "display-video-player", true);

	// delete old pipeline and setup NULL
	setup_null();

	// create pipeline
	if(create_pipeline())
	{
		m_uri = uri;
		Glib::ustring location = Glib::filename_from_uri(uri);

		std::cout << location << std::endl;

		g_object_set(G_OBJECT(m_filesrc), "location", location.c_str(), NULL);

		ready();
		pause();

	  gst_element_get_state(m_pipeline, NULL, NULL, 100 * GST_MSECOND);
		return true;
	}
	else
	{
		m_uri = Glib::ustring();
		setup_null();
	}
	return false;
}

void GStreamerPlayer::setup_null()
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "setup null...");

	if(GST_IS_ELEMENT(m_pipeline))
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "set pipeline state NULL...");
		set_state(DISABLE);

		//if(is_playing())
		{
			GstMessage *msg = NULL;
			GstBus *bus = NULL;

			gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_NULL);
	
			bus = gst_element_get_bus(GST_ELEMENT(m_pipeline));
			while((msg = gst_bus_poll(bus, GST_MESSAGE_STATE_CHANGED, 0)))
			{
				gst_bus_async_signal_func(bus, msg, NULL);
				gst_message_unref(msg);
			}
			gst_object_unref(bus);
		}
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "set pipeline state NULL...OK");
	}

#define OBJECT_UNREF(obj)	\
	if(GST_IS_OBJECT(obj)) { \
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "unref object '%s'", #obj); \
		gst_element_set_state(GST_ELEMENT(obj), GST_STATE_NULL); \
		g_object_unref(G_OBJECT(obj)); \
		obj = NULL; \
	} else { \
		obj = NULL; \
	}

	OBJECT_UNREF(m_pipeline);
	OBJECT_UNREF(m_filesrc);
	OBJECT_UNREF(m_textoverlay);
	OBJECT_UNREF(m_videosink);
	OBJECT_UNREF(m_audiosink);

#undef OBJECT_UNREF
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "setup null...OK");

}

/*
 *
 */
bool GStreamerPlayer::close()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(m_videosink && GST_IS_X_OVERLAY(m_videosink))
	{
		//gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(m_videosink), 0);
	}

	if(GST_IS_ELEMENT(m_pipeline))
	{
		gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_NULL);
		gst_object_unref(GST_OBJECT(m_pipeline));
	}

	m_pipeline=NULL;
	m_filesrc=NULL;
	m_textoverlay=NULL;
	m_audiosink=NULL;
	m_videosink=NULL;

	m_current_text = Glib::ustring();
	return true;
}

/*
 *
 */
bool GStreamerPlayer::is_playing()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);
	
	if(!GST_IS_ELEMENT(m_pipeline))
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "m_pipeline == NULL");
		return false;
	}
	/*
	GstState state;
	gst_element_get_state(GST_ELEMENT(m_pipeline), &state, NULL, GST_CLOCK_TIME_NONE);
	*/
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, (m_pipeline_state == GST_STATE_PLAYING) ? "PLAYING" : "NOT PLAYING");

	return (m_pipeline_state == GST_STATE_PLAYING);
}

/*
 *	init le pipeline a GST_STATE_READY
 */
void GStreamerPlayer::ready()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(!GST_IS_ELEMENT(m_pipeline))
		return;

	if(gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_READY) == GST_STATE_CHANGE_FAILURE)
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "GST_STATE_CHANGE_FAILURE");
	}
}

/*
 *
 */
void GStreamerPlayer::play()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(!GST_IS_ELEMENT(m_pipeline))
		return;

	if(gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "GST_STATE_CHANGE_FAILURE");
	}
	else
		on_timeout();

  gst_element_get_state(m_pipeline, NULL, NULL, 100 * GST_MSECOND);
}

/*
 *
 */
void GStreamerPlayer::pause()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(!GST_IS_ELEMENT(m_pipeline))
		return;

	if(gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE)
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "GST_STATE_CHANGE_FAILURE");
	}
}

/*
 *
 */
long GStreamerPlayer::get_duration()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	g_return_val_if_fail(GST_IS_ELEMENT(m_pipeline), 0);

	gint64 len = 0;
	
	GstFormat fmt = GST_FORMAT_TIME;
	
	if(gst_element_query_duration(GST_ELEMENT(m_pipeline), &fmt, &len))
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "duration=%s", SubtitleTime(len / GST_MSECOND).str().c_str());
		return (long)(len / GST_MSECOND);
	}
	return 0;
}

/*
 *
 */
long GStreamerPlayer::get_position()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	g_return_val_if_fail(GST_IS_ELEMENT(m_pipeline), 0);

	gint64 pos = 0;
	
	GstFormat fmt = GST_FORMAT_TIME;
	
	if(gst_element_query_position(GST_ELEMENT(m_pipeline), &fmt, &pos))
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "position=%s", SubtitleTime(pos / GST_MSECOND).str().c_str());

		return (long)(pos / GST_MSECOND);
	}
	return 0;
}

/*
 *
 */
void GStreamerPlayer::seek(long position, bool faster)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "seek [%s]", SubtitleTime(position).str().c_str());

	position = CLAMP(position, 0, get_duration());

	gint64 pos = position * GST_MSECOND;
	gint64 dur = (gint64)(get_duration() * GST_MSECOND);

	gst_element_seek(m_pipeline, 1.0,
			GST_FORMAT_TIME, 
			(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | (faster ? GST_SEEK_FLAG_KEY_UNIT : GST_SEEK_FLAG_ACCURATE)), 
			GST_SEEK_TYPE_SET, pos,
			GST_SEEK_TYPE_SET, dur);
			//GST_SEEK_TYPE_NONE, 0); //don't work

  gst_element_get_state(m_pipeline, NULL, NULL, 100 * GST_MSECOND);

	// update video player (slider, waveform ...)
	on_timeout();
}
	
/*
 *
 */
void GStreamerPlayer::seek(long _start, long _end, bool faster)
{
	if(_start < 0) _start = 0;
	if(_end < 0) _end = 0;

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "seek [%s] [%s]", SubtitleTime(_start).str().c_str(), SubtitleTime(_end).str().c_str());

	gint64 start = (gint64)_start * GST_MSECOND;
	gint64 end = (gint64)_end * GST_MSECOND;

	if(start > end)
	{
		gint64 tmp=start;
		start=end;
		end=tmp;
	}
/*
	std::cout << "seek --------------------------" << std::endl;
	std::cout << "seek s: " << start << std::endl;
	std::cout << "seek e: " << end << std::endl;
*/
	/*bool res = */gst_element_seek(m_pipeline, 1.0,
			GST_FORMAT_TIME, 
			(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | (faster ? GST_SEEK_FLAG_KEY_UNIT : GST_SEEK_FLAG_ACCURATE)), 
			GST_SEEK_TYPE_SET, start,
			GST_SEEK_TYPE_SET, end);
	//return res;

  gst_element_get_state(m_pipeline, NULL, NULL, 100 * GST_MSECOND);

	on_timeout();
}

/*
 *
 */
void GStreamerPlayer::show_text(const Glib::ustring& text)
{
	if(G_IS_OBJECT(m_textoverlay) && GST_IS_ELEMENT(m_textoverlay))
	{
		if(m_current_text != text)
		{
			se_debug_message(SE_DEBUG_VIDEO_PLAYER, "text=%s", text.c_str());
	
			g_object_set(G_OBJECT(m_textoverlay),
					"text", text.c_str(),
					NULL);

			m_current_text = text;
		}
	}
	else
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "doesn't have textoverlay");
	}
}

/*
 *
 */
bool GStreamerPlayer::bus_message(GstBus *bus, GstMessage *msg)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, 
			"type='%s' name='%s'", 
			GST_MESSAGE_TYPE_NAME(msg), GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)));

	if(msg->type == GST_MESSAGE_ERROR)
	{
		bus_message_error(bus, msg);
	}
	else if(msg->type == GST_MESSAGE_WARNING)
	{
		bus_message_warning(bus, msg);
	}
	else if(msg->type == GST_MESSAGE_EOS)
	{
		bus_message_eos(bus, msg);
	}
	else if(msg->type == GST_MESSAGE_STATE_CHANGED)
	{
		bus_message_state_changed(bus, msg);
	}
	else if(msg->type == GST_MESSAGE_INFO)
	{
		set_state(READY);
	}
	else
	{
#warning "FIXME: clean this!"

		Glib::ustring temp =  GST_MESSAGE_TYPE_NAME(msg);

		if(temp != "element")
			return true;

		const gchar* type_name = NULL;
		gchar* src_name = NULL;
	
		src_name = gst_object_get_name(msg->src);

		if(msg->structure)
			type_name = gst_structure_get_name(msg->structure);

		if(type_name == NULL)
		{
			se_debug_message(SE_DEBUG_VIDEO_PLAYER, 
					"Unhandled element message %s from %s: %" GST_PTR_FORMAT,
					GST_STR_NULL(type_name), GST_STR_NULL(src_name), msg);

			g_free(src_name);
			return true;
		}
	
		if(strcmp(type_name, "prepare-xwindow-id") == 0 || strcmp(type_name, "have-xwindow-id") == 0)
		{
			Glib::RefPtr<Gdk::Window> window = get_widget()->get_window();
			
			GstXOverlay *xoverlay = GST_X_OVERLAY( GST_MESSAGE_SRC(msg) );
	
			gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(xoverlay), GDK_WINDOW_XWINDOW(window->gobj()));
		}
		g_free(src_name);

	}
	return true;
}

GstBusSyncReply GStreamerPlayer::on_element_msg_sync(GstBus *bus, GstMessage *msg)
{
	// ignore anything but 'prepare-xwindow-id' element messages
	if(GST_MESSAGE_TYPE (msg) != GST_MESSAGE_ELEMENT)
		return GST_BUS_PASS;
	
	if (!gst_structure_has_name (msg->structure, "prepare-xwindow-id"))
		return GST_BUS_PASS;

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "XOverlay: prepare-xwindow-id");

	Glib::RefPtr<Gdk::Window> window = get_widget()->get_window();

	GstXOverlay *xoverlay = GST_X_OVERLAY( GST_MESSAGE_SRC(msg) );
	
	gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(xoverlay), GDK_WINDOW_XWINDOW(window->gobj()));

	gst_message_unref (msg);

	return GST_BUS_DROP;
}

/*
 *
 */
void GStreamerPlayer::widget_expose(Gtk::Widget *widget)
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(GST_IS_ELEMENT(m_videosink) && GST_IS_X_OVERLAY(m_videosink))
	{
		Glib::RefPtr<Gdk::Window> window = widget->get_window();
		if(window)
		{
			gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(m_videosink), GDK_WINDOW_XID(window->gobj()));
			gst_x_overlay_expose(GST_X_OVERLAY(m_videosink));
		}
	}
}

/*
 *
 */
void GStreamerPlayer::on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "'%s'='%s'", key.c_str(), value.c_str());

	if(!G_IS_OBJECT(m_pipeline))
		return;
	if(!GST_IS_ELEMENT(m_pipeline))
		return;

	if(key == "force-aspect-ratio" && G_IS_OBJECT(m_videosink))
	{
		bool state;
		if(from_string(value, state))
		{
			if(g_object_class_find_property(G_OBJECT_GET_CLASS(m_videosink), "force-aspect-ratio") != NULL)
				g_object_set(G_OBJECT(m_videosink), "force-aspect-ratio", state, NULL);
		}
		get_widget()->queue_draw();
	}
	else if(key == "shaded-background" && G_IS_OBJECT(m_textoverlay))
	{
		bool state;
		if(from_string(value, state) && m_textoverlay!=NULL)
			g_object_set(G_OBJECT(m_textoverlay), "shaded-background", state, NULL);
	}
	else if(key == "font-desc" && G_IS_OBJECT(m_textoverlay))
	{
			g_object_set(G_OBJECT(m_textoverlay), "font-desc", value.c_str(), NULL);
	}
	else if(key == "brightness" && G_IS_OBJECT(m_videosink))
	{
		double val = 0;
		if(from_string(value, val))
			if(g_object_class_find_property(G_OBJECT_GET_CLASS(m_videosink), "brightness") != NULL)
				g_object_set(G_OBJECT(m_videosink), "brightness", (gint)val, NULL);
	}
	else if(key == "contrast" && G_IS_OBJECT(m_videosink))
	{
		double val = 0;
		if(from_string(value, val))
			if(g_object_class_find_property(G_OBJECT_GET_CLASS(m_videosink), "contrast") != NULL)
				g_object_set(G_OBJECT(m_videosink), "contrast", (gint)val, NULL);
	}
	else if(key == "saturation" && G_IS_OBJECT(m_videosink))
	{
		double val = 0;
		if(from_string(value, val))
			if(g_object_class_find_property(G_OBJECT_GET_CLASS(m_videosink), "saturation") != NULL)
				g_object_set(G_OBJECT(m_videosink), "saturation", (gint)val, NULL);
	}
	else if(key == "hue" && G_IS_OBJECT(m_videosink))
	{
		double val = 0;
		if(from_string(value, val))
			if(g_object_class_find_property(G_OBJECT_GET_CLASS(m_videosink), "hue") != NULL)
				g_object_set(G_OBJECT(m_videosink), "hue", (gint)val, NULL);
	}
}

/*
 *
 */
void GStreamerPlayer::on_newpad(GstElement *element, GstPad *pad, bool last)
{
	GstCaps *caps = NULL;
	GstStructure *structure = NULL;
	GstPad *sinkpad = NULL;
	GstElement *sink = NULL;
	const gchar *mimetype = NULL;
	GstStateChangeReturn ret;
	GstPadLinkReturn lret;

	// check media type
	caps = gst_pad_get_caps(pad);

	//if(caps == NULL || gst_caps_is_empty(caps) || gst_caps_is_any(caps))
	//	goto no_type;

	// get the mime type
	structure = gst_caps_get_structure(caps, 0);
	mimetype = gst_structure_get_name(structure);

	std::cout << "mimetype: " << mimetype << std::endl;

	if(g_str_has_prefix(mimetype, "audio/") && (m_audiosink == NULL))
	{
		sink = gen_audio_element();
	}
	else if(g_str_has_prefix(mimetype, "video/") && (m_videosink == NULL))
	{
		sink = gen_video_element();
	}
	else if(g_str_has_prefix(mimetype, "text/"))
	{
		if(m_audiosink == NULL || m_videosink == NULL)
		{
			GST_ELEMENT_ERROR(m_pipeline, STREAM, WRONG_TYPE,
					(_("Only a text stream was detected. "
						 "Either you are loading a subtitle file or some other type of "
						 "text file, or the media file was not recognized. "
						 "This is not a media file.")), (NULL));
		}

		sink = NULL;
	}
	else
	{
		sink = NULL;
	}
	
	gst_caps_unref(caps);

	// traitement
	if(sink)
	{
		// add new sink to the pipeline
		gst_bin_add(GST_BIN_CAST(m_pipeline), sink);

		// set the new sink tp PAUSED as well
		ret = gst_element_set_state(sink, GST_STATE_PAUSED);
		if(ret == GST_STATE_CHANGE_FAILURE)
			goto state_error;

		// get the ghostpad of the sink bin
		sinkpad = gst_element_get_pad(sink, "sink");

		// link'n'play
		lret = gst_pad_link(pad, sinkpad);
		if(lret != GST_PAD_LINK_OK)
			goto link_failed;

		gst_object_unref(sinkpad);
	}

	return;

	// ERRORS
state_error:
	{
		gst_bin_remove(GST_BIN_CAST(m_pipeline), sink);
		g_warning("could not change state of new sink (%d)", ret);
		return;
	}
link_failed:
	{
		g_warning("could not link pad and sink (%d)", lret);
		return;
	}
}

/*
 *
 */
void GStreamerPlayer::__static_newpad(GstElement *e, GstPad *p, gboolean last, gpointer data)
{
	((GStreamerPlayer*)data)->on_newpad(e,p, last);
}


/*
 *
 */
void GStreamerPlayer::bus_message_warning(GstBus *bus, GstMessage *msg)
{
	if(GST_MESSAGE_TYPE(msg) != GST_MESSAGE_WARNING)
		return;
	
	GError *error = NULL;
	gchar* debug = NULL;

	gst_message_parse_warning(msg, &error, &debug);

	GST_DEBUG("Warning message: %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "GST_MESSAGE_WARNING : %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));

	g_warning ("%s [%s]", GST_STR_NULL (error->message), GST_STR_NULL (debug));

	g_error_free(error);
	g_free(debug);
}

/*
 *
 */
void GStreamerPlayer::bus_message_error(GstBus *bus, GstMessage *msg)
{
	if(GST_MESSAGE_TYPE(msg) != GST_MESSAGE_ERROR)
		return;

	GError *error = NULL;
	gchar* debug = NULL;

	gst_message_parse_error(msg, &error, &debug);

	GST_DEBUG("Error message: %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));
		
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "GST_MESSAGE_ERROR : %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));

	dialog_error(build_message(_("Media file could not be played.\n%s"), m_uri.c_str()), GST_STR_NULL(error->message));

	g_error_free(error);
	g_free(debug);

	setup_null();
}

/*
 *	message emit a la fin d'une lecteur
 *	valide pour la fin du fichier ou d'un segement (play subtitle)
 */
void GStreamerPlayer::bus_message_eos(GstBus *bus, GstMessage *msg)
{
	if(GST_MESSAGE_TYPE(msg) != GST_MESSAGE_EOS)
		return;

	m_pipeline_state = GST_STATE_PAUSED;

	set_state(PAUSED);

//#warning "CHECK ME!"
//	seek(get_position());
}

/*
 *	traitement des changements d'etat dans le pipeline 
 *	modifie l'etat du lecteur en consequence (disable, paused, playing)
 *
 *	seul le pipeline est ecouter pour une question de performance
 */
void GStreamerPlayer::bus_message_state_changed(GstBus *bus, GstMessage *msg)
{
	if(GST_MESSAGE_TYPE(msg) != GST_MESSAGE_STATE_CHANGED)
		return;

	// on ne traite que les changements du pipeline
	if(GST_MESSAGE_SRC(msg) != GST_OBJECT(m_pipeline))
		return;

	GstState old_state, new_state;

	gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "transition %s->%s",
					gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));

	if(old_state == new_state)
		return;
	
	GstStateChange transition = (GstStateChange)GST_STATE_TRANSITION(old_state, new_state);

	if(transition == GST_STATE_CHANGE_NULL_TO_READY)
	{
	}
	else if(transition == GST_STATE_CHANGE_READY_TO_PAUSED)
	{
		set_state(READY);
		set_state(PAUSED);
	}
	else if(transition == GST_STATE_CHANGE_PAUSED_TO_PLAYING)
	{
		set_state(PLAYING);
	}
	else if(transition == GST_STATE_CHANGE_PLAYING_TO_PAUSED)
	{
		set_state(PAUSED);
	}
	else if(transition == GST_STATE_CHANGE_PAUSED_TO_READY)
	{
	}
	else if(transition == GST_STATE_CHANGE_READY_TO_NULL)
	{
		set_state(DISABLE);
	}

	m_pipeline_state = new_state;
}

