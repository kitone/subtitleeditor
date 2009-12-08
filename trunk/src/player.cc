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

#include "player.h"

/*
 */
Player::Player()
{
	m_player_state = NONE;
}

/*
 */
Player::~Player()
{
}

/*
 * Sets the current state of the pipeline.
 * Block or unblock the timeout signal and emit the signal state_changed.
 */
void Player::set_player_state(Player::State state)
{
	m_player_state = state;

	// create the timeout callback
	// the signal is directly blocked
	if(!m_timeout_connection)
	{
		int msec = 100;
		m_timeout_connection = Glib::signal_timeout().connect(
				sigc::mem_fun(*this, &Player::on_timeout), msec);
		m_timeout_connection.block();
	}

	if(state == PLAYING)
	{
		m_timeout_connection.unblock();
		m_timeout_signal();
	}
	else
	{
		m_timeout_signal(); // update with last position
		m_timeout_connection.block();
	}
	m_signal_state_changed(state);
}

/*
 */
bool Player::on_timeout()
{
	m_timeout_signal();

	return is_playing();
}

/*
 */
sigc::signal<void, Player::State>& Player::signal_state_changed()
{
	return m_signal_state_changed;
}

/*
 */
sigc::signal<void>& Player::signal_timeout()
{
	return m_timeout_signal;
}

/*
 * Return the state of the player.
 * NONE can be considerate as NULL, the pipeline is not create.
 */
Player::State Player::get_state()
{
	return m_player_state;
}

/*
 */
void Player::set_keyframes(Glib::RefPtr<KeyFrames> keyframes)
{
	m_keyframes = keyframes;
	m_keyframes_signal_changed();
}

/*
 */
Glib::RefPtr<KeyFrames> Player::get_keyframes()
{
	return m_keyframes;
}

/*
 */
sigc::signal<void>& Player::signal_keyframes_changed()
{
	return m_keyframes_signal_changed;
}

