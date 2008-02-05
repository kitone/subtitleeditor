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


#include "SubtitleSubRip.h"
#include <iostream>
#include <fstream>
#include <glibmm/ustring.h>
#include "utility.h"
#include "RegEx.h"

/*
 *
 */
Glib::ustring SubtitleSubRip::get_name()
{
	return "SubRip";
}

/*
 *
 */
Glib::ustring SubtitleSubRip::get_extension()
{
	return "srt";
}

/*
 *
 */
bool SubtitleSubRip::check(const std::string &line)
{
	std::string time("\\d\\d:\\d\\d:\\d\\d,\\d\\d\\d");
	static RegEx ex(
			"^" + time + " --> " + time + "\\s*$");

	return ex.exec(line);
}

/*
 *
 */
SubtitleSubRip::SubtitleSubRip(Document* doc)
:SubtitleFormat(doc, get_name())
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *
 */
SubtitleSubRip::~SubtitleSubRip()
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}


/*
 *
 */
bool SubtitleSubRip::on_open(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_LOADER);

	std::ifstream file(filename.c_str());
	if(!file)
	{
		throw SubtitleException("SubtitleSubRip", _("I can't open this file."));
	}

	std::string line;
	Subtitles subtitles = document()->subtitles();

	while(!file.eof() && std::getline(file, line))
	{
		unsigned int num = 0;

		// lecture du numero
		if(sscanf(line.c_str(), "%d", &num) > 0)
		{
			// lecture du temps
			if(std::getline(file, line))
			{
				int start[4], end[4];

				if(sscanf(line.c_str(), "%d:%d:%d,%d --> %d:%d:%d,%d",
							&start[0], &start[1], &start[2], &start[3], 
							&end[0], &end[1], &end[2], &end[3]) == 8)
				{
					Glib::ustring text;
					int count = 0; // permet de compter le nombre de ligne

					while(std::getline(file, line))
					{
						line = check_end_char(charset_to_utf8(line));

						if(line.size()==0)
							break;
						else
						{
							if(count > 0) 
								text += get_newline();

							text += line;
							++count;
						}
					}

					Subtitle subtitle = subtitles.append();

					subtitle.set_text(text);

					subtitle.set_start_and_end(
							SubtitleTime(start[0], start[1], start[2], start[3]),
							SubtitleTime(end[0], end[1], end[2], end[3]));
				}
			}
		}
	}

	file.close();

	return true;
}

/*
 *
 */
bool SubtitleSubRip::on_save(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_SAVER);

	std::ofstream file(filename.c_str());
	if(!file)
	{
		throw SubtitleException("SubtitleSubRip", _("I can't open this file."));
	}

	bool change_newline = (m_newline != "\n");

	Glib::ustring text;

	for(Subtitle subtitle = document()->subtitles().get_first(); subtitle; ++subtitle)
	{
		file << subtitle.get_num() 
			<< get_newline()
			<< subtitletime_to_subrip_time(subtitle.get_start().str()) 
			<< " --> " 
			<< subtitletime_to_subrip_time(subtitle.get_end().str()) 
			<< get_newline();

		text = subtitle.get_text();

		// convertir le \n selon le format de sortie
		if(change_newline)
			newline_to_characters(text, get_newline());

		file << utf8_to_charset(text) << get_newline() << get_newline();
	}
	
	file.close();
	return true;
}


Glib::ustring SubtitleSubRip::subtitletime_to_subrip_time(const Glib::ustring &time)
{
	SubtitleTime t(time);

	char *tmp = g_strdup_printf("%02i:%02i:%02i,%03i",
			t.hours, t.mins, t.secs, t.msecs);

	Glib::ustring str(tmp);

	g_free(tmp);

	return str;
}

