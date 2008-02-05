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
 

#include "WaveformGeneratorUI.h"
#include "debug.h"
#include "utility.h"
#include "Config.h"

// old method 8.05
// new method ?

/*
 *
 */
gboolean WaveformGeneratorUI::__static_bus_message(GstBus *bus, GstMessage *message, gpointer data)
{
	return ((WaveformGeneratorUI*)data)->bus_message(bus, message);
}

void WaveformGeneratorUI::__static_newpad(GstElement *decodebin, GstPad *pad, gboolean last, gpointer data)
{
	((WaveformGeneratorUI*)data)->newpad(decodebin, pad, last);
}

/*
 *
 */
WaveformGeneratorUI::WaveformGeneratorUI(Glib::RefPtr<Waveform> &waveform, const Glib::ustring &uri)
:Gtk::Dialog(_("Generate Waveform"), true)
{
	m_pipeline = NULL;
	m_bus = NULL;
	m_duration = 0;

	waveform.clear();

	set_border_width(6);
	set_default_size(300, -1);
	get_vbox()->pack_start(m_progressbar, false, false);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);//->signal_clicked().connect(sigc::mem_fun(*this, &WaveformGeneratorUI::on_cancel));

	show_all();
/*
	guint interval = 10000000;

	Glib::ustring pipe = build_message(
			"filesrc name=filesrc ! decodebin name=decode !"
			"queue max-size-buffers=500 max-size-bytes=0 max-size-time=0 !"
			"audioconvert ! level interval=%i message=true ! fakesink",
			interval);

	GError *error = NULL;

	m_pipeline = gst_parse_launch(pipe.c_str(), &error);

	if(error || ! m_pipeline)
	{
		g_error_free(error);
		return;
	}
*/
	//
	//
	//
	int interval = 0;
	
	Config::getInstance().get_value_int("waveform", "interval", interval);

	//interval = 0.01 * GST_SECOND;

	GstPad *audiopad = NULL;
	
	m_pipeline = gst_pipeline_new("pipeline");

	m_src = gst_element_factory_make("filesrc", "filesrc");
	
	m_dec = gst_element_factory_make("decodebin", "decoder");
	g_signal_connect(m_dec, "new-decoded-pad", G_CALLBACK(__static_newpad), this);
	gst_bin_add_many(GST_BIN(m_pipeline), m_src, m_dec, NULL);
	gst_element_link(m_src, m_dec);

	// create audio output
	//
	/*
	Glib::ustring description = build_message("audioconvert name=aconv ! audio/x-raw-int,channels=2,width=16,depth=32 ! level ! fakesink silent=TRUE");

	GError *error = NULL;
	m_audio = gst_parse_bin_from_description(description.c_str(), true, &error);
	if(error)
	{
		std::cerr << GST_STR_NULL(error->message) << std::endl;
		g_error_free(error);
	}
	gst_bin_add(GST_BIN(m_pipeline), m_audio);
	*/
	//
	
	m_audio = gst_bin_new("audiobin");
	
	m_conv = gst_element_factory_make("audioconvert", "aconv");

	m_lev = gst_element_factory_make("level", "level");
	g_object_set(G_OBJECT(m_lev), "interval", interval, NULL);
			
	m_sink = gst_element_factory_make("fakesink", "sink");
	g_object_set(G_OBJECT(m_sink), "silent", TRUE, NULL);

	audiopad = gst_element_get_pad(m_conv, "sink");
	// add & link
	gst_bin_add_many(GST_BIN(m_audio), m_conv, m_lev, m_sink, NULL);
	
	gst_element_link_many(m_conv, m_lev, m_sink, NULL);

	gst_element_add_pad(m_audio,
			gst_ghost_pad_new("sink", audiopad));
	gst_object_unref(audiopad);

	gst_bin_add(GST_BIN(m_pipeline), m_audio);
	//
	//
	//
	GstElement *filesrc = gst_bin_get_by_name(GST_BIN(m_pipeline), "filesrc");
	gst_uri_handler_set_uri(GST_URI_HANDLER(filesrc), uri.c_str());

	// create bus...
	m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
	gst_bus_add_watch(GST_BUS(m_bus), __static_bus_message, this);
	gst_object_unref(m_bus);
	
	m_waveform = Glib::RefPtr<Waveform>(new Waveform);

	std::clock_t start = std::clock();
	//
	gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_PLAYING);

	sigc::connection m_connection = Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &WaveformGeneratorUI::on_timeout), 1000);

	if( run() == Gtk::RESPONSE_OK)
	{
		waveform = m_waveform;
		waveform->m_video_uri = uri;
	}
	else
	{
		m_waveform.clear();
	}

	m_connection.disconnect();
	std::cout << "WaveformGenerator: " << double(std::clock() - start) / CLOCKS_PER_SEC << std::endl;
}

