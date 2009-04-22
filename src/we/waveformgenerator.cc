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

#include "waveformgenerator.h"
#include "utility.h"
#include "gstreamer_utility.h"
#include <gst/pbutils/missing-plugins.h>

#define TIMEOUT 100

/*
 *
 */
void debug_structure(GstStructure *structure)
{
	const gchar* structure_name = gst_structure_get_name(structure);

	se_debug_message(SE_DEBUG_WAVEFORM, "structure-name=%s", structure_name);

	gchar *human_structure_name = gst_structure_to_string(structure);

	se_debug_message(SE_DEBUG_WAVEFORM, "human-structure-name=%s", human_structure_name);

	g_free(human_structure_name);

	for(gint i=0; i<gst_structure_n_fields(structure); ++i)
	{
		se_debug_message(SE_DEBUG_WAVEFORM, "  structure-field-%d=%s", i, gst_structure_nth_field_name(structure, i));
	}
}

/*
 * Warp gstreamer to c++
 */
gboolean WaveformGenerator::static_bus_message(GstBus *bus, GstMessage *msg, gpointer data)
{
	return ((WaveformGenerator*)data)->on_bus_message(bus, msg);
}

/*
 * Warp gstreamer to c++
 */
void WaveformGenerator::static_newpad(GstElement *decodebin, GstPad *pad, gboolean last, gpointer data)
{
	((WaveformGenerator*)data)->on_newpad(decodebin, pad, last);
}


/*
 *
 */
WaveformGenerator::WaveformGenerator(const Glib::ustring &uri, Glib::RefPtr<Waveform> &wf)
:Gtk::Dialog(_("Generate Waveform"), true), m_pipeline(NULL), m_position(GST_CLOCK_TIME_NONE), m_duration(GST_CLOCK_TIME_NONE)
{
	set_border_width(12);
	set_default_size(300, -1);

	m_progressbar = manage(new Gtk::ProgressBar);
	get_vbox()->pack_start(*m_progressbar, false, false);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

	show_all();

	m_waveform = Glib::RefPtr<Waveform>(new Waveform);
	
	create_pipeline(uri);

	if(GST_IS_ELEMENT(m_pipeline))
	{
		gst_element_set_state(m_pipeline, GST_STATE_PLAYING);

		if(run() == Gtk::RESPONSE_OK)
		{
			wf = m_waveform;
		}
	}
}
	

/*
 * Destroy the pipeline
 */
WaveformGenerator::~WaveformGenerator()
{
	se_debug(SE_DEBUG_WAVEFORM);

	destroy_pipeline();
}

/*
 * Create the GStreamer pipeline.
 *
 * filesrc uri ! decodebin ! audioconvert ! level ! fakesink
 */
void WaveformGenerator::create_pipeline(const Glib::ustring &uri)
{
	se_debug_message(SE_DEBUG_WAVEFORM, "uri=%s", uri.c_str());

	if(m_pipeline)
		destroy_pipeline();

	// check gstreamer version
	{
		if(gstreamer_utility::check_registry("filesrc", 0, 10, 0)	== false)
			return;
		if(gstreamer_utility::check_registry("decodebin", 0, 10, 0)	== false)
			return;
		if(gstreamer_utility::check_registry("audioconvert", 0, 10, 0)	== false)
			return;
		if(gstreamer_utility::check_registry("level", 0, 10, 0)	== false)
			return;
	}

	GstElement *src = NULL, *dec = NULL;
	
	// Pipeline
	m_pipeline = gst_pipeline_new("pipeline");

	src = gst_element_factory_make("filesrc", "filesrc");

	dec = gst_element_factory_make("decodebin", "decoder");
	g_signal_connect(dec, "new-decoded-pad", G_CALLBACK(static_newpad), this);
	
	gst_bin_add_many(GST_BIN(m_pipeline), src, dec, NULL);
	gst_element_link(src, dec);

	// AudioBin
	create_audio_bin();

	// URI
	gst_uri_handler_set_uri(GST_URI_HANDLER(src), uri.c_str());

	// Bus
	GstBus *m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
	
	gst_bus_add_watch(GST_BUS(m_bus), static_bus_message, this);
	
	gst_object_unref(m_bus);

	gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_PAUSED);

	m_waveform->m_video_uri = uri;
}

/*
 * Destroy the GStreamer pipeline.
 */
