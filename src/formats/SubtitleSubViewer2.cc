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


#include "SubtitleSubViewer2.h"
#include <iostream>
#include <fstream>
#include <glibmm/ustring.h>
#include "utility.h"
#include "RegEx.h"

/*
 *
 */
Glib::ustring SubtitleSubViewer2::get_name()
{
	return "SubViewer 2.0";
}

/*
 *
 */
Glib::ustring SubtitleSubViewer2::get_extension()
{
	return "sub";
}

/*
 *
 */
bool SubtitleSubViewer2::check(const std::string &line)
{
	static RegEx ex(
			"^\\d{2}:\\d{2}:\\d{2}.\\d+,\\d{2}:\\d{2}:\\d{2}.\\d+\\s*$");

	return ex.exec(line);
}

/*
 *
 */
SubtitleSubViewer2::SubtitleSubViewer2(Document* doc)
:SubtitleFormat(doc, get_name())
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *
 */
SubtitleSubViewer2::~SubtitleSubViewer2()
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}


/*
 *
 */
bool SubtitleSubViewer2::on_open(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_LOADER);

	std::ifstream file(filename.c_str());
	if(!file)
	{
		throw SubtitleException("SubtitleSubViewer2", _("I can't open this file."));
	}

	std::string line;

	Subtitles subtitles = document()->subtitles();

	while(!file.eof() && std::getline(file, line))
	{
		// lecture du temps
		if(std::getline(file, line))
		{
			int start[4], end[4];

			if(sscanf(line.c_str(), "%d:%d:%d.%d,%d:%d:%d.%d",
						&start[0], &start[1], &start[2], &start[3], 
						&end[0], &end[1], &end[2], &end[3]) == 8)
			{
				if(std::getline(file, line))
				{
					Glib::ustring text = check_end_char(charset_to_utf8(line));

					Subtitle subtitle = subtitles.append();

					characters_to_newline(text, "[br]");

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
bool SubtitleSubViewer2::on_save(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_SAVER);

	std::ofstream file(filename.c_str());
	if(!file)
		throw SubtitleException("SubtitleSubViewer2", _("I can't open this file."));

	file << "[INFORMATION]" << get_newline();
	file << "[TITLE]" << utf8_to_charset(m_scriptInfo->Title) << get_newline();
	file << "[AUTHOR]" << utf8_to_charset(m_scriptInfo->OriginalEditing) << get_newline();
	file << "[COMMENT]" << utf8_to_charset(m_scriptInfo->Comment) << get_newline();
	file << "[END INFORMATION]" << get_newline();
	file << "[SUBTITLE]" << get_newline();

	SubtitleColumnRecorder column;
	Glib::ustring text;
	
	for(Subtitle subtitle = document()->subtitles().get_first(); subtitle; ++subtitle)
	{
		SubtitleTime start = subtitle.get_start();
		SubtitleTime end = subtitle.get_end();

		gchar *tmp = g_strdup_printf("%.2i:%.2i:%.2i.%.2i,%.2i:%.2i:%.2i.%.2i",
				start.hours(), start.minutes(), start.seconds(), start.mseconds(),
				end.hours(), end.minutes(), end.seconds(), end.mseconds());

		file << tmp << get_newline();

		g_free(tmp);

		text = subtitle.get_text();

		newline_to_characters(text, "[br]");

		file << utf8_to_charset(text) << get_newline();
		file << get_newline();
	}
	
	file.close();
	return true;
}
