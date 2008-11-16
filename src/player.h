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
#include "document.h"

/*
 *
 */
class Player
{
public:

	enum State
	{
		NONE,
		READY,
		PAUSED,
		PLAYING
	};

	/*
	 *
	 */
	Player()
	{
	}

	/*
	 *
	 */
	virtual ~Player()
	{
	}

	/*
	 * 
	 */
	virtual bool open(const Glib::ustring &uri) = 0;

	/*
	 *
	 */
	virtual void close() = 0;

	/*
	 *
	 */
	virtual void play() = 0;

	/*
	 * Try to play the segment defined by the subtitle (start to end).
	 * This function supports the looping.
	 * The state is sets to playing.
	 */
	virtual void play_subtitle(const Subtitle &sub) = 0;

	/*
	 * Try to play the segment defined (start to end).
	 * This function don't support the mode looping.
	 * The state is sets to playing.
	 */
	virtual void play_segment(const SubtitleTime &start, const SubtitleTime &end) = 0;

	/*
	 *
	 */
	virtual void pause() = 0;

	/*
	 *
	 */
	virtual bool is_playing() = 0;

	/*
	 *
	 */
	virtual long get_duration() = 0;

	/*
	 *
	 */
	virtual long get_position() = 0;

	/*
	 *
	 */
	virtual void seek(long position) = 0;

	/*
	 *
	 */
	virtual void set_subtitle_text(const Glib::ustring &text) = 0;

	/*
	 *
	 */
	virtual State get_state() = 0;

	/*
	 *
	 */
	virtual sigc::signal<void, State>& signal_state_changed() = 0;

	/*
	 *
	 */
	virtual sigc::signal<void>& signal_timeout() = 0;

	/*
	 * Sets the new playback rate. Used for slow or fast motion.
	 * Default value : 1.0
	 * Min : 0.1
	 * Max : 1.5 
	 */
	virtual void set_playback_rate(double value) = 0;

	/*
	 * Return the playback rate.
	 */
	virtual double get_playback_rate() = 0;

	/*
	 * Enable/Disable the repeat mode.
	 * Works only with play_subtitle.
	 */
	virtual void set_repeat(bool state) = 0;
};

#endif//_Player_h

