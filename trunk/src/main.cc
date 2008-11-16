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
#include "utility.h"
#include "gtkmm_utility.h"
#include "Options.h"

#include <ctime>
#include <gst/gst.h>
#include <gdk/gdkx.h>
#include <glib.h>

#ifdef ENABLE_GL
	#include <gtkglmm.h>
#endif//ENABLE_GL

/*
 *
 */
void get_default_config(std::map<Glib::ustring, std::map<Glib::ustring, Glib::ustring> > &config)
{
	//[general]

	//[document]
	config["document"]["format"] = "SubRip";
	config["document"]["newline"] = "Unix";

	//[video-player]
	config["video-player"]["display-translated-subtitle"] = "false";
	config["video-player"]["shaded-background"] = "true";
	config["video-player"]["font-desc"] = "Sans 26";
	config["video-player"]["force-aspect-ratio"] = "true";
	config["video-player"]["brightness"] = "0";
	config["video-player"]["contrast"] = "0";
	config["video-player"]["saturation"] = "0";
	config["video-player"]["hue"] = "0";
	config["video-player"]["skip-very-short"] = "3";
	config["video-player"]["skip-short"] = "10";
	config["video-player"]["skip-medium"] = "60";
	config["video-player"]["skip-long"] = "300";
	config["video-player"]["audio-sink"] = "autoaudiosink";
	config["video-player"]["video-sink"] = "xvimagesink";
	config["video-player"]["timeout"] = "100";
	config["video-player"]["repeat"] = "false";
	config["video-player"]["display"] = "false";
	config["video-player"]["automatically-open-video"] = "true";

	//[waveform]
	config["waveform"]["zoom"] = "1";
	config["waveform"]["scale"] = "1";
	config["waveform"]["display-background"] = "false";
	config["waveform"]["display-waveform-fill"] = "true";
	config["waveform"]["scrolling-with-player"] = "true";
	config["waveform"]["scrolling-with-selection"] = "true";
	config["waveform"]["respect-timing"] = "true";
	config["waveform"]["display"] = "false";
	config["waveform"]["renderer"] = "cairo";

	//[waveform-renderer]
	config["waveform-renderer"]["display-subtitle-text"]="true";
	config["waveform-renderer"]["color-background"]="#4C4C4CFF";
	config["waveform-renderer"]["color-wave"]="#99CC4CFF";
	config["waveform-renderer"]["color-wave-fill"]="#FFFFFFFF";
	config["waveform-renderer"]["color-subtitle"]="#994C1999";
	config["waveform-renderer"]["color-subtitle-selected"]="#E57F4C99";
	config["waveform-renderer"]["color-subtitle-invalid"]="#FFFF00CC";
	config["waveform-renderer"]["color-text"]="#FFFFFFFF";
	config["waveform-renderer"]["color-player-position"]="#FFFFFFFF";

	//[interface]
	config["interface"]["use-dynamic-keyboard-shortcuts"] = "true";
	config["interface"]["maximize-window"] = "false";
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
	config["subtitle-view"]["enable-rubberband-selection"] = "false";
	config["subtitle-view"]["columns-displayed"] = "number;start;end;duration;text";
	config["subtitle-view"]["columns-list"] = "cps;duration;effect;end;layer;margin-l;margin-r;margin-v;name;note;number;start;style;text;translation";
	config["subtitle-view"]["used-ctrl-enter-to-confirm-change"] = "false";

	//[timing]
	config["timing"]["min-characters-per-second"] = "5";
	config["timing"]["max-characters-per-second"] = "25";
	config["timing"]["min-gap-between-subtitles"] = "100";
	config["timing"]["min-display"] = "1000";
	config["timing"]["max-characters-per-line"] = "40";
	config["timing"]["max-line-per-subtitle"] = "2";

	//[external-video-player]
	config["external-video-player"]["command"] = "mplayer \"#video_file\" -noautosub -sub \"#subtitle_file\" -ss #seconds -osdlevel 2";

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
		if(op.debug_io)
			flags |= SE_DEBUG_IO;
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
		if(op.debug_profiling)
			flags |= SE_DEBUG_PROFILING;

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
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	// init Gtk+
	Gtk::Main kit(argc, argv);

#ifdef ENABLE_GL
	// init OpenGL
	Gtk::GL::init_check(argc, argv);
#endif//ENABLE_GL

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

	// Run Application
	Application*	application = gtkmm_utility::get_widget_derived<Application>(
			SE_DEV_VALUE(PACKAGE_GLADE_DIR, PACKAGE_GLADE_DIR_DEV),
			"subtitleeditor.glade", 
			"window-main");

	application->init(options);

	application->show();

	// DEBUG
	std::cout << "application run: " << double(std::clock() - start) / CLOCKS_PER_SEC << std::endl;
	
	kit.run(*application);

	delete application;

	return EXIT_SUCCESS;
}
