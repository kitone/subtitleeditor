#ifndef _mediadecoder_h
#define _mediadecoder_h

/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2014, kitone
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
// missing plugins
#include "gstreamer_utility.h"
#include <gst/pbutils/missing-plugins.h>
// std
#include <iostream>
#include <iomanip>

/*
 * Class to help with gstreamer(mm)
 */
class MediaDecoder : virtual public sigc::trackable
{
public:

	/*
	 */
	MediaDecoder(guint timeout = 0)
	:m_watch_id(0), m_timeout(timeout)
	{
	}

	/*
	 */
	virtual ~MediaDecoder()
	{
		destroy_pipeline();
	}

	/*
	 */
	void create_pipeline(const Glib::ustring &uri)
	{
		se_debug_message(SE_DEBUG_PLUGINS, "uri=%s", uri.c_str());

		if(m_pipeline)
			destroy_pipeline();

		m_pipeline = Gst::Pipeline::create("pipeline");

		Glib::RefPtr<Gst::FileSrc> filesrc = Gst::FileSrc::create("filesrc");

		Glib::RefPtr<Gst::DecodeBin> decodebin = Gst::DecodeBin::create("decoder");

		decodebin->signal_pad_added().connect(
				sigc::mem_fun(*this, &MediaDecoder::on_pad_added));

		try
		{
			m_pipeline->add(filesrc);
			m_pipeline->add(decodebin);
	
			filesrc->link(decodebin);
		}
		catch(const std::runtime_error &ex)
		{
			std::cerr << ex.what() << std::endl;
			// FIXME destroy pipeline ?
		}
		filesrc->set_uri(uri);

		// Bus watching
		Glib::RefPtr<Gst::Bus> bus = m_pipeline->get_bus();
		m_watch_id = bus->add_watch(
			sigc::mem_fun(*this, &MediaDecoder::on_bus_message));

		//m_pipeline->set_state(Gst::STATE_PAUSED);
		if( m_pipeline->set_state(Gst::STATE_PLAYING) == Gst::STATE_CHANGE_FAILURE )
		{
			se_debug_message(SE_DEBUG_PLUGINS, 
					"Failed to change the state of the pipeline to PLAYING");
		}

	}

	/*
	 */
	void destroy_pipeline()
	{
		se_debug(SE_DEBUG_PLUGINS);

		if(m_connection_timeout)
			m_connection_timeout.disconnect();

		if(m_pipeline)
		{
			m_pipeline->get_bus()->remove_watch(m_watch_id);
			m_pipeline->set_state(Gst::STATE_NULL);
		}

		m_watch_id = 0;
		m_pipeline = Glib::RefPtr<Gst::Pipeline>();
	}

	/*
	 */
	virtual void on_pad_added(const Glib::RefPtr<Gst::Pad> &newpad)
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Gst::Caps> caps_null;
		Glib::RefPtr<Gst::Caps> caps = newpad->query_caps(caps_null);
		se_debug_message(SE_DEBUG_PLUGINS, "newpad->caps: %s", caps->to_string().c_str());

		const Gst::Structure structure = caps->get_structure(0);
		if(!structure)
			return;

