#ifndef _MPlayer_h
#define _MPlayer_h

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

class MPlayer : public Player
{
public:
	MPlayer();
	~MPlayer();

	/*
	 *	open movie
	 */
	bool open(const Glib::ustring &uri);

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
	void seek(long position);
	
	/*
	 *
	 */
	void seek(long start, long end);

	/*
	 *
	 */
	void show_text(const Glib::ustring& text);
protected:

	/*
	 *	xid = xwindow
	 */
	bool create(const Glib::ustring &uri);

	/*
	 *
	 */
	void close_pid();
	void close_iochannel();

	/*
	 *
	 */
	void command(const Glib::ustring &cmd);
	
	/*
	 *
	 */
	Glib::ustring get(const Glib::ustring &cmd);

	int			get_int(const Glib::ustring &cmd);
	float		get_float(const Glib::ustring &cmd);
	double	get_double(const Glib::ustring &cmd);

	/*
	 *
	 */
	bool on_output_monitor(Glib::IOCondition);

protected:
	Glib::Pid m_pid;
	Glib::RefPtr<Glib::IOChannel> m_input;
	Glib::RefPtr<Glib::IOChannel> m_output;

	Glib::ustring m_uri;
};

#endif//_MPlayer_h