void WaveformGenerator::destroy_pipeline()
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(GST_IS_ELEMENT(m_pipeline))
	{
		gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_NULL);
		gst_object_unref(GST_OBJECT(m_pipeline));
		m_pipeline = NULL;
	}
	m_position = m_duration = GST_CLOCK_TIME_NONE;
}

/*
 * Create a GStreamer audio container.
 *
 * audioconvert ! level ! fakesink.
 */
GstElement* WaveformGenerator::create_audio_bin()
{
	se_debug(SE_DEBUG_WAVEFORM);

	GstElement *audiobin = NULL;
	GstElement *conv = NULL;
	GstElement *level = NULL;
	GstElement *sink = NULL;
	GstPad* audiopad = NULL;

	// container
	audiobin = gst_bin_new("audiobin");

	// elements
	conv = gst_element_factory_make("audioconvert", "aconv");

	level = gst_element_factory_make("level", "level");
	//g_object_set(G_OBJECT(level), "interval", interval, NULL);
	
	sink = gst_element_factory_make("fakesink", "sink");
	g_object_set(G_OBJECT(sink), "silent", TRUE, NULL);

	// add & link
	gst_bin_add_many(GST_BIN(audiobin), conv, level, sink, NULL);

	gst_element_link_many(conv, level, sink, NULL);

	// create ghost pad
	audiopad = gst_element_get_pad(conv, "sink");
	
	gst_element_add_pad(audiobin,	gst_ghost_pad_new("sink", audiopad));

	gst_object_unref(audiopad);
	
	gst_bin_add(GST_BIN(m_pipeline), audiobin);

	return audiobin;
}

/*
 * Check if are missing plugins, if it's true display a message.
 * Return true if missing.
 */
bool WaveformGenerator::check_missing_plugins()
{
	if(!m_missing_plugins.empty())
	{
		gstreamer_utility::dialog_missing_plugins(m_missing_plugins);
		m_missing_plugins.clear();

		return true;
	}

	return false;
}

/*
 * Dispatch a GStreamer message.
 */
bool WaveformGenerator::on_bus_message(GstBus *bus, GstMessage *msg)
{
	se_debug_message(SE_DEBUG_WAVEFORM, "type='%s' name='%s'", 
			GST_MESSAGE_TYPE_NAME(msg), GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)));

	if(gst_is_missing_plugin_message(msg))
	{
		gchar *desc = gst_missing_plugin_message_get_description(msg);

		m_missing_plugins.push_back(desc);

		g_free(desc);
	}

	if(msg->type == GST_MESSAGE_ERROR)
	{
		on_bus_message_error(bus, msg);
	}
	else if(msg->type == GST_MESSAGE_WARNING)
	{
		on_bus_message_warning(bus, msg);
	}
	else if(msg->type == GST_MESSAGE_STATE_CHANGED)
	{
		on_bus_message_state_changed(bus, msg);
	}
	else if(msg->type == GST_MESSAGE_EOS)
	{
		on_bus_message_eos(bus, msg);
	}
	else if(msg->type == GST_MESSAGE_TAG)
	{
		on_bus_message_tag(bus, msg);
	}
	else if(msg->type == GST_MESSAGE_ELEMENT)
	{
		const GstStructure* structure = gst_message_get_structure(msg);
		const gchar* name = gst_structure_get_name(structure);

		if(Glib::ustring("level") == name)
		{
			on_bus_message_element_level(bus, msg);
		}
	}
	else if(msg->type == GST_MESSAGE_DURATION || msg->type == GST_MESSAGE_INFO)
	{
		m_position = get_position();
		m_duration = get_duration();

		update_progress_bar();
	}
	return true;
}

/*
 * A GStreamer internal error. 
 * Display a dialog message.
 */
bool WaveformGenerator::on_bus_message_error(GstBus *bus, GstMessage *msg)
{
	se_debug(SE_DEBUG_WAVEFORM);

	check_missing_plugins();

	GError *error = NULL;
	gchar* debug = NULL;

	gst_message_parse_error(msg, &error, &debug);

	GST_DEBUG("Error message: %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));
		
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "GST_MESSAGE_ERROR : %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));

	dialog_error(_("Media file could not be played.\n"), GST_STR_NULL(error->message));

	g_error_free(error);
	g_free(debug);

	response(Gtk::RESPONSE_CANCEL);

	return true;
}

/*
 * A GStreamer warning. Log the message.
 */
bool WaveformGenerator::on_bus_message_warning(GstBus *bus, GstMessage *msg)
{
	se_debug(SE_DEBUG_WAVEFORM);

	check_missing_plugins();

	GError *error = NULL;
	gchar* debug = NULL;

	gst_message_parse_warning(msg, &error, &debug);

	GST_DEBUG("Warning message: %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "GST_MESSAGE_WARNING : %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));

	g_warning ("%s [%s]", GST_STR_NULL (error->message), GST_STR_NULL (debug));

	g_error_free(error);
	g_free(debug);

	return true;
}

