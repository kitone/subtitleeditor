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
 

#include "SubtitleMPL2.h"
#include <iostream>
#include <fstream>
#include <glibmm/ustring.h>
#include "utility.h"
#include "RegEx.h"

/*
 *
 */
Glib::ustring SubtitleMPL2::get_name()
{
	return "MPL2";
}

/*
 *
 */
Glib::ustring SubtitleMPL2::get_extension()
{
	return "txt";
}

/*
 *
 */
bool SubtitleMPL2::check(const std::string &line)
{
	static RegEx ex("^\\[(\\d+)\\]\\[(\\d+)\\](.*?)$");

	return ex.exec(line);
}


/*
 *
 */
SubtitleMPL2::SubtitleMPL2(Document* doc)
:SubtitleFormat(doc, get_name())
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *
 */
SubtitleMPL2::~SubtitleMPL2()
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}


/*
 *
 */
bool SubtitleMPL2::on_open(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_LOADER);

	std::ifstream file(filename.c_str());
	if(!file)
		throw IOFileError(_("Failed to open the file for reading."));

	//double ds = SubtitleTime(0,0,0,100).totalmsecs;
	double ds = 100;

	RegEx ex("^\\[(\\d+)\\]\\[(\\d+)\\](.*?)$");

	std::string text;
	int start, end;
	std::string line;
	Subtitles subtitles = document()->subtitles();

	while(!file.eof() && std::getline(file, line))
	{
		// on la passe tout de suite en utf8
		// car regex utilise de l'utf8
		Glib::ustring utf8_line = charset_to_utf8(line);

		if(ex.FullMatch(utf8_line.c_str(), &start, &end, &text))
		{
			Subtitle subtitle = subtitles.append();

			Glib::ustring utf8_text = check_end_char(charset_to_utf8(text));

			characters_to_newline(utf8_text, "|");

			subtitle.set_text(utf8_text);
			subtitle.set_start_and_end(
					SubtitleTime((long int)(start*ds)),
					SubtitleTime((long int)(end*ds)));
		}

	}

	file.close();

	return true;
}

/*
 *
 */
bool SubtitleMPL2::on_save(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_SAVER);

	std::ofstream file(filename.c_str());
	if(!file)
		throw IOFileError(_("Failed to open the file for writing."));

	// TODO : change!
	double ds = 100;
	
	Glib::ustring text;
	
	for(Subtitle subtitle = document()->subtitles().get_first(); subtitle; ++subtitle)
	{
		SubtitleTime start = subtitle.get_start();
		SubtitleTime end = subtitle.get_end();

		text = subtitle.get_text();

		newline_to_characters(text, "|");

		file << "["
			<< (unsigned int)(start.totalmsecs / ds)
			<< "]["
			<< (unsigned int)(end.totalmsecs / ds)
			<< "]"
			<< utf8_to_charset(text)
			<< get_newline();
	}
	
	file.close();

	return true;
}

