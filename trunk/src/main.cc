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

#include <iostream>
#include <config.h>
#include <gtkmm/main.h>
#include "gui/Application.h"

#include "debug.h"
#include "DocumentSystem.h"
#include "utility.h"

#include <ctime>
#include <gst/gst.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include <gtkmm/socket.h>

#include "gui/MPlayer.h"
#include "gui/GStreamerPlayer.h"
#include "SubtitleSystem.h"
#include "Options.h"

#ifdef ENABLE_UNITTEST
	bool launch_unittest();
#endif//ENABLE_UNITTEST

#include "RegEx.h"

/*
 *
 */
void get_default_config(std::map<Glib::ustring, std::map<Glib::ustring, Glib::ustring> > &config)
{
	//[general]
	config["general"]["automatically-open-video"] = "true";

	//[video-player]
	config["video-player"]["display-translated-subtitle"] = "false";
	config["video-player"]["shaded-background"] = "true";
	config["video-player"]["font-desc"] = "Sans 26";
	config["video-player"]["force-aspect-ratio"] = "true";
	config["video-player"]["brightness"] = "0";
	config["video-player"]["contrast"] = "0";
	config["video-player"]["saturation"] = "0";
	config["video-player"]["hue"] = "0";
	config["video-player"]["jump-very-short"] = "3";
	config["video-player"]["jump-short"] = "10";
	config["video-player"]["jump-medium"] = "60";
	config["video-player"]["jump-long"] = "300";
	config["video-player"]["audio-sink"] = "autoaudiosink";
	config["video-player"]["video-sink"] = "xvimagesink";
	config["video-player"]["timeout"] = "100";

	//[waveform]
	config["waveform"]["zoom"] = "1";
	config["waveform"]["scale"] = "1";
	config["waveform"]["color-background"] = "#E6E6E6FF";
	config["waveform"]["color-play-line"] = "#000000FF";
	config["waveform"]["color-text"] = "#000000FF";
	config["waveform"]["color-wave"] = "#00B300FF";
	config["waveform"]["color-wave-fill"] = "#007F00FF";
	config["waveform"]["color-marker"] = "#6666667F";
	config["waveform"]["color-marker-hightlight"] = "#4C7FCF7F";
	config["waveform"]["color-marker-invalid"] = "#FF00007F";
	config["waveform"]["display-background"] = "false";
	config["waveform"]["display-waveform-fill"] = "true";
	config["waveform"]["display-subtitle-text"] = "true";
	config["waveform"]["scrolling-with-cursor"] = "true";
	config["waveform"]["scrolling-with-selection"] = "true";

	//[interface]
	config["interface"]["use-dynamic-keyboard-shortcuts"] = "true";
	config["interface"]["maximize-window"] = "false";
	config["interface"]["display-video-player"] = "false";
	config["interface"]["display-waveform"] = "false";
	config["interface"]["used-autosave"] = "false";
	config["interface"]["ask-to-save-on-exit"] = "false";
	config["interface"]["create-backup-copy"] = "false";
	config["interface"]["autosave-minutes"] = "10";
	config["interface"]["max-undo"] = "20";

	//[encodings]
	config["encodings"]["encodings"] = "ISO-8859-15;UTF-8";
	config["encodings"]["used-auto-detected"] = "true";
	config["encodings"]["default"] = "UTF-8";

	//[subtitle-view]
	config["subtitle-view"]["property-alignment-center"] = "false";
	config["subtitle-view"]["show-character-per-line"] = "true";
	config["subtitle-view"]["show-column-number"] = "true";
	config["subtitle-view"]["show-column-layer"] = "false";
	config["subtitle-view"]["show-column-start"] = "true";
	config["subtitle-view"]["show-column-end"] = "true";
	config["subtitle-view"]["show-column-duration"] = "true";
	config["subtitle-view"]["show-column-style"] = "false";
	config["subtitle-view"]["show-column-name"] = "false";
	config["subtitle-view"]["show-column-margin-l"] = "false";
	config["subtitle-view"]["show-column-margin-r"] = "false";
	config["subtitle-view"]["show-column-margin-v"] = "false";
	config["subtitle-view"]["show-column-effect"] = "false";
	config["subtitle-view"]["show-column-text"] = "true";
	config["subtitle-view"]["show-column-translation"] = "false";
	config["subtitle-view"]["show-column-note"] = "false";
	config["subtitle-view"]["enable-rubberband-selection"] = "false";
	config["subtitle-view"]["columns"] = "number;layer;start;end;duration;style;name;margin-r;margin-l;margin-v;effect;text;cps;translation;note";

	//[timing]
	config["timing"]["min-characters-per-second"] = "5";
	config["timing"]["max-characters-per-second"] = "25";
	config["timing"]["min-gap-between-subtitles"] = "100";
	config["timing"]["min-display"] = "1000";
	config["timing"]["max-characters-per-line"] = "40";
	config["timing"]["max-line-per-subtitle"] = "2";

	//[external-video-player]
	config["external-video-player"]["command"] = "mplayer \"#video_file\" -sub \"#subtitle_file\" -ss #seconds -osdlevel 2";

	//[dialog-check-errors]
	config["dialog-check-errors"]["check-overlapping"] = "true";
	config["dialog-check-errors"]["check-too-short-display-time"] = "true";
	config["dialog-check-errors"]["check-too-long-display-time"] = "true";
	config["dialog-check-errors"]["check-too-long-line"] = "true";
	config["dialog-check-errors"]["check-gap-between-subtitles"] = "true";
	config["dialog-check-errors"]["check-max-line-per-subtitle"] = "true";

	//[check-error-plugins]
	config["check-error-plugins"]["overlapping-color"] = "#FF0000FF";
	config["check-error-plugins"]["too-short-display-time-color"] = "#B6D2FFFF";
	config["check-error-plugins"]["too-long-display-time-color"] = "#0414FFFF";
	config["check-error-plugins"]["too-long-line-color"] = "#FFDC0AFF";
	config["check-error-plugins"]["max-line-per-subtitle-color"] = "#00BF23FF";
	config["check-error-plugins"]["min-gap-between-subtitles-color"] = "#FF66ECFF";
}