/*
 * The pipeline state has changed.
 * Update with only the pipeline message.
 */
bool WaveformGenerator::on_bus_message_state_changed(GstBus *bus, GstMessage *msg)
{
	if(GST_MESSAGE_TYPE(msg) != GST_MESSAGE_STATE_CHANGED)
		return false;

	// on ne traite que les changements du pipeline
	if(GST_MESSAGE_SRC(msg) != GST_OBJECT(m_pipeline))
		return false;

	GstState old_state, new_state;

	gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);

	se_debug_message(SE_DEBUG_WAVEFORM, "transition %s->%s",
			gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));

	GstStateChange transition = (GstStateChange)GST_STATE_TRANSITION(old_state, new_state);

	if(transition == GST_STATE_CHANGE_NULL_TO_READY)
	{
	}
	else if(transition == GST_STATE_CHANGE_READY_TO_PAUSED)
	{
		if(!GST_CLOCK_TIME_IS_VALID(m_duration))
		{
			m_position = get_position();
			m_duration = get_duration();
		}
		update_progress_bar();
	}
	else if(transition == GST_STATE_CHANGE_PAUSED_TO_PLAYING)
	{
		m_connection_timeout = Glib::signal_timeout().connect(
				sigc::mem_fun(*this, &WaveformGenerator::update_progress_bar), TIMEOUT);
	}
	else if(transition == GST_STATE_CHANGE_PLAYING_TO_PAUSED)
	{
		m_connection_timeout.disconnect();
	}
	else if(transition == GST_STATE_CHANGE_PAUSED_TO_READY)
	{
	}
	else if(transition == GST_STATE_CHANGE_READY_TO_NULL)
	{
	}

	return true;
}

/*
 * End of the stream. Send RESPONSE_OK.
 */
bool WaveformGenerator::on_bus_message_eos(GstBus *bus, GstMessage *msg)
{
	se_debug(SE_DEBUG_WAVEFORM);

	gst_element_set_state(m_pipeline, GST_STATE_PAUSED);

	m_progressbar->set_fraction(1);

	m_waveform->m_duration = get_duration() / GST_MSECOND;

	response(Gtk::RESPONSE_OK);
	
	return true;
}

/*
 *
 */
bool WaveformGenerator::on_bus_message_tag(GstBus *bus, GstMessage *msg)
{
	se_debug(SE_DEBUG_WAVEFORM);

	GstTagList *tag_list = NULL;

	gst_message_parse_tag(msg, &tag_list);

	debug_structure(tag_list);
	
	gst_tag_list_free (tag_list);
	
	return true;
}

/*
 * Generate the waveform with the level message.
 */
bool WaveformGenerator::on_bus_message_element_level(GstBus *bus, GstMessage *msg)
{
	se_debug(SE_DEBUG_WAVEFORM);

	const GstStructure* structure = gst_message_get_structure(msg);
	//const gchar* name = gst_structure_get_name(structure);

	gst_structure_get_clock_time (structure, "endtime", &m_position);

	const GValue* list = NULL;
	const GValue* value = NULL;

	list = gst_structure_get_value (structure, "rms");
				
	m_waveform->m_n_channels = gst_value_list_get_size (list);

	if(m_waveform->m_n_channels > 2)
		m_waveform->m_n_channels = 2;

	for(unsigned int i=0; i< m_waveform->m_n_channels; ++i)
	{
		gdouble rms_dB, peak_dB, decay_dB;
		gdouble rms;

		list = gst_structure_get_value (structure, "rms");
		value = gst_value_list_get_value (list, i);
		rms_dB = g_value_get_double (value);

		list = gst_structure_get_value (structure, "peak");
		value = gst_value_list_get_value (list, i);
		peak_dB = g_value_get_double (value);

		list = gst_structure_get_value (structure, "decay");
		value = gst_value_list_get_value (list, i);
		decay_dB = g_value_get_double (value);

		rms = pow (10, rms_dB / 20);

		m_waveform->m_channels[i].push_back(rms);
	}

	return true;
}

