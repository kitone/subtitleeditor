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
#include <keyframes.h>
#include <utility.h>
//#include <gstreamer_utility.h>
#include "mediadecoder.h"

/*
 */
class KeyframesGenerator : public Gtk::Dialog, public MediaDecoder
{
public:

	/*
	 */
	KeyframesGenerator(const Glib::ustring &uri, Glib::RefPtr<KeyFrames> &keyframes)
	:Gtk::Dialog(_("Generate Keyframes"), true), MediaDecoder(1000)
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
				keyframes = Glib::RefPtr<KeyFrames>(new KeyFrames);
				keyframes->insert(keyframes->end(), m_values.begin(), m_values.end());
			}
		}
		catch(const std::runtime_error &ex)
		{
			std::cerr << ex.what() << std::endl;
		}
	}

	/*
	 * Check buffer and try to catch keyframes.
	 */
	void on_video_identity_handoff(const Glib::RefPtr<Gst::Buffer>& buf, const Glib::RefPtr<Gst::Pad>& pad)
	{
		// FIXME: http://bugzilla.gnome.org/show_bug.cgi?id=590923
		if(!buf->flag_is_set(GST_BUFFER_FLAG_DELTA_UNIT))//Gst::BUFFER_FLAG_DELTA_UNIT))
		{
			long pos = buf->get_timestamp() / GST_MSECOND;
			m_values.push_back(pos);
		}
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

			Glib::RefPtr<Gst::Bin> videobin = Glib::RefPtr<Gst::Bin>::cast_dynamic(
					Gst::Parse::create_bin(
						"ffmpegcolorspace ! fakesink name=vsink", true));

			Glib::RefPtr<Gst::FakeSink> vsink = Glib::RefPtr<Gst::FakeSink>::cast_dynamic(
					videobin->get_element("vsink"));

			vsink->set_sync(false);
			vsink->property_silent() = true;
			vsink->property_signal_handoffs() = true;
			vsink->signal_handoff().connect(
					sigc::mem_fun(*this, &KeyframesGenerator::on_video_identity_handoff));

			// Set the new sink tp READY as well
			Gst::StateChangeReturn retst = videobin->set_state(Gst::STATE_READY);
			if( retst == Gst::STATE_CHANGE_FAILURE )
				std::cerr << "Could not change state of new sink: " << retst << std::endl;

			return Glib::RefPtr<Gst::Element>::cast_dynamic(videobin);
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
};

/*
 */
Glib::RefPtr<KeyFrames> generate_keyframes_from_file(const Glib::ustring &uri)
{
	Glib::RefPtr<KeyFrames> kf;
	KeyframesGenerator ui(uri, kf);
	return kf;

}