#ifdef DEBUG

void parse_debug_options(OptionGroup &op)
{
	int flags = 0;
	if(op.debug_all)
	{
		std::cout << "DEBUG_ALL" << std::endl;

		flags |= SE_DEBUG_ALL;

		se_debug_init(flags);
	}
	else
	{
		if(op.debug_app)
			flags |= SE_DEBUG_APP;
		if(op.debug_view)
			flags |= SE_DEBUG_VIEW;
		if(op.debug_loader)
			flags |= SE_DEBUG_LOADER;
		if(op.debug_saver)
			flags |= SE_DEBUG_SAVER;
		if(op.debug_search)
			flags |= SE_DEBUG_SEARCH;
		if(op.debug_regex)
			flags |= SE_DEBUG_REGEX;
		if(op.debug_video_player)
			flags |= SE_DEBUG_VIDEO_PLAYER;
		if(op.debug_spell_checking)
			flags |= SE_DEBUG_SPELL_CHECKING;
		if(op.debug_waveform)
			flags |= SE_DEBUG_WAVEFORM;
		if(op.debug_utility)
			flags |= SE_DEBUG_UTILITY;
		if(op.debug_command)
			flags |= SE_DEBUG_COMMAND;
		if(op.debug_plugins)
			flags |= SE_DEBUG_PLUGINS;

		se_debug_init(flags);
	}
}

#endif//DEBUG


/*
 *
 */
int main(int argc, char *argv[])
{
	if(!g_thread_supported())
		g_thread_init (NULL);

	// DEBUG
	std::clock_t start = std::clock();

	// Bindtextdomain
	bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain(PACKAGE);

	// init Gtk+
	Gtk::Main kit(argc, argv);

	Glib::set_application_name("subtitleeditor");

	// SubtitleEditor Options
	OptionGroup options;
	try
	{
		Glib::OptionContext context(_(" - edit subtitles files"));
		context.set_main_group(options);
		
		Glib::OptionGroup gst_group(gst_init_get_option_group());
		context.add_group(gst_group);
		
		Glib::OptionGroup gtk_group(gtk_get_option_group(TRUE));
		context.add_group(gtk_group);

		context.parse(argc, argv);
	}
	catch(const Glib::Error &ex)
	{
		std::cerr << "Error loading options : " << ex.what() << std::endl;
	}
	
	// Init Debug
#ifdef DEBUG
	parse_debug_options(options);
	se_debug_message(SE_DEBUG_APP, "Startup subtitle version %s", VERSION);
#endif//DEBUG

	if(!options.profile.empty())
		set_profile_name(options.profile);

	// Init GStreamer 
	se_debug_message(SE_DEBUG_APP, "Init GStreamer");
	
	gst_init(&argc, &argv);

#ifdef ENABLE_UNITTEST
	if(options.launch_unittest)
	{
		return launch_unittest();
	}
#endif//ENABLE_UNITTEST
	
	// Run Application
	Application*	application = utility::get_widget_derived<Application>("subtitleeditor.glade", "window-main");

	application->init(options);

	application->show();

	// DEBUG
	std::cout << "application run: " << double(std::clock() - start) / CLOCKS_PER_SEC << std::endl;
	
	kit.run(*application);

	delete application;

	return EXIT_SUCCESS;
}
