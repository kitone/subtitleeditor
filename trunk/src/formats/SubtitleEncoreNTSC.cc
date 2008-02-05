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
 *
 *
 *	Adobe Encore DVD text script support for subtitle editor
 *      NTSC version.
 *
 *	Adobe Encore DVD text script support by Laurens Keek
 *      Created using following documentation:
 *	http://www.adobe.com/support/techdocs/329569.html
 */
 

#include "SubtitleEncoreNTSC.h"
#include "Color.h"
#include "utility.h"

#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "RegEx.h"

/*
 *
 */
Glib::ustring SubtitleEncoreNTSC::get_name()
{
	return "Adobe Encore DVD NTSC";
}

/*
 *
 */
Glib::ustring SubtitleEncoreNTSC::get_extension()
{
	return "txt";
}

/*
 *
 */
bool SubtitleEncoreNTSC::check(const std::string &line)
{
	/* First line should simply be: number start_time stop_time some_text*/
	static RegEx ex("^\\d+ (\\d\\d(;)\\d\\d\\2\\d\\d\\2\\d\\d ){2}.*?$");

	return ex.exec(line);
}

/*
 *	constructor
 */
SubtitleEncoreNTSC::SubtitleEncoreNTSC(Document* doc)
:SubtitleFormat(doc, get_name())
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *
 */
SubtitleEncoreNTSC::~SubtitleEncoreNTSC()
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *	read subtitle file
 */
bool SubtitleEncoreNTSC::on_open(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_LOADER);

	std::ifstream file(filename.c_str());
	
	if(!file)
	{
		throw SubtitleException("SubtitleEncoreNTSC", _("I can't open this file."));
	}

	std::string line;
	int num;
	int start[4], end[4];
	char *text;
	int framerate = 30;

	Subtitles subtitles = document()->subtitles();
	Subtitle sub;

	while(!file.eof() && std::getline(file, line))
	{
		text = (char *) malloc (line.length());

		if (sscanf(line.c_str(), "%d %d;%d;%d;%d %d;%d;%d;%d %[^\n]", &num,
					&start[0], &start[1], &start[2], &start[3], 
					&end[0], &end[1], &end[2], &end[3],
					text) == 10) 
		{
			// we have a new subtitle
			sub = subtitles.append();
			//convert times
			start[3] = start[3]*1000/framerate;
			end[3] = end[3]*1000/framerate;
			//set subtitle data
			sub.set_start_and_end(
						SubtitleTime(start[0], start[1], start[2], start[3]),
						SubtitleTime(end[0], end[1], end[2], end[3]));
			sub.set_text(charset_to_utf8(text));
		}
		else {
			//this is another line of the previous subtitle
			sub.set_text(sub.get_text() + get_newline() + charset_to_utf8(line));
		}
		free(text);
	}

	file.close();

	return true;
}

/*
 *	Save subtitle file
 */
bool SubtitleEncoreNTSC::on_save(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_SAVER);

	std::ofstream file(filename.c_str());

	if(!file)
	{
		throw SubtitleException("SubtitleEncoreNTSC", _("I can't open this file."));
	}
	
	se_debug_message(SE_DEBUG_SAVER, "save Event");

	// Event
	{
		// dialog:
		for(Subtitle subtitle = document()->subtitles().get_first(); subtitle; ++subtitle)
		{
			std::ostringstream oss;

			Glib::ustring text = subtitle.get_text();
			
			newline_to_characters(text, get_newline());

			oss	<< subtitle.get_num() << " "
				<< subtitletime_to_encore_time(subtitle.get_start()) << " "
				<< subtitletime_to_encore_time(subtitle.get_end()) << " "
				<< utf8_to_charset(text) << get_newline();

			file << oss.str();
		}
	}
	
	file.close();

	return true;
}

/*
 *	convertir le temps utiliser par subtitle editor en temps valide pour le format Encore DVD
 *	0:00:00.000 -> 00:00:00:00 (last 00 are frames, not time!)
 */
Glib::ustring SubtitleEncoreNTSC::subtitletime_to_encore_time(const SubtitleTime &time)
{
	int framerate = 30;
	int frame = (int)(time.msecs*framerate*0.001);

	gchar *tmp = g_strdup_printf("%02i;%02i;%02i;%02i",
			time.hours, time.mins, time.secs, frame);

	Glib::ustring res(tmp);

	g_free(tmp);

	return res;
}