/*
 *
 */
WaveformGeneratorUI::~WaveformGeneratorUI()
{
	gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(m_pipeline));
}

/*
 *
 */
void WaveformGeneratorUI::newpad(GstElement *decodebin, GstPad *pad, gboolean last)
{
	GstCaps *caps = NULL;
	GstStructure *structure = NULL;
	GstPad *audiopad = NULL;

	// only link once
	audiopad = gst_element_get_pad(m_audio, "sink");
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
		gst_caps_unref(caps);
		gst_object_unref(audiopad);
		return;
	}

	gst_caps_unref(caps);

	// link'n'play
	gst_pad_link(pad, audiopad);
}

/*
 *
 */
bool WaveformGeneratorUI::bus_message(GstBus *bus, GstMessage *msg)
{
	if(msg->type == GST_MESSAGE_ERROR)
	{
		GError *error = NULL;
		gchar* debug = NULL;

		gst_message_parse_error(msg, &error, &debug);

		GST_DEBUG("Error message: %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));
		
		se_debug_message(SE_DEBUG_APP, "GST_MESSAGE_ERROR : %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));

		dialog_error(_("Media file could not be played."), GST_STR_NULL(error->message));

		// signal error emit...
		if(m_pipeline)
			gst_element_set_state(GST_ELEMENT(m_pipeline), GST_STATE_NULL);

		g_error_free(error);
		g_free(debug);

		response(Gtk::RESPONSE_CANCEL);
	}
	else if(msg->type == GST_MESSAGE_WARNING)
	{
		GError *error = NULL;
		gchar* debug = NULL;

		gst_message_parse_warning(msg, &error, &debug);

		GST_DEBUG("Warning message: %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));

		se_debug_message(SE_DEBUG_APP, "GST_MESSAGE_WARNING : %s [%s]", GST_STR_NULL(error->message), GST_STR_NULL(debug));

		g_warning ("%s [%s]", GST_STR_NULL (error->message), GST_STR_NULL (debug));

		g_error_free(error);
		g_free(debug);

		//response(Gtk::RESPONSE_CANCEL);
	}
	else if(msg->type == GST_MESSAGE_EOS)
	{
		response(Gtk::RESPONSE_OK);
	}
	else if(msg->type == GST_MESSAGE_ELEMENT)
	{
		const GstStructure* structure = gst_message_get_structure(msg);
		const gchar* name = gst_structure_get_name(structure);

		if(strcmp(name, "level") == 0)
		{
			gst_structure_get_clock_time (structure, "endtime", &endtime);

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

				// TODO fixme!
				m_waveform->m_channels[i].push_back(rms);
			}

			// TODO fixme!
			if(m_waveform->m_duration == 0)
			{
				GstFormat fmt = GST_FORMAT_TIME;
	
				if(gst_element_query_duration(GST_ELEMENT(m_pipeline), &fmt, &m_duration))
					m_waveform->m_duration = m_duration;
			}
		}
	}
	return true;
}

/*
 *
 */
bool WaveformGeneratorUI::on_timeout()
{
	if(m_pipeline && m_duration != 0)
	{
		gint64 pos = endtime;
		double percent = ((double)pos / (double)m_duration);
		utility::clamp(percent, 0.0, 1.0);

		m_progressbar.set_fraction(percent);
		m_progressbar.set_text((Gst::time_to_string(pos) + " / " + Gst::time_to_string(m_duration)));
	}
	return true;
}
