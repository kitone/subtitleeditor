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
 
#include "MPlayer.h"
#include "utility.h"

void print_status(Glib::IOStatus status)
{
	std::map<Glib::IOStatus, Glib::ustring> map;

	map[Glib::IO_STATUS_ERROR]="IO_STATUS_ERROR";
	map[Glib::IO_STATUS_NORMAL]="IO_STATUS_NORMAL";
	map[Glib::IO_STATUS_EOF]="IO_STATUS_EOF";
	map[Glib::IO_STATUS_AGAIN]="IO_STATUS_AGAIN";

	std::cout << "IOStatus:" << map[status] << std::endl;
}
		
MPlayer::MPlayer()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);
}

MPlayer::~MPlayer()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(m_input && m_output)
		command("quit");

	close_iochannel();
	close_pid();
}

void MPlayer::close_pid()
{
#warning "FIXME bug ici try catch..."
/*
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(m_input)
	{
		m_input->close();
	}
	if(m_output)
	{
		m_output->close();
	}
*/
	if(m_pid ==0)
		return;
	
	Glib::spawn_close_pid(m_pid);
	m_pid = 0;
}

/*
 *
 */
void MPlayer::close_iochannel()
{
	if(m_input)
		m_input->close();
	if(m_output)
		m_output->close();
}

/*
 *	open movie
 */
bool MPlayer::open(const Glib::ustring &uri)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "uri=%s", uri.c_str());

	bool state = create(uri);
	if(state)
		m_uri = uri;

	return state;
}

/*
 *
 */
bool MPlayer::is_playing()
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "is_player=%i", (get_state() == PLAYING));

	return (get_state() == PLAYING);
}

/*
 *
 */
void MPlayer::play()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(get_state() != PLAYING)
	{
		command("pause");
		set_state(PLAYING);
	}

	if(m_connection_timeout.blocked())
		m_connection_timeout.unblock();
}

/*
 *
 */
void MPlayer::pause()
{
	se_debug(SE_DEBUG_VIDEO_PLAYER);

	if(get_state() != PAUSED)
	{
		command("pause");
		set_state(PAUSED);
	}
	if(!m_connection_timeout.blocked())
		m_connection_timeout.block();
}

/*
 *
 */
long MPlayer::get_duration()
{
	float pos = (get_state() == PLAYING) ? get_float("get_time_length") : get_float("pausing get_time_length");

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "duration = %d", (long)(pos * 1000));

	return (long)(pos * 1000);
}

/*
 *
 */
long MPlayer::get_position()
{
	float pos = (get_state() == PLAYING) ? get_float("get_time_pos") : get_float("pausing get_time_pos");

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "position = %d", (long)(pos * 1000));

	return (long)(pos * 1000);
}

/*
 *
 */
void MPlayer::seek(long position)
{
	float pos = (float)(position / 1000);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "seek = %f", pos);

	command("pausing_keep seek " + to_string(pos) + " 2");
}

/*
 *
 */
void MPlayer::seek(long start, long end)
{
	float pos = (float)(start / 1000);

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "seek [%i, %i]", start, end);

	command("pausing_keep seek " + to_string(pos) + " 2");
}

/*
 *
 */
