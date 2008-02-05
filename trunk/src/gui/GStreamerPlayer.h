#ifndef _GStreamerPlayer_h
#define _GStreamerPlayer_h

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
 

#include "Player.h"
#include <gst/gst.h>

class GStreamerPlayer : public Player
{
public:
	GStreamerPlayer();
	~GStreamerPlayer();

	/*
	 *	open movie
	 */
	bool open(const Glib::ustring &uri);

	/*
	 *
	 */
	bool close();

	/*
	 *
	 */
	bool is_playing();

	/*
	 *
	 */
	void play();

	/*
	 *
	 */
	void pause();

	/*
	 *
	 */
	long get_duration();

	/*
	 *
	 */
	long get_position();

	/*
	 *
	 */
	void seek(long position, bool faster=false);
	
	/*
	 *
	 */
	void seek(long start, long end, bool faster=false);

	/*
	 *
	 */
	void show_text(const Glib::ustring& text);

	/*
	 *
	 */
	void widget_expose(Gtk::Widget *widget);
protected:
	
	/*
	 *
	 */
	static gboolean __static_bus_message(GstBus *bus, GstMessage *msg, gpointer data);

	/*
	 *
	 */
	bool create_pipeline();

	/*
	 *
	 */
	void ready();

	/*
	 *
	 */
	bool bus_message(GstBus *bus, GstMessage *msg);

	/*
	 *
	 */
	void bus_message_warning(GstBus *bus, GstMessage *msg);
	void bus_message_error(GstBus *bus, GstMessage *msg);
	void bus_message_eos(GstBus *bus, GstMessage *msg);
	void bus_message_state_changed(GstBus *bus, GstMessage *msg);
	//void bus_message(GstBus *bus, GstMessage *msg);


	void on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value);

	/*
	 *
	 */
	GstElement* gen_video_element();

	/*
	 *
	 */
	GstElement* gen_audio_element();

	/*
	 *
	 */
	void on_newpad(GstElement *element, GstPad *pad, bool last);

	/*
	 *
	 */
	static void __static_newpad(GstElement *e, GstPad *p, gboolean last, gpointer data);

	/*
	 *
	 */
	GstBusSyncReply on_element_msg_sync(GstBus *bus, GstMessage *msg);
	static GstBusSyncReply __static_element_msg_sync(GstBus *bus, GstMessage *msg, gpointer data);

	/*
	 *	crée un element grace à gst_element_factory_make
	 *	s'il y a une erreur affiche un message d'erreur dans un dialog (msg_error)
	 */
	GstElement* create_element(const std::string &element, const std::string &name, const Glib::ustring &msg_error);

	/*
	 *
	 */
	void setup_null();
protected:

	Glib::ustring	m_uri;
	
	// Gstreamer pipeline
	GstElement*	m_pipeline;
	GstElement*	m_filesrc;
	GstElement*	m_textoverlay;
	GstElement*	m_audiosink;
	GstElement*	m_videosink;

	GstState		m_pipeline_state;

	Glib::ustring m_current_text;
};

#endif//_GStreamerPlayer_h

