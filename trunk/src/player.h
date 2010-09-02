#ifndef _Player_h
#define _Player_h

/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2010, kitone
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
#include "keyframes.h"

/*
 *
 */
class Player
{
public:

	enum State
	{
		NONE,
		PAUSED,
		PLAYING
	};

	enum Message
	{
		STATE_NONE,
		STATE_PAUSED,
		STATE_PLAYING,
		
		STREAM_READY,
		STREAM_INFO_CHANGED,
		STREAM_AUDIO_CHANGED,
		STREAM_VIDEO_CHANGED,
		STREAM_DURATION_CHANGED,
		STREAM_EOS,
	
		KEYFRAME_CHANGED,
	};

	/*
	 */
	Player();

	/*
	 */
	virtual ~Player();

	/*
	 */
	State get_state();

	/*
	 * Callback used by the player to send message to the application 
	 * like the change of the state of the player or change on the stream...
	 */
	sigc::signal<void, Message>& signal_message();

	/*
	 * void my_tick(long current_time, long stream_length, double current_position)
	 *
	 * current_time: position in the stream in milliseconds
	 * stream_length: length of the stream in milliseconds
	 * current_position: position in the stream as a percentage betwwen 0 and 1 (%)
	 *
	 * Emitted every time event happens or at regular intervals during playing state.
	 */
	sigc::signal<void, long, long, double>& signal_tick();

	/*
	 * 
	 */
	virtual bool open(const Glib::ustring &uri) = 0;

	/*
	 */
	virtual void close() = 0;

	/*
	 * Return the uri of the current video.
	 */
	virtual Glib::ustring get_uri() = 0;

	/*
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
	 */
	virtual void pause() = 0;

	/*
	 */
	virtual bool is_playing() = 0;

	/*
	 */
	virtual long get_duration() = 0;

	/*
	 */
	virtual long get_position() = 0;

	/*
	 */
	virtual void seek(long position) = 0;

	/*
	 */
	virtual void set_subtitle_text(const Glib::ustring &text) = 0;

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

	/*
	 * Return the number of audio track.
	 */
	virtual gint get_n_audio() = 0;

	/*
	 * Sets the current audio track. (-1 = auto)
	 */
	virtual void set_current_audio(gint track) = 0;

	/*
	 * Return the current audio track.
	 */
	virtual gint get_current_audio() = 0;

	/*
	 * Return the framerate of the video.
	 */
	virtual float get_framerate() = 0;

	/*
	 */
	void set_keyframes(Glib::RefPtr<KeyFrames> keyframes);

	/*
	 */
	Glib::RefPtr<KeyFrames> get_keyframes();

protected:

	/*
	 */
	void set_player_state(State state);

	/*
	 */
	void got_tick();

	/*
	 */
	bool on_timeout();

	/*
	 */
	void send_message(Message msg);

protected:
	sigc::signal<void, Player::Message> m_signal_message;

	sigc::connection m_timeout_connection;
	sigc::signal<void, long, long, double> m_signal_tick;

	Player::State m_player_state;

	Glib::RefPtr<KeyFrames> m_keyframes;
};

#endif//_Player_h

