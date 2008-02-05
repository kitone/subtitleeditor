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
#include <iostream>
#include "SubtitleTime.h"
#include <gdk/gdkx.h>
#include "Config.h"

/*
 *
 */
Player::Player()
{
	m_widget_socket = NULL;
	m_widget_pid = 0;
	m_state = DISABLE;
}

/*
 *
 */
Player::~Player()
{
	m_connection_timeout.disconnect();
}

/*
 *
 */
bool Player::is_valid()
{
	return m_state != DISABLE;
}

/*
 *
 */
void Player::set_widget(Gtk::Widget *socket)
{
	g_return_if_fail(socket);

	m_widget_socket = socket;
	//m_widget_pid = GDK_WINDOW_XID(socket->get_window()->gobj());
}

/*
 *
 */
Gtk::Widget* Player::get_widget()
{
	//g_return_val_if_fail(m_widget_socket, NULL);
	if(m_widget_socket == NULL)
		return NULL;

	return m_widget_socket;
}

/*
 *
 */
Glib::Pid Player::get_widget_pid()
{
	//g_return_val_if_fail(m_widget_pid, 0);
	if(m_widget_socket == NULL)
		return 0;

	Glib::RefPtr<Gdk::Window> window = m_widget_socket->get_window();

	if(window)
		m_widget_pid = GDK_WINDOW_XID(window->gobj());

	return m_widget_pid;
}

/*
 *
 */
void Player::set_state(Player::State state)
{
	/*
	if(state == DISABLE)
		std::cout << "Player::set_state: DISABLE" << std::endl;
	else if(state == READY)
		std::cout << "Player::set_state: READY" << std::endl;
	else if(state == PAUSED)
		std::cout << "Player::set_state: PAUSED" << std::endl;
	else if(state == PLAYING)
		std::cout << "Player::set_state: PLAYING" << std::endl;
	*/

	if(!m_connection_timeout)
	{
		// read config for timeout value
		int msec = Config::getInstance().get_value_int("video-player", "timeout");

		// 
		m_connection_timeout = Glib::signal_timeout().connect(
				sigc::mem_fun(*this, &Player::on_timeout), msec);
		m_connection_timeout.block();
	}

	m_state = state;

	if(state == DISABLE )
		m_connection_timeout.block();
	else if(state == PAUSED)
		m_connection_timeout.block();
	else if(state == PLAYING)
	{
		m_connection_timeout.unblock();
		m_signal_timeout();
	}
	m_signal_state_changed(state);

}

Player::State Player::get_state()
{
	return m_state;
}

/*
 *	signaux
 */
sigc::signal<void>& Player::get_signal_timeout()
{
	return m_signal_timeout;
}

/*
 *
 */
sigc::signal<void, Player::State>& Player::get_signal_state_changed()
{
	return m_signal_state_changed;
}

/*
 *
 */
bool Player::on_timeout()
{
	m_signal_timeout();
	return is_playing();
}
