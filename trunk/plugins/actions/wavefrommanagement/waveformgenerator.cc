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

#include <gstreamermm.h>
#include <gtkmm.h>
#include <iostream>
#include <iomanip>
#include <waveform.h>
#include <utility.h>
#include "mediadecoder.h"

/*
 */
class WaveformGenerator : public Gtk::Dialog, public MediaDecoder
{
public:

	/*
	 */
	WaveformGenerator(const Glib::ustring &uri, Glib::RefPtr<Waveform> &wf)
	:Gtk::Dialog(_("Generate Waveform"), true), MediaDecoder(1000), m_duration(GST_CLOCK_TIME_NONE), m_n_channels(0)
	{
		se_debug_message(SE_DEBUG_PLUGINS, "uri=%s", uri.c_str());

		set_border_width(12);
		set_default_size(300, -1);
		get_vbox()->pack_start(m_progressbar, false, false);
		add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		m_progressbar.set_text(_("Waiting..."));
		show_all();

		try
		{
			create_pipeline(uri);

			if(run() == Gtk::RESPONSE_OK)
			{
				wf = Glib::RefPtr<Waveform>(new Waveform);
				wf->m_duration = m_duration / GST_MSECOND;
				wf->m_n_channels = m_n_channels;
				for(guint i=0; i< m_n_channels; ++i)
					wf->m_channels[i] = std::vector<double>(m_values[i].begin(), m_values[i].end());
				wf->m_video_uri = uri;
			}
		}
		catch(const std::runtime_error &ex)
		{
			std::cerr << ex.what() << std::endl;
		}
	}

	/*
	 * Create audio bin
	 */
	Glib::RefPtr<Gst::Element> create_element(const Glib::ustring &structure_name)
	{
		se_debug_message(SE_DEBUG_PLUGINS, "structure_name=%s", structure_name.c_str());
		try
		{
			// We only need and want create the video sink
			if(structure_name.find("audio") == Glib::ustring::npos)
				return Glib::RefPtr<Gst::Element>(NULL);

			Glib::RefPtr<Gst::Bin> audiobin = Glib::RefPtr<Gst::Bin>::cast_dynamic(
					Gst::Parse::create_bin(
						"audioconvert ! "
						//"audioresample ! "
						//"audio/x-raw-float, channels=1 ! "
						"level name=level ! "
						"fakesink name=asink"
						, true));
			// Set the new sink tp READY as well
			Gst::StateChangeReturn retst = audiobin->set_state(Gst::STATE_READY);
			if( retst == Gst::STATE_CHANGE_FAILURE )
				std::cerr << "Could not change state of new sink: " << retst << std::endl;

			return Glib::RefPtr<Gst::Element>::cast_dynamic(audiobin);
		}
		catch(std::runtime_error &ex)
		{
			se_debug_message(SE_DEBUG_PLUGINS, "runtime_error=%s", ex.what());
			std::cerr << "create_audio_bin: " << ex.what() << std::endl;
		}
		return Glib::RefPtr<Gst::Element>(NULL);
	}

	/*
	 * BUS MESSAGE
	 */
	bool on_bus_message(const Glib::RefPtr<Gst::Bus> &bus, const Glib::RefPtr<Gst::Message> &msg)
	{
		MediaDecoder::on_bus_message(bus, msg);

		if(msg->get_message_type() ==Gst::MESSAGE_ELEMENT)
		{
			if(msg->get_structure().get_name() == "level")
				return on_bus_message_element_level( msg );
		}
		return true;
	}

	/*
	 * Update the progress bar
	 */
	bool on_timeout()
	{
		se_debug(SE_DEBUG_PLUGINS);

		if(!m_pipeline)
			return false;

		Gst::Format fmt = Gst::FORMAT_TIME;
		gint64 pos = 0, len = 0;
		if(m_pipeline->query_position(fmt, pos) && m_pipeline->query_duration(fmt, len))
		{
			double percent = (double)pos / (double)len;

			percent = CLAMP(percent, 0.0, 1.0);
	
			m_progressbar.set_fraction(percent);
			m_progressbar.set_text(time_to_string(pos) + " / " + time_to_string(len));
			m_duration = len;

			return pos != len;
		}

		return true;
	}

	/*
	 */
	bool on_bus_message_element_level(Glib::RefPtr<Gst::Message> msg)
	{
		se_debug_message(SE_DEBUG_PLUGINS, 
				"type='%s' name='%s'", 
				GST_MESSAGE_TYPE_NAME(msg->gobj()), GST_OBJECT_NAME(GST_MESSAGE_SRC(msg->gobj())));
		
		Gst::Structure structure = msg->get_structure();

		const GValue* list = gst_structure_get_value(structure.gobj(), "rms");
		gint num_channels = gst_value_list_get_size(list);

		guint first_channel, last_channel;
		if(num_channels >= 6)
		{
			first_channel = 1;
			last_channel = 3;
		}
		else if(num_channels == 5)
		{
			first_channel = 1;
			last_channel = 2;
		}
		else if(num_channels == 2)
		{
			first_channel = 0;
			last_channel = 1;
		}
		else
		{
			first_channel = last_channel = 0;
		}
		// build the number of channels
		m_n_channels = last_channel - first_channel + 1;

		// get peak from channels
		for(guint c= first_channel, i=0; c <= last_channel; ++c, ++i)
		{
			double peak = pow(10, g_value_get_double(gst_value_list_get_value(list, c)) / 20);
			m_values[i].push_back(peak);
		}
		return true;
	}

	/*
	 */
	void on_work_finished()
	{
		se_debug(SE_DEBUG_PLUGINS);

		response(Gtk::RESPONSE_OK);
	}

	/*
	 */
	void on_work_cancel()
	{
		se_debug(SE_DEBUG_PLUGINS);

		response(Gtk::RESPONSE_CANCEL);
	}

protected:
	Gtk::ProgressBar m_progressbar;
	guint64 m_duration;
	guint m_n_channels;
	std::list<gdouble> m_values[3];
};

Glib::RefPtr<Waveform> generate_waveform_from_file(const Glib::ustring &uri)
{
	Glib::RefPtr<Waveform> wf;
	WaveformGenerator ui(uri, wf);
	return wf;
}
