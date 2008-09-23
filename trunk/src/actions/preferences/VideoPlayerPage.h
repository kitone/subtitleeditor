#ifndef _VideoPlayerPage_h
#define _VideoPlayerPage_h

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

#include "PreferencePage.h"

struct t_output
{
	const gchar *label;
	const gchar *name;
};

t_output m_audio_output[]={
	{N_("Autodetect"), "autoaudiosink"},
	{N_("ALSA - Advanced Linux Sound Architecture"), "alsasink"},
	{N_("ESD - Enlightenment Sound Daemon"), "esdsink"},
	{N_("OSS - Open Sound System"), "ossink"},
	{N_("SDL - Simple DirectMedia Layer"), "sdlaudiosink"},
	{N_("GConf"), "gconfaudiosink"},
	{NULL, NULL}
};

t_output m_video_output[]={
	{N_("Autodetect"), "autovideosink"},
	{N_("X Window System (X11/XShm/Xv)"), "xvimagesink"},
	{N_("X Window System (No Xv)"), "ximagesink"},
	{N_("SDL - Simple DirectMedia Layer"), "sdlvideosink"},
	{N_("GConf"), "gconfvideosink"},
	{N_("OpenGL"), "glimagesink"},
	{NULL, NULL}
};

class VideoPlayerPage : public PreferencePage
{
public:

	/*
	 *
	 */
	VideoPlayerPage(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
	:PreferencePage(cobject)
	{
		init_widget(xml, "fontbutton-subtitle", "video-player", "font-desc");
		init_widget(xml, "check-use-shaded-background", "video-player", "shaded-background");
		init_widget(xml, "check-display-translated-subtitle", "video-player", "display-translated-subtitle");

		init_widget(xml, "check-force-aspect-ratio", "video-player", "force-aspect-ratio");
		init_widget(xml, "check-automatically-open-video", "video-player", "automatically-open-video");

		xml->get_widget_derived("combo-audio-output", m_comboAudioOutput);
		xml->get_widget_derived("combo-video-output", m_comboVideoOutput);

		for(unsigned int i=0; m_audio_output[i].label != NULL; ++i)
		{
			m_comboAudioOutput->append_text(m_audio_output[i].label);
		}

		for(unsigned int i=0; m_video_output[i].label != NULL; ++i)
		{
			m_comboVideoOutput->append_text(m_video_output[i].label);
		}

		Glib::ustring audiosink, videosink;
		Config::getInstance().get_value_string("video-player", "audio-sink", audiosink);
		Config::getInstance().get_value_string("video-player", "video-sink", videosink);

		m_comboAudioOutput->set_active_text(get_output_label_by_name(m_audio_output, audiosink));
		m_comboVideoOutput->set_active_text(get_output_label_by_name(m_video_output, videosink));

		m_comboAudioOutput->signal_changed().connect(
				sigc::mem_fun(*this, &VideoPlayerPage::on_audio_output_changed));
		m_comboVideoOutput->signal_changed().connect(
				sigc::mem_fun(*this, &VideoPlayerPage::on_video_output_changed));
	}
	
protected:

	Glib::ustring get_output_label_by_name(t_output output[], const Glib::ustring &name)
	{
		for(unsigned int i=0; output[i].name != NULL; ++i)
		{
			if(name == output[i].name)
				return _(output[i].label);
		}
		return Glib::ustring();
	}

	Glib::ustring get_output_name_by_label(t_output output[], const Glib::ustring &label)
	{
		for(unsigned int i=0; output[i].name != NULL; ++i)
		{
			if(label == _(output[i].label))
				return output[i].name;
		}
		return Glib::ustring();
	}

	void on_audio_output_changed()
	{
		Glib::ustring name = get_output_name_by_label(m_audio_output, m_comboAudioOutput->get_active_text());
		Config::getInstance().set_value_string("video-player", "audio-sink", name);
	}

	void on_video_output_changed()
	{
		Glib::ustring name = get_output_name_by_label(m_video_output, m_comboVideoOutput->get_active_text());
		Config::getInstance().set_value_string("video-player", "video-sink", name);
	}

protected:
	ComboBoxText*	m_comboAudioOutput;
	ComboBoxText*	m_comboVideoOutput;
};

#endif//_VideoPlayerPage_h
