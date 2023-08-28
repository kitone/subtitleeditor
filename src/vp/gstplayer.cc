// subtitleeditor -- a tool to create or edit subtitle
//
// https://kitone.github.io/subtitleeditor/
// https://github.com/kitone/subtitleeditor/
//
// Copyright @ 2005-2018, kitone
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "gstplayer.h"

#include <debug.h>
#include <gst/pbutils/missing-plugins.h>
#include <gstreamer_utility.h>
#include <i18n.h>
#include <utility.h>

static gboolean vp_handle_message(GstBus *bus, GstMessage *msg, void *data) {
  GstPlayer *p = static_cast<GstPlayer *>(data);
  return p->on_bus_message(bus, msg);
}

// Constructor
// Init values
GstPlayer::GstPlayer() {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  m_pipeline = nullptr;
  m_gtksink_widget = nullptr;
  m_textoverlay = nullptr;
  m_watch_id = 0;
  m_pipeline_state = GST_STATE_NULL;
  m_pipeline_duration = GST_CLOCK_TIME_NONE;
  m_pipeline_rate = 1.0;
  m_pipeline_async_done = false;
  m_loop_seek = cfg::get_boolean("video-player", "repeat");

  show();

  cfg::signal_changed("video-player").connect(sigc::mem_fun(*this, &GstPlayer::on_config_video_player_changed));
}

// Destructor
// Set up pipeline to NULL.
GstPlayer::~GstPlayer() {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  if (m_pipeline) {
    set_pipeline_state(GST_STATE_NULL);
  }
}

// Create the pipeline and sets the uri.
bool GstPlayer::open(const Glib::ustring &uri) {
  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "try to open uri '%s'", uri.c_str());
  // we need to make sure the widget is realized
  realize_if_needed();

  if (!create_pipeline()) {
    se_dbg_msg(SE_DBG_VIDEO_PLAYER, "could not open uri");
    return false;
  }
  // setup the uri property and init the player state to paused
  g_object_set(m_pipeline, "uri", uri.c_str(), NULL);
  m_uri = uri;

  return set_pipeline_state(GST_STATE_PAUSED);
}

// Set up the pipeline to NULL.
void GstPlayer::close() {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  set_pipeline_null();
}

// Return the uri of the current video.
Glib::ustring GstPlayer::get_uri() {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  if (!m_pipeline)
    return Glib::ustring();
  return m_uri;
}

// Sets the pipeline state to playing.
void GstPlayer::play() {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  set_pipeline_state(GST_STATE_PLAYING);
}

// Try to play the segment defined by the subtitle (from start to end).
// This function supports the looping.
// The state is sets to playing.
void GstPlayer::play_subtitle(const Subtitle &sub) {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  if (!m_pipeline)
    return;
  GstSeekFlags flags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE);
  if (m_loop_seek) {
    flags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_SEGMENT);
    m_subtitle_play = sub;
  }
  // if the seek success, we update the timeout and
  // swap the pipeline state to playing
  if (seek(sub.get_start().totalmsecs, sub.get_end().totalmsecs, flags)) {
    update_pipeline_state_and_timeout();
    set_pipeline_state(GST_STATE_PLAYING);
  }
}

// Try to play the segment defined (start to end).
// This function don't support the mode looping.
// The state is sets to playing.
void GstPlayer::play_segment(const SubtitleTime &start, const SubtitleTime &end) {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  if (!m_pipeline)
    return;

  GstSeekFlags flags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE);

  if (seek(start.totalmsecs, end.totalmsecs, flags))
    update_pipeline_state_and_timeout();
  set_pipeline_state(GST_STATE_PLAYING);
}

// Sets the pipeline state to paused.
void GstPlayer::pause() {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  set_pipeline_state(GST_STATE_PAUSED);
}

// Return true if the state of the pipeline is playing.
bool GstPlayer::is_playing() {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  return (m_pipeline_state == GST_STATE_PLAYING);
}

// Return the duration of the stream or 0.
long GstPlayer::get_duration() {
  //  se_dbg(SE_DBG_VIDEO_PLAYER);

  if (!m_pipeline) {
    se_dbg_msg(SE_DBG_VIDEO_PLAYER, "0 because no pipeline");
    return 0;
  }

  return GST_TIME_AS_MSECONDS(m_pipeline_duration);
}

