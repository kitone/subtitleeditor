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
	:Gtk::Dialog(_("Generate Waveform"), true), MediaDecoder(1000), m_duration(GST_CLOCK_TIME_NONE)
	{
		set_border_width(12);
		set_default_size(300, -1);
		get_vbox()->pack_start(m_progressbar, false, false);
		add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		show_all();

		try
		{
			create_pipeline(uri);

			if(run() == Gtk::RESPONSE_OK)
			{
				wf = Glib::RefPtr<Waveform>(new Waveform);
				wf->m_duration = m_duration / GST_MSECOND;
				wf->m_n_channels = 1;
				wf->m_channels[0] = std::vector<double>(m_values.begin(), m_values.end());
				wf->m_video_uri = uri;
				std::cout << "waveform pushed: duration:" << time_to_string(m_duration) << std::endl;
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
		try
		{
			// We only need and want create the video sink
			if(structure_name.find("audio") == Glib::ustring::npos)
				return Glib::RefPtr<Gst::Element>(NULL);

			Glib::RefPtr<Gst::Bin> audiobin = Glib::RefPtr<Gst::Bin>::cast_dynamic(
					Gst::Parse::create_bin(
						"audioconvert ! "
						"audioresample ! "
						"audio/x-raw-float, channels=1 ! "
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
		else
			m_progressbar.set_text(_("Waiting..."));
		return true;
	}

	/*
	 */
	bool on_bus_message_element_level(Glib::RefPtr<Gst::Message> msg)
	{
		Gst::Structure structure = msg->get_structure();
		//for(unsigned int i=0; i < 1; ++i) // channels
			//structure.get_field("rms", rms_dB);

		gdouble rms_dB = 0.0;
		const GValue* list = gst_structure_get_value(structure.gobj(), "rms");
		gint size = gst_value_list_get_size(list);
		for(int i=0; i<size; ++i)
		{
			const GValue* value = gst_value_list_get_value(list, i);
			rms_dB += g_value_get_double(value);
		}
		rms_dB = rms_dB / size;

		double rms = pow(10, rms_dB / 20);
		m_values.push_back(rms);
		return true;
	}

	/*
	 */
	void on_work_finished()
	{
		response(Gtk::RESPONSE_OK);
	}

	/*
	 */
	void on_work_cancel()
	{
		response(Gtk::RESPONSE_CANCEL);
	}

protected:
	Gtk::ProgressBar m_progressbar;
	guint64 m_duration;
	std::list<gdouble> m_values;
};

Glib::RefPtr<Waveform> generate_waveform_from_file(const Glib::ustring &uri)
{
	Glib::RefPtr<Waveform> wf;
	WaveformGenerator ui(uri, wf);
	return wf;
}