bool MPlayer::create(const Glib::ustring &uri)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "uri = %s", uri.c_str());

	set_state(DISABLE);
	close_pid();
	close_iochannel();

	int xid = get_widget_pid();

	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "xid = %i", xid);

	//if(init) 
	//	return true;

	std::vector<std::string> cmd;
	cmd.push_back("mplayer");
	cmd.push_back("-slave");
	cmd.push_back("-wid");
	cmd.push_back(to_string(xid));
	cmd.push_back("-msglevel");
	cmd.push_back("demux=-1");
	cmd.push_back("-quiet");
	cmd.push_back("-noautosub");
	cmd.push_back(Glib::filename_from_uri(uri));

	try
	{
		Glib::Pid inputFD, outputFD, errorFD;

		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "create pipe");
	
		Glib::spawn_async_with_pipes(
				".",
				cmd,
				(Glib::SPAWN_DO_NOT_REAP_CHILD | Glib::SPAWN_SEARCH_PATH),
				sigc::slot0<void>(),
				&m_pid,
				&inputFD,
				&outputFD,
				&errorFD);

		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "create iochannel (input)");

		m_input = Glib::IOChannel::create_from_fd(inputFD);

		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "create iochannel (output)");

		m_output = Glib::IOChannel::create_from_fd(outputFD);
		m_output->set_encoding("");

		if(!m_input || !m_output)
		{
			std::cerr << "Can't create iochannel" << std::endl;
			close_pid();
			return false;
		}

		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "init mplayer");

		//
		command("pausing get_time_length");

		while(true)
		{
			Glib::ustring msg;
			Glib::IOStatus status = m_output->read_line(msg);

			print_status(status);

			if(status == Glib::IO_STATUS_EOF || status == Glib::IO_STATUS_ERROR)
				break;

			se_debug_message(SE_DEBUG_VIDEO_PLAYER, "output_monitor: %s", msg.c_str());


			if(msg.find("Exiting") != Glib::ustring::npos)
			{
				close_pid();
				close_iochannel();
				return false;
			}

			if(msg.find("ANS_LENGTH=") != Glib::ustring::npos)
				break;
		}

		//
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "create iochannel output monitor");

		Glib::signal_io().connect(
				sigc::mem_fun(*this, &MPlayer::on_output_monitor), 
				m_output, 
				Glib::IO_IN | Glib::IO_ERR);//Glib::IO_IN);

		command("pause");
		set_state(PAUSED);

		//init = true;
		return true;
	}
	catch(Glib::Error &ex)
	{
		std::cerr << ex.what() << std::endl;
		close_pid();
	}
	return false;
}

/*
 *
 */
void MPlayer::show_text(const Glib::ustring &text)
{
	//osd_show_text <string> [duration] [level]
	command("osd_show_text \"" + text + "\" 5 1");
}

/*
 *
 */
bool MPlayer::on_output_monitor(Glib::IOCondition in)
{
	Glib::ustring line;
	m_output->read_line(line);
	
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "output_monitor: ", line.c_str());

	//std::cout << "on_output_read:" << line << std::endl;

	if(line.find("Exiting") != Glib::ustring::npos)
	{
		set_state(DISABLE);

		close_iochannel();

		create(m_uri);

	}
	return true;
}

void MPlayer::command(const Glib::ustring &cmd)
{
	//std::cout << cmd << std::endl;
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "command=%s", cmd.c_str());

	m_input->write(cmd + "\n");
	Glib::IOStatus status = m_input->flush();
	print_status(status);
}

/*
 *
 */
Glib::ustring MPlayer::get(const Glib::ustring &cmd)
{
	se_debug_message(SE_DEBUG_VIDEO_PLAYER, "get=%s", cmd.c_str());

	command(cmd);

	Glib::ustring result;
	m_output->read_line(result);


	Glib::ustring::size_type t = result.find("=");

	if(t != Glib::ustring::npos)
	{
		se_debug_message(SE_DEBUG_VIDEO_PLAYER, "command=%s result=%s", cmd.c_str(), result.substr(t+1).c_str());
		//std::cout << "result= " << result.substr(t+1) << std::endl;
		return result.substr(t+1);
	}
	return Glib::ustring();
}

/*
 *
 */
int	MPlayer::get_int(const Glib::ustring &cmd)
{
	Glib::ustring string = get(cmd);

	if(string.empty())
		return 0;

	try
	{
		int value = 0;
		from_string(string, value);

		return value;
	}
	catch(const std::exception &ex)
	{
		std::cerr << ex.what() << std::endl;
	}
	return 0;
}

/*
 *
 */
float	MPlayer::get_float(const Glib::ustring &cmd)
{
	Glib::ustring string = get(cmd);

	if(string.empty())
		return 0;

	try
	{
		float value = 0;
		from_string(string, value);

		return value;
	}
	catch(const std::exception &ex)
	{
		std::cerr << ex.what() << std::endl;
	}
	return 0;
}

/*
 *
 */
double MPlayer::get_double(const Glib::ustring &cmd)
{
	Glib::ustring string = get(cmd);

	if(string.empty())
		return 0;

	try
	{
		double value = 0;
		from_string(string, value);

		return value;
	}
	catch(const std::exception &ex)
	{
		std::cerr << ex.what() << std::endl;
	}
	return 0;
}