// Return the current position in the stream.
long GstPlayer::get_position() {
  //  se_dbg(SE_DBG_VIDEO_PLAYER);

  if (!m_pipeline) {
    se_dbg_msg(SE_DBG_VIDEO_PLAYER, "0 because no pipeline");
    return 0;
  }

  gint64 pos = 0;
  if (!gst_element_query_position(m_pipeline, GST_FORMAT_TIME, &pos))
    return 0;
  return GST_TIME_AS_MSECONDS(pos);
}

bool GstPlayer::seek(long start, long end, const GstSeekFlags &flags) {
  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "try to seek %s (%d) - %s (%d)", SubtitleTime(start).str().c_str(), start, SubtitleTime(end).str().c_str(), end);

  if (!m_pipeline)
    return false;
  long dur = get_duration();
  // clamp
  start = CLAMP(start, 0, dur);
  end = CLAMP(end, 0, dur);
  // check the order
  if (start > end)
    std::swap(start, end);
  // convert to gstreamer time
  gint64 gstart = start * GST_MSECOND;
  gint64 gend = end * GST_MSECOND;

  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "pipeline->seek(%" GST_TIME_FORMAT ", %" GST_TIME_FORMAT ")", GST_TIME_ARGS(gstart), GST_TIME_ARGS(gend));

  // out of range ?
  if (start == end) {
    return false;
  }

  bool ret = gst_element_seek(m_pipeline, m_pipeline_rate, GST_FORMAT_TIME, flags, GST_SEEK_TYPE_SET, gstart, GST_SEEK_TYPE_SET, gend);

  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "result of seek %s", (ret) ? "true" : "false");

  return ret;
}

// Seeking, the state of the pipeline is not modified.
void GstPlayer::seek(long position) {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  if (!m_pipeline)
    return;

  GstSeekFlags flags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE);
  if (seek(position, get_duration(), flags))
    update_pipeline_state_and_timeout();
}

// Update the text overlay with this new text.
void GstPlayer::set_subtitle_text(const Glib::ustring &text) {
  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "text='%s'", text.c_str());

  if (!m_textoverlay)
    return;

  Glib::ustring corrected = text;
  utility::replace(corrected, "&", "&amp;");
  g_object_set(G_OBJECT(m_textoverlay), "text", corrected.c_str(), NULL);
}

// Sets the new playback rate. Used for slow or fast motion.
// Default value : 1.0
// Min : 0.1
// Max : 1.5
void GstPlayer::set_playback_rate(double value) {
  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "rate=%f", value);

  value = CLAMP(value, 0.1, 1.5);  // FIXME

  m_pipeline_rate = value;

  if (seek(get_position(), get_duration(), GST_SEEK_FLAG_FLUSH))
    update_pipeline_state_and_timeout();
}

// Return the playback rate.
double GstPlayer::get_playback_rate() {
  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "pipeline_rate=%f", m_pipeline_rate);

  return m_pipeline_rate;
}

// Enable/Disable the repeat mode.
// Works only with play_subtitle.
void GstPlayer::set_repeat(bool state) {
  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "state=%s", (state) ? "true" : "false");

  m_loop_seek = state;
  // FIXME flush pipeline ?
}

// Create a gstreamer pipeline (Gst::PlayBin2), initialize the
// audio and video sink with the configuration.
// Connect the bug message to the player.
bool GstPlayer::create_pipeline() {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  // Clean or destroy the old pipeline
  set_pipeline_null();

  m_pipeline = gst_element_factory_make("playbin3", NULL);

  GstElement *videosink = gen_video_element();
  g_object_set(GST_OBJECT(m_pipeline), "video-sink", videosink, NULL);

  show_all();

  // Add a bus watch, so we get notified when a message arrives
  GstBus *bus = gst_element_get_bus(m_pipeline);
  m_watch_id = gst_bus_add_watch(bus, (GstBusFunc)vp_handle_message, this);
  gst_object_unref(bus);
  return true;
}

