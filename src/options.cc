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
 
#include "options.h"
#include "i18n.h"
#include "debug.h"

/*
 */
OptionGroup::OptionGroup()
:Glib::OptionGroup("subtitleeditor...", "description...", "help...")
{
	se_debug(SE_DEBUG_APP);

	set_translation_domain(GETTEXT_PACKAGE);

	// FILES...
	Glib::OptionEntry entryFiles;
	entryFiles.set_long_name(G_OPTION_REMAINING);
	entryFiles.set_description(G_OPTION_REMAINING);
	entryFiles.set_arg_description(_("[FILE...]"));
	add_entry(entryFiles, files);

	// files_list
	Glib::OptionEntry entryFile;
	entryFile.set_long_name("file");
	entryFile.set_short_name('f');
	entryFile.set_description("open a file (-f file1 -f file2 --file=file3)");
	entryFile.set_arg_description(_("FILE"));
	add_entry(entryFile, files_list);

	// profile
	Glib::OptionEntry entryProfile;
	entryProfile.set_long_name("profile");
	entryProfile.set_short_name('p');
	entryProfile.set_description("the name of the profile used by the config");
	entryProfile.set_arg_description(_("NAME"));
	add_entry(entryProfile, profile);

	// encoding
	Glib::OptionEntry entryEncoding;
	entryEncoding.set_long_name("encoding");
	entryEncoding.set_short_name('e');
	entryEncoding.set_description("encoding used to open files");
	entryEncoding.set_arg_description(_("ENCODING"));
	add_entry(entryEncoding, encoding);

	// video
	Glib::OptionEntry entryVideo;
	entryVideo.set_long_name("video");
	entryVideo.set_short_name('v');
	entryVideo.set_description("open video file");
	entryVideo.set_arg_description(_("FILE"));
	add_entry(entryVideo, video);
		
	// waveform
	Glib::OptionEntry entryWaveform;
	entryWaveform.set_long_name("waveform");
	entryWaveform.set_short_name('w');
	entryWaveform.set_description("open waveform file");
	entryWaveform.set_arg_description(_("FILE"));
	add_entry(entryWaveform, waveform);

#ifdef DEBUG

#define add_debug_option(name, value) { \
					value = false; \
					Glib::OptionEntry e; \
					e.set_long_name("debug-"#name); \
					add_entry(e, value); \
				}

	add_debug_option(all, debug_all);
	add_debug_option(app, debug_app);
	add_debug_option(view, debug_view);
	add_debug_option(io, debug_io);
	add_debug_option(search, debug_search);
	add_debug_option(regex, debug_regex);
	add_debug_option(video-player, debug_video_player);
	add_debug_option(spell-checking, debug_spell_checking);
	add_debug_option(waveform, debug_waveform);
	add_debug_option(utility, debug_utility);
	add_debug_option(command, debug_command);
	add_debug_option(plugins, debug_plugins);
	add_debug_option(profiling, debug_profiling);

#undef add_debug_option

#endif//DEBUG
}


/*
 */
int OptionGroup::get_debug_flags()
{
	int flags = 0;

#ifdef DEBUG
	if(debug_all) {
		flags |= SE_DEBUG_ALL;
		return flags;
	}

	if(debug_app)
		flags |= SE_DEBUG_APP;
	if(debug_view)
		flags |= SE_DEBUG_VIEW;
	if(debug_io)
		flags |= SE_DEBUG_IO;
	if(debug_search)
		flags |= SE_DEBUG_SEARCH;
	if(debug_regex)
		flags |= SE_DEBUG_REGEX;
	if(debug_video_player)
		flags |= SE_DEBUG_VIDEO_PLAYER;
	if(debug_spell_checking)
		flags |= SE_DEBUG_SPELL_CHECKING;
	if(debug_waveform)
		flags |= SE_DEBUG_WAVEFORM;
	if(debug_utility)
		flags |= SE_DEBUG_UTILITY;
	if(debug_command)
		flags |= SE_DEBUG_COMMAND;
	if(debug_plugins)
		flags |= SE_DEBUG_PLUGINS;
	if(debug_profiling)
		flags |= SE_DEBUG_PROFILING;

#endif//DEBUG

	return flags;
}