/*
 * Only used the audio pad. Connect the the AudioBin.
 */
void WaveformGenerator::on_newpad(GstElement *decodebin, GstPad *pad, bool last)
{
	se_debug(SE_DEBUG_WAVEFORM);

	GstCaps *caps = NULL;
	GstStructure *structure = NULL;
	GstPad *audiopad = NULL;

	if(se_debug_check_flags(SE_DEBUG_WAVEFORM))
	{
		caps = gst_pad_get_caps(pad);
		structure = gst_caps_get_structure(caps, 0);
	
		const gchar* pad_name = gst_pad_get_name(pad);
		const gchar* structure_name = gst_structure_get_name(structure);
	
		se_debug_message(SE_DEBUG_WAVEFORM, "pad-name=%s structure-name=%s", pad_name, structure_name);

		debug_structure(structure);
	}

	GstElement* audiobin = gst_bin_get_by_name(GST_BIN(m_pipeline), "audiobin");
	if(audiobin == NULL)
	{
		audiobin = create_audio_bin();
	}

	// only link once
	audiopad = gst_element_get_pad(audiobin, "sink");
	if(GST_PAD_IS_LINKED(audiopad))
	{
		g_object_unref(audiopad);
		return;
	}

	// check media
	caps = gst_pad_get_caps(pad);
	structure = gst_caps_get_structure(caps, 0);

	if(!g_strrstr(gst_structure_get_name(structure), "audio"))
	{
		se_debug_message(SE_DEBUG_WAVEFORM, "this is not <audio>, unref caps");
		
		gst_caps_unref(caps);
		gst_object_unref(audiopad);
		return;
	}

	gst_caps_unref(caps);

	se_debug_message(SE_DEBUG_WAVEFORM, "link the pad with audiobin");
	// link'n'play
	gst_pad_link(pad, audiopad);
}

/*
 * Return the current position in the pipeline.
 */
gint64 WaveformGenerator::get_position()
{
	if(GST_IS_ELEMENT(m_pipeline))
	{
		gint64 pos;
		GstFormat fmt = GST_FORMAT_TIME;
		if(gst_element_query_position(GST_ELEMENT(m_pipeline), &fmt, &pos))
		{
			se_debug_message(SE_DEBUG_WAVEFORM, "%s", gstreamer_utility::time_to_string(pos).c_str());
			return pos;
		}
	}
	se_debug_message(SE_DEBUG_WAVEFORM, "%s", gstreamer_utility::time_to_string(GST_CLOCK_TIME_NONE).c_str());
	return GST_CLOCK_TIME_NONE;
}

/*
 * Return the duration of the pipeline.
 */
gint64 WaveformGenerator::get_duration()
{
	if(GST_IS_ELEMENT(m_pipeline))
	{
		gint64 dur;
		GstFormat fmt = GST_FORMAT_TIME;
		if(gst_element_query_duration(GST_ELEMENT(m_pipeline), &fmt, &dur))
		{
			se_debug_message(SE_DEBUG_WAVEFORM, "%s", gstreamer_utility::time_to_string(dur).c_str());
			return dur;
		}
	}
	se_debug_message(SE_DEBUG_WAVEFORM, "%s", gstreamer_utility::time_to_string(GST_CLOCK_TIME_NONE).c_str());
	return GST_CLOCK_TIME_NONE;
}

/*
 * Update the progress bar with the position in the pipeline.
 */
bool WaveformGenerator::update_progress_bar()
{
	se_debug(SE_DEBUG_WAVEFORM);

	if(m_pipeline)
	{
		if(GST_CLOCK_TIME_IS_VALID(m_position) && GST_CLOCK_TIME_IS_VALID(m_duration))
		{
			double percent = (double)m_position / (double)m_duration;

			percent = CLAMP(percent, 0.0, 1.0);
	
			m_progressbar->set_fraction(percent);
			m_progressbar->set_text(gstreamer_utility::time_to_string(m_position) + " / " + gstreamer_utility::time_to_string(m_duration));
		}
	}

	return true;
}

/*
 * Open the dialog and try to generate the waveform.
 * Return the new waveform.
 */
Glib::RefPtr<Waveform> WaveformGenerator::create(const Glib::ustring &uri)
{
	se_debug_message(SE_DEBUG_WAVEFORM, uri.c_str());
	
	Glib::RefPtr<Waveform> wf;

	WaveformGenerator wg(uri, wf);

	return wf;
}