// Return a gstreamer audio sink from the configuration option.
// Glib::RefPtr<Gst::Element> GstPlayer::gen_audio_element() {
//   se_dbg(SE_DBG_VIDEO_PLAYER);
//
//   Glib::ustring cfg_audiosink = cfg::get_string("video-player", "audio-sink");
//
//   try {
//     Glib::RefPtr<Gst::Element> sink = Gst::ElementFactory::create_element(cfg_audiosink, "audiosink");
//     if (!sink) {
//       throw std::runtime_error(build_message(_("Failed to create a GStreamer audio output (%s). "
//                                                "Please check your GStreamer installation."),
//                                              cfg_audiosink.c_str()));
//     }
//     return sink;
//   } catch (std::runtime_error &ex) {
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wformat-security"
//     se_dbg_msg(SE_DBG_VIDEO_PLAYER, "failed to gen_audio_element '%s'", ex.what());
//     GST_ELEMENT_WARNING(m_pipeline->gobj(), RESOURCE, NOT_FOUND, (ex.what()), (NULL));
// #pragma GCC diagnostic pop
//   }
//   // Return an NULL ptr
//   return Glib::RefPtr<Gst::Element>();
// }

// Return a gstreamer video sink from the configuration option.
GstElement *GstPlayer::gen_video_element() {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  Glib::ustring cfg_videosink = cfg::get_string("video-player", "video-sink");
  Glib::ustring cfg_font_desc = cfg::get_string("video-player", "font-desc");
  bool cfg_shaded_background = cfg::get_boolean("video-player", "shaded-background");
  bool cfg_force_aspect_ratio = cfg::get_boolean("video-player", "force-aspect-ratio");
  guint cfg_text_valignment = get_text_valignment_based_on_config();

  GError *error = nullptr;
  GstElement *videobin = gst_parse_bin_from_description("textoverlay name=textoverlay ! videoconvert ! gtksink name=gtksink", true, &error);
  if (error) {
    g_printerr("Error trying generate video element: %s\n", error);
    g_clear_error(&error);
    return nullptr;
  }

  // get gtksink widget
  GstElement *gtksink = gst_bin_get_by_name(GST_BIN(videobin), "gtksink");
  g_object_get(gtksink, "widget", &m_gtksink_widget, NULL);
  gtk_container_add(GTK_CONTAINER(gobj()), m_gtksink_widget);
  g_object_unref(m_gtksink_widget);
  gtk_widget_realize(m_gtksink_widget);

  // configure text overlay
  m_textoverlay = gst_bin_get_by_name(GST_BIN(videobin), "textoverlay");
  g_object_set(GST_OBJECT(m_textoverlay), "valignment", cfg_text_valignment, NULL);
  g_object_set(GST_OBJECT(m_textoverlay), "shaded_background", cfg_shaded_background, NULL);
  g_object_set(GST_OBJECT(m_textoverlay), "font_desc", cfg_font_desc.c_str(), NULL);

  return videobin;
}

// Set the state of the pipeline.
// The state change can be asynchronously.
bool GstPlayer::set_pipeline_state(GstState state) {
  if (!m_pipeline && m_pipeline_state == state) {
    return false;
  }
  GstStateChangeReturn ret = gst_element_set_state(m_pipeline, state);
  return (ret != GST_STATE_CHANGE_FAILURE);
}

// Sets the state of the pipeline to NULL.
void GstPlayer::set_pipeline_null() {
  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "set up pipeline to null...");

  if (!m_pipeline)
    return;

  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "set up pipeline to NULL");

  set_pipeline_state(GST_STATE_NULL);

  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "remove watch id");

  gtk_widget_destroy(m_gtksink_widget);
  g_object_unref(m_pipeline);
  g_source_remove(m_watch_id);

  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "set up all values to NULL");

  m_watch_id = 0;
  m_pipeline_state = GST_STATE_NULL;
  m_pipeline_duration = GST_CLOCK_TIME_NONE;
  m_pipeline_rate = 1.0;
  m_pipeline_async_done = false;

  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "clear RefPtr");

  m_pipeline = nullptr;
  m_gtksink_widget = nullptr;
  m_textoverlay = nullptr;
  // m_xoverlay.clear();

  set_player_state(NONE);

  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "set up pipeline to null... ok");
}

