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
 
#include "Options.h"
#include "utility.h"

/*
 *
 */
OptionGroup::OptionGroup()
:Glib::OptionGroup("subtitleeditor...", "description...", "help...")
{
	se_debug(SE_DEBUG_APP);
/*
#ifdef DEBUG
	debug_all = false;
	debug_app = false;
	debug_view = false;
	debug_loader = false;
	debug_saver = false;
	debug_search = false;
	debug_regex = false;
	debug_video_player = false;
	debug_spell_checking = false;
	debug_waveform = false;
	debug_utility = false;
	debug_command = false;
	debug_plugins = false;
#endif//DEBUG
*/

	set_translation_domain(GETTEXT_PACKAGE);

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

#ifdef ENABLE_UNITTEST
	launch_unittest = false;
	Glib::OptionEntry entryUnitTest;
	entryUnitTest.set_short_name('u');
	entryUnitTest.set_long_name("unittest");
	entryUnitTest.set_description("Launch a unit test suite");
	add_entry(entryUnitTest, launch_unittest);
#endif

	// FILES...
	Glib::OptionEntry entryFiles;
	entryFiles.set_long_name("");
	entryFiles.set_short_name('\0');
	entryFiles.set_description("");
	entryFiles.set_arg_description(_("[FILE...]"));
	//entryFiles.set_flags(Glib::OptionEntry::FLAG_FILENAME);
	add_entry(entryFiles, files);

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
	add_debug_option(loader, debug_loader);
	add_debug_option(saver, debug_saver);
	add_debug_option(search, debug_search);
	add_debug_option(regex, debug_regex);
	add_debug_option(video-player, debug_video_player);
	add_debug_option(spell-checking, debug_spell_checking);
	add_debug_option(waveform, debug_waveform);
	add_debug_option(utility, debug_utility);
	add_debug_option(command, debug_command);
	add_debug_option(plugins, debug_plugins);
#endif//DEBUG
}