		Glib::RefPtr<Gst::Element> sink = create_element(structure.get_name());;
		if(sink)
		{
			// Add bin to the pipeline
			m_pipeline->add(sink);
			
			// Set the new sink tp PAUSED as well
			Gst::StateChangeReturn retst = sink->set_state(Gst::STATE_PAUSED);
			if( retst == Gst::STATE_CHANGE_FAILURE )
			{
				std::cerr << "Could not change state of new sink: " << retst << std::endl;
				se_debug_message(SE_DEBUG_PLUGINS, "Could not change the state of new sink");
				m_pipeline->remove(sink);
				return;
			}
			// Get the ghostpad of the sink bin
			Glib::RefPtr<Gst::Pad> sinkpad = sink->get_static_pad("sink");

			Gst::PadLinkReturn ret = newpad->link(sinkpad);

			if(ret != Gst::PAD_LINK_OK && ret != Gst::PAD_LINK_WAS_LINKED)
			{
				std::cerr << "Linking of pads " << newpad->get_name() << " and " << sinkpad->get_name() << " failed." << std::endl;
				se_debug_message(SE_DEBUG_PLUGINS, "Linking of pads failed");
			}
			else
			{
				se_debug_message(SE_DEBUG_PLUGINS, "Pads linking with success");
			}
		}
		else
		{
			se_debug_message(SE_DEBUG_PLUGINS, "create_element return an NULL sink");
		}
	}

	/*
	 * BUS MESSAGE
	 */
	virtual bool on_bus_message(const Glib::RefPtr<Gst::Bus> &/*bus*/, const Glib::RefPtr<Gst::Message> &msg)
	{
		se_debug_message(SE_DEBUG_PLUGINS, 
				"type='%s' name='%s'", 
				GST_MESSAGE_TYPE_NAME(msg->gobj()), GST_OBJECT_NAME(GST_MESSAGE_SRC(msg->gobj())));

		switch(msg->get_message_type())
		{
		case Gst::MESSAGE_ELEMENT: 
			return on_bus_message_element( Glib::RefPtr<Gst::MessageElement>::cast_static(msg) );
		case Gst::MESSAGE_EOS: 
			return on_bus_message_eos( Glib::RefPtr<Gst::MessageEos>::cast_static(msg) );
		case Gst::MESSAGE_ERROR:
			return on_bus_message_error( Glib::RefPtr<Gst::MessageError>::cast_static(msg) );
		case Gst::MESSAGE_WARNING:
			return on_bus_message_warning( Glib::RefPtr<Gst::MessageWarning>::cast_static(msg) );
		case Gst::MESSAGE_STATE_CHANGED:
			return on_bus_message_state_changed( Glib::RefPtr<Gst::MessageStateChanged>::cast_static(msg) );
		default:
				break;
		}
		return true;
	}

	/*
	 */
	virtual bool on_bus_message_error(Glib::RefPtr<Gst::MessageError> msg)
	{
		check_missing_plugins();
		
		Glib::ustring error = (msg) ? msg->parse().what() : Glib::ustring();
		
		dialog_error(_("Media file could not be played.\n"), error);
		// Critical error, cancel the work.
		on_work_cancel();
		return true;
	}

	/*
	 */
	virtual bool on_bus_message_warning(Glib::RefPtr<Gst::MessageWarning> msg)
	{
		check_missing_plugins();
		
		Glib::ustring error = (msg) ? msg->parse().what() : Glib::ustring();
		dialog_error(_("Media file could not be played.\n"), error);

		return true;
	}

	/*
	 */
	virtual bool on_bus_message_state_changed(Glib::RefPtr<Gst::MessageStateChanged> msg)
	{
		if(m_timeout > 0)
			return on_bus_message_state_changed_timeout(msg);
		return true;
	}

	/*
	 */
	virtual bool on_bus_message_eos(Glib::RefPtr<Gst::MessageEos> )
	{
		m_pipeline->set_state(Gst::STATE_PAUSED);
		on_work_finished();
		return true;
	}

	/*
	 */
	virtual bool on_bus_message_element(Glib::RefPtr<Gst::MessageElement> msg)
	{
		check_missing_plugin_message(msg);
		return true;
	}


	/*
	 */
	virtual void on_work_finished()
	{
		// FIXME
	}

	/*
	 */
	virtual void on_work_cancel()
	{
		// FIXME
	}

	/*
	 */
	virtual Glib::RefPtr<Gst::Element> create_element(const Glib::ustring &)
	{
		return Glib::RefPtr<Gst::Element>();
	}

	/*
	 */
	virtual bool on_timeout()
	{
		return false;
	}

	/*
	 * utility
	 */
	Glib::ustring time_to_string(gint64 pos)
	{
		return Glib::ustring::compose("%1:%2:%3",
				Glib::ustring::format(std::setfill(L'0'), std::setw(2), Gst::get_hours(pos)),
				Glib::ustring::format(std::setfill(L'0'), std::setw(2), Gst::get_minutes(pos)),
				Glib::ustring::format(std::setfill(L'0'), std::setw(2), Gst::get_seconds(pos)));
	}

protected:

	/*
	 */
	bool on_bus_message_state_changed_timeout(Glib::RefPtr<Gst::MessageStateChanged> msg)
	{
		se_debug(SE_DEBUG_PLUGINS);

		// We only update when it is the pipeline object
		if(msg->get_source()->get_name() != "pipeline")
			return true;

		Gst::State old_state, new_state, pending;

		msg->parse(old_state, new_state, pending);

		if(old_state == Gst::STATE_PAUSED && new_state == Gst::STATE_PLAYING)
		{
			if(!m_connection_timeout)
				m_connection_timeout = Glib::signal_timeout().connect(
						sigc::mem_fun(*this, &MediaDecoder::on_timeout), m_timeout);
		}
		else if(old_state == Gst::STATE_PLAYING && new_state == Gst::STATE_PAUSED)
		{
			if(m_connection_timeout)
				m_connection_timeout.disconnect();
		}
		return true;
	}

	/*
	 */
	void check_missing_plugin_message(const Glib::RefPtr<Gst::MessageElement> &msg)
	{
		se_debug(SE_DEBUG_PLUGINS);

		if(!msg)
			return;
		GstMessage *gstmsg = GST_MESSAGE(msg->gobj());
		if(!gstmsg)
			return;
		if(!gst_is_missing_plugin_message(gstmsg))
			return;

		gchar *description = gst_missing_plugin_message_get_description(gstmsg);
		if(!description)
			return;

		se_debug_message(SE_DEBUG_PLUGINS, "missing plugin msg '%s'", description);

		m_missing_plugins.push_back(description);
		g_free(description);
		return;
	}

	/*
	 */
	bool check_missing_plugins()
	{
		if(m_missing_plugins.empty())
			return false;

		dialog_missing_plugins(m_missing_plugins);
		m_missing_plugins.clear();
		return true;
	}

	/*
	 * Display a message for missing plugins.
	 */
	void dialog_missing_plugins(const std::list<Glib::ustring> &list)
	{
		Glib::ustring plugins;
		
		std::list<Glib::ustring>::const_iterator it = list.begin();
		std::list<Glib::ustring>::const_iterator end = list.end();

		while(it != end)
		{
			plugins += *it;
			plugins += "\n";
			++it;
		}

		Glib::ustring msg = _(
					"GStreamer plugins missing.\n"
					"The playback of this movie requires the following decoders "
					"which are not installed:");

		dialog_error(msg, plugins);

		se_debug_message(SE_DEBUG_UTILITY, "%s %s", msg.c_str(), plugins.c_str());
	}

protected:
	guint m_watch_id;
	Glib::RefPtr<Gst::Pipeline> m_pipeline;

	// timeout
	guint m_timeout;
	sigc::connection m_connection_timeout;

	std::list<Glib::ustring> m_missing_plugins;
};

#endif//_mediadecoder_h