// Check if are missing plugin, if it's true display a message.
// Return true if missing.
bool GstPlayer::check_missing_plugins() {
  if (m_missing_plugins.empty())
    return false;

  gstreamer_utility::dialog_missing_plugins(m_missing_plugins);
  m_missing_plugins.clear();
  return true;
}

// Check if it's a Missing Plugin Message.
// Add the description of the missing plugin in the list.
bool GstPlayer::is_missing_plugin_message(GstMessage *msg) {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  if (!msg)
    return false;

  if (!gst_is_missing_plugin_message(msg))
    return false;

  gchar *description = gst_missing_plugin_message_get_description(msg);
  if (!description)
    return false;

  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "missing plugin msg '%s'", description);

  m_missing_plugins.push_back(description);
  g_free(description);
  return true;
}

// Dispatch the gstreamer message.
bool GstPlayer::on_bus_message(GstBus *bus, GstMessage *msg) {
  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "type='%s' name='%s'", GST_MESSAGE_TYPE_NAME(msg), GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)));

  GstMessageType msg_type = GST_MESSAGE_TYPE(msg);
  switch (msg_type) {
    case GST_MESSAGE_ELEMENT:
      on_bus_message_element(msg);
      break;
    case GST_MESSAGE_EOS:
      on_bus_message_eos(msg);
      break;
    case GST_MESSAGE_ERROR:
      on_bus_message_error(msg);
      break;
    case GST_MESSAGE_WARNING:
      on_bus_message_warning(msg);
      break;
    case GST_MESSAGE_STATE_CHANGED:
      on_bus_message_state_changed(msg);
      break;
    case GST_MESSAGE_SEGMENT_DONE:
      on_bus_message_segment_done(msg);
      break;
    case GST_MESSAGE_ASYNC_DONE:
      if (m_pipeline_async_done == false) {
        // We wait for the first async-done message, then the application
        // can ask about duration, info about the stream...
        m_pipeline_async_done = true;
        send_message(Player::STREAM_READY);
      }
      break;
    case GST_MESSAGE_STREAM_COLLECTION:
      on_bus_message_stream_collection(msg);
      break;
    default:
      break;
  }
  return true;
}

// Check the missing plugin.
// If is missing add in the list of missing plugins.
// This list should be show later.
void GstPlayer::on_bus_message_element(GstMessage *msg) {
  se_dbg(SE_DBG_VIDEO_PLAYER);
  is_missing_plugin_message(msg);
}

// An error is detected.
// Destroy the pipeline and show the error message in a dialog.
void GstPlayer::on_bus_message_error(GstMessage *msg) {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  check_missing_plugins();

  GError *err = nullptr;
  gchar *err_dbg = nullptr;
  gst_message_parse_error(msg, &err, &err_dbg);

  g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
  g_printerr("Debugging information: %s\n", err_dbg ? err_dbg : "none");

  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "GST_MESSAGE_ERROR : %s [%s]", err, err_dbg);

  dialog_error(build_message(_("Media file could not be played.\n%s"), get_uri().c_str()), err->message);

  g_clear_error(&err);
  g_free(err_dbg);

  set_pipeline_null();
}

// An warning message is detected.
void GstPlayer::on_bus_message_warning(GstMessage *msg) {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  check_missing_plugins();

  GError *err = nullptr;
  gchar *err_dbg = nullptr;
  gst_message_parse_error(msg, &err, &err_dbg);

  g_warning("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
  g_warning("Debugging information: %s\n", err_dbg ? err_dbg : "none");

  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "GST_MESSAGE_WARNING : %s [%s]", err, err_dbg);

  g_clear_error(&err);
  g_free(err_dbg);
}

