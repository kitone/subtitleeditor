#ifndef _Options_h
#define _Options_h

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
 

#include <config.h>
#include <glibmm.h>

/*
 *
 */
class OptionGroup : public Glib::OptionGroup
{
public:
	OptionGroup();

	int get_debug_flags();

public:
	std::vector<Glib::ustring> files;
	std::vector<Glib::ustring> files_list; // simple file (glibmm Bug #526831)

	Glib::ustring profile;	// profile name
	Glib::ustring	encoding;	//
	Glib::ustring video;		// video location
	Glib::ustring waveform;	// waveform location

#ifdef DEBUG
	bool debug_all;
	bool debug_app;
	bool debug_view;
	bool debug_io;
	bool debug_search;
	bool debug_regex;
	bool debug_video_player;
	bool debug_spell_checking;
	bool debug_waveform;
	bool debug_utility;
	bool debug_command;
	bool debug_plugins;
	bool debug_profiling;
#endif//DEBUG

};

#endif//_Options_h

