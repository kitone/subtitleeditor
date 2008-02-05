#ifndef _Player_h
#define _Player_h

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
 

#include <glibmm.h>
#include <gtkmm/socket.h>

class Player
{
public:
	Player();
	virtual ~Player();

	/*
	 *
	 */
	bool is_valid();

	/*
	 *
	 */
	void set_widget(Gtk::Widget *widget);

	/*
	 *
	 */
	Gtk::Widget* get_widget();

	/*
	 *
	 */
	virtual void widget_expose(Gtk::Widget *widget) = 0;

	/*
	 *
	 */
	Glib::Pid get_widget_pid();

	/*
	 *	open movie
	 */
	virtual bool open(const Glib::ustring &uri) = 0;

	/*
	 *
	 */
	virtual bool close() = 0;

	/*
	 *
	 */
	virtual bool is_playing() = 0;

	/*
	 *
	 */
	virtual void play() = 0;

	/*
	 *
	 */
	virtual void pause() = 0;

	/*
	 *	msec
	 *	SubtitleTime(get_duration)
	 */
	virtual long get_duration() = 0;

	/*
	 *	msec
	 *	SubtitleTime(get_position)
	 */
	virtual long get_position() = 0;

	/*
	 *
	 */
	virtual void seek(long position, bool faster=false) = 0;
	/*
	 *
	 */
	virtual void seek(long start, long end, bool faster=false) = 0;

	/*
	 *	signaux sur l'état du lecteur vidéo
	 *	DISABLE: pas de vidéo ou echec
	 *	READY: player disponible pour donner des info (lenght, ...)
	 *	PAUSED: une vidéo est dispo et le lecteur aussi
	 *	PLAYING: joue actuellement une vidéo
	 */
	enum State
	{
		DISABLE,
		READY,
		PAUSED,
		PLAYING
	};

	/*
	 *
	 */
	void set_state(State state);

	/*
	 *
	 */
	State get_state();

	/*
	 *	signaux
	 */
	sigc::signal<void>& get_signal_timeout();

	/*
	 *
	 */
	sigc::signal<void, State>& get_signal_state_changed();


	/*
	 *
	 */
	virtual void show_text(const Glib::ustring &text) = 0;

protected:
	
	bool on_timeout();

protected:
	Gtk::Widget* m_widget_socket;
	Glib::Pid m_widget_pid;
	State m_state;
	sigc::connection m_connection_timeout;
	sigc::signal<void> m_signal_timeout;
	sigc::signal<void, State> m_signal_state_changed;
};

#endif//_Player_h