// The state of the pipeline has changed.
// Update the player state.
void GstPlayer::on_bus_message_state_changed(GstMessage *msg) {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  // We only update when it's the pipeline object
  if (GST_MESSAGE_SRC(msg) != GST_OBJECT(m_pipeline))
    return;

  GstState old_state, new_state, pending;

  gst_message_parse_state_changed(msg, &old_state, &new_state, &pending);

  // Update the current state of the pipeline
  m_pipeline_state = new_state;

  if (old_state == GST_STATE_NULL && new_state == GST_STATE_READY) {
    set_player_state(NONE);
  } else if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED) {
    set_player_state(PAUSED);
    check_missing_plugins();
  } else if (old_state == GST_STATE_PAUSED && new_state == GST_STATE_PLAYING) {
    set_player_state(PLAYING);
  } else if (old_state == GST_STATE_PLAYING && new_state == GST_STATE_PAUSED) {
    set_player_state(PAUSED);
  } else if (old_state == GST_STATE_PAUSED && new_state == GST_STATE_READY) {
    set_player_state(NONE);
  } else if (old_state == GST_STATE_READY && new_state == GST_STATE_NULL) {
    set_player_state(NONE);
  }
}

// End-of-stream (segment or stream) has been detected,
// update the pipeline state to PAUSED.
// Seek to the beginning if it's the end of the stream.
void GstPlayer::on_bus_message_eos(GstMessage *msg) {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  // FIXME with seek_loop
  set_pipeline_state(GST_STATE_PAUSED);

  if (get_position() == get_duration())
    seek(0);
}

// The pipeline completed playback of a segment.
// If the looping is activated send new seek event.
// Works only with play_subtitle.
void GstPlayer::on_bus_message_segment_done(GstMessage *msg) {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  if (!(m_loop_seek && m_subtitle_play))
    return;

  // TODO debug information of MessageSegmentDone
  GstSeekFlags flags = (GstSeekFlags)(GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_SEGMENT);
  seek(m_subtitle_play.get_start().totalmsecs, m_subtitle_play.get_end().totalmsecs, flags);
}

//
void GstPlayer::on_bus_message_stream_collection(GstMessage *msg) {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  // GstStreamCollection *collection = nullptr;
  // gst_message_parse_stream_collection(msg, &collection);
  // if (!collection) {
  //   return;
  // }
  // guint numStreams = gst_stream_collection_get_size(collection);
  // for (guint i = 0; i < numStreams; i++) {
  //   GstStream *stream = gst_stream_collection_get_stream(collection, i);
  //   g_print("Stream %d: Type=%s, ID=%s\n", i, gst_stream_type_get_name(gst_stream_get_stream_type(stream)), gst_stream_get_stream_id(stream));
  // }

  // g_object_unref(collection);
}

// The video-player configuration has changed, update the player.
void GstPlayer::on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value) {
  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "%s %s", key.c_str(), value.c_str());

  if (key == "repeat") {
    set_repeat(utility::string_to_bool(value));
  }

  if (!m_pipeline) {
    return;
  }

  if (key == "shaded-background" && m_textoverlay) {
    g_object_set(G_OBJECT(m_textoverlay), "shaded_background", utility::string_to_bool(value), NULL);
  } else if (key == "font-desc" && m_textoverlay) {
    g_object_set(G_OBJECT(m_textoverlay), "font_desc", value.c_str(), NULL);
  }
}

void GstPlayer::update_pipeline_state_and_timeout() {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  if (!m_pipeline)
    return;
  GstState old_st, new_st;
  gst_element_get_state(m_pipeline, &old_st, &new_st, 100 * GST_MSECOND);
  got_tick();
}

// Set up the duration value of the stream if need.
bool GstPlayer::update_pipeline_duration() {
  se_dbg(SE_DBG_VIDEO_PLAYER);

  if (!m_pipeline)
    return false;

  m_pipeline_duration = GST_CLOCK_TIME_NONE;

  gint64 dur = -1;
  if (gst_element_query_duration(m_pipeline, GST_FORMAT_TIME, &dur) && dur != -1) {
    m_pipeline_duration = dur;
    se_dbg_msg(SE_DBG_VIDEO_PLAYER, "Success to query the duration (%" GST_TIME_FORMAT ")", GST_TIME_ARGS(dur));
    // send_message(STREAM_DURATION_CHANGED);
    return true;
  }
  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "The query of the duration of the stream failed");
  return false;
}

