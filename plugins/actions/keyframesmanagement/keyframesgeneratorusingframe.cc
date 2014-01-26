/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2012, kitone
 *	2012, Martin Doucha
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
#include <keyframes.h>
#include <utility.h>
#include "mediadecoder.h"
#include <cfg.h>

/*
 */
class KeyframesGeneratorUsingFrame : public Gtk::Dialog, public MediaDecoder
{
public:

	/*
	 */
	KeyframesGeneratorUsingFrame(const Glib::ustring &uri, Glib::RefPtr<KeyFrames> &keyframes)
	:Gtk::Dialog(_("Generate Keyframes"), true), MediaDecoder(1000), m_prev_frame(NULL), m_prev_frame_size(0), m_difference(0.2)
	{
		set_border_width(12);
		set_default_size(300, -1);
		get_vbox()->pack_start(m_progressbar, false, false);
		add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		m_progressbar.set_text(_("Waiting..."));
		show_all();

		try
		{
			read_config();
			create_pipeline(uri);

			if(run() == Gtk::RESPONSE_OK)
			{
				keyframes = Glib::RefPtr<KeyFrames>(new KeyFrames);
				keyframes->insert(keyframes->end(), m_values.begin(), m_values.end());
				keyframes->set_video_uri(uri);
			}
		}
		catch(const std::runtime_error &ex)
		{
			std::cerr << ex.what() << std::endl;
		}
	}

	~KeyframesGeneratorUsingFrame(void) {
		delete[] m_prev_frame;
	}

	/*
	 */
	void read_config()
	{
		Config &cfg = Config::getInstance();
		if(cfg.has_key("KeyframesGeneratorUsingFrame", "difference") == false)
		{
			cfg.set_value_string(
				"KeyframesGeneratorUsingFrame", 
				"difference",
				"0.2",
				"difference between frames as percent");
		}
		else
			cfg.get_value_float("KeyframesGeneratorUsingFrame", "difference", m_difference);
	}

	/*
	 * Check buffer and try to catch keyframes.
	 */
	void on_video_identity_handoff(const Glib::RefPtr<Gst::Buffer>& buf, const Glib::RefPtr<Gst::Pad>&)
	{
		// ignore preroll
		//if(buf->flag_is_set(Gst::BUFFER_FLAG_PREROLL))
		if(GST_BUFFER_FLAG_IS_SET(buf->gobj(), GST_BUFFER_FLAG_LIVE))
			return;

		// first frame or change of buffer size, alloc & push
		if (!m_prev_frame || buf->get_size() != m_prev_frame_size)
		{
			delete[] m_prev_frame;
			m_prev_frame_size = buf->get_size();
			m_prev_frame = new guint8[m_prev_frame_size];
			long pos = buf->get_pts() / GST_MSECOND;
			m_values.push_back(pos);
		// continuation frame, compare
		}
		else
		{
			guint64 delta = 0, full = buf->get_size() / 3;
			unsigned long diff, i, j;
			long tmp;
			const guint8 *data = NULL; //buf->get_data();

			// calculate difference between frames
			for (i = 0; i < full; i++)
			{
				diff = 0;

				// get max difference in individual color channels
				for (j = 0; j < 3; j++)
				{
					tmp = (int)data[3 * i + j] - (int)m_prev_frame[3 * i + j];
					if (tmp < 0)
					{
						tmp = -tmp;
					}

					diff = tmp > diff ? tmp : diff;
				}

				// add max color diff to total delta
				delta += diff;
			}

			full *= 255;

			// >20% difference => scene cut
			if ((double)delta / (double)full > m_difference)
			{
				// FIXME: gstreamer 1.0
				//long pos = buf->get_timestamps() / GST_MSECOND;
				long pos = buf->get_pts() / GST_MSECOND;
				m_values.push_back(pos);
			}
		}

		//memcpy(m_prev_frame, buf->get_data(), buf->get_size());
	}

	/*
	 * Create video bin
	 */
	Glib::RefPtr<Gst::Element> create_element(const Glib::ustring &structure_name)
	{
		try
		{
			// We only need and want create the video sink
			if(structure_name.find("video") == Glib::ustring::npos)
				return Glib::RefPtr<Gst::Element>(NULL);

			Glib::RefPtr<Gst::FakeSink> fakesink = Gst::FakeSink::create("fakesink");
			fakesink->set_sync(false);
			fakesink->property_silent() = true;
			fakesink->property_signal_handoffs() = true;
			fakesink->signal_handoff().connect(
					sigc::mem_fun(*this, &KeyframesGeneratorUsingFrame::on_video_identity_handoff));

			// Set the new sink tp READY as well
			Gst::StateChangeReturn retst = fakesink->set_state(Gst::STATE_READY);
			if( retst == Gst::STATE_CHANGE_FAILURE )
				std::cerr << "Could not change state of new sink: " << retst << std::endl;

			return fakesink;
		}
		catch(std::runtime_error &ex)
		{
			std::cerr << "create_element runtime_error: " << ex.what() << std::endl;
		}
		return Glib::RefPtr<Gst::Element>(NULL);
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

	std::list<long> m_values;
	guint64 m_duration;
	guint64 m_prev_frame_size;
	guint8 *m_prev_frame;
	gfloat m_difference;
};

/*
 */
Glib::RefPtr<KeyFrames> generate_keyframes_from_file_using_frame(const Glib::ustring &uri)
{
	Glib::RefPtr<KeyFrames> kf;
	KeyframesGeneratorUsingFrame ui(uri, kf);
	return kf;

}