// Return the number of audio track.
gint GstPlayer::get_n_audio() {
  // FIXME
  // se_dbg_msg(SE_DBG_VIDEO_PLAYER, "n_audio: %d", (m_pipeline) ? m_pipeline->property_n_audio() : 0);
  // if (m_pipeline)
  //   return m_pipeline->property_n_audio();
  return 0;
}

// Sets the current audio track. (-1 = auto)
void GstPlayer::set_current_audio(gint track) {
  se_dbg_msg(SE_DBG_VIDEO_PLAYER, "track=%d", track);
  // FIXME
  // if (!m_pipeline)
  //   return;

  // if (track < -1)
  //   track = -1;
  // m_pipeline->property_current_audio() = track;
  // send_message(Player::STREAM_AUDIO_CHANGED);
}

// Return the current audio track.
gint GstPlayer::get_current_audio() {
  // se_dbg_msg(SE_DBG_VIDEO_PLAYER, "current_audio: %d", (m_pipeline) ? m_pipeline->property_current_audio() : 0);
  // FIXME
  // if (m_pipeline)
  //   return m_pipeline->property_current_audio();
  return -1;
}

// Return the framerate of the video or zero (0).
// Update numerator and denominator if the values are not null.
float GstPlayer::get_framerate(int *numerator, int *denominator) {
  // se_dbg(SE_DBG_VIDEO_PLAYER);

  // if (!m_pipeline)
  //   return 0;
  // Glib::RefPtr<Gst::Pad> pad = m_pipeline->get_video_pad(0);
  // g_return_val_if_fail(pad, 0);

  // Glib::RefPtr<Gst::Caps> caps = pad->get_current_caps();
  // g_return_val_if_fail(caps, 0);

  // const Gst::Structure structure = caps->get_structure(0);
  // if (structure.has_field("framerate") == false) {
  //   se_dbg_msg(SE_DBG_VIDEO_PLAYER, "structure has not field \"framerate\"");
  //   return 0;
  // }

  // Glib::ValueBase gst_value;
  // structure.get_field("framerate", gst_value);

  // Gst::Fraction fps(gst_value);
  // float framerate = static_cast<float>(fps.num) / static_cast<float>(fps.denom);

  // if (numerator != NULL)
  //   *numerator = fps.num;
  // if (denominator != NULL)
  //   *denominator = fps.denom;

  // se_dbg_msg(SE_DBG_VIDEO_PLAYER, "framerate: %f (num: %i, denom: %i)", framerate, fps.num, fps.denom);

  // return framerate;
  return 0;
}

guint GstPlayer::get_text_valignment_based_on_config() {
  guint alignment = 0;  // baseline by default

  if (!cfg::has_key("video-player", "text-valignment")) {
    cfg::set_string("video-player", "text-valignment", "baseline");
    return alignment;
  }
  auto valignment = cfg::get_string("video-player", "text-valignment");

  if (valignment == "baseline") {
    alignment = 0;
  } else if (valignment == "bottom") {
    alignment = 1;
  } else if (valignment == "top") {
    alignment = 2;
  } else if (valignment == "position") {
    alignment = 3;
  } else if (valignment == "center") {
    alignment = 4;
  }
  return alignment;
}

void GstPlayer::on_hierarchy_changed(Widget *previous_toplevel) {
  if (!previous_toplevel) {
    get_toplevel()->signal_configure_event().connect(sigc::mem_fun(*this, &GstPlayer::on_configure_event), false);
  }
}

bool GstPlayer::on_configure_event(GdkEventConfigure *) {
  set_render_rectangle();
  return false;
}

void GstPlayer::set_render_rectangle(bool is_mapped) {
#ifdef GDK_WINDOWING_WAYLAND
  if (!m_xoverlay) {
    return;
  }

  auto gdkWindow = get_window();

  if (GDK_IS_WAYLAND_WINDOW(gdkWindow->gobj())) {
    int x, y, width, height;

    if (is_mapped) {
      width = gdkWindow->get_width();
      height = gdkWindow->get_height();
    } else {
      width = height = 1;
    }

    translate_coordinates(*get_toplevel(), 0, 0, x, y);
    gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(m_xoverlay->gobj()), x, y, width, height);
    gst_video_overlay_expose(GST_VIDEO_OVERLAY(m_xoverlay->gobj()));
  }
#endif
}
