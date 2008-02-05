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


#include "SubtitleText.h"
#include <iostream>
#include <fstream>
#include "utility.h"
#include "RegEx.h"

/*
 *
 */
Glib::ustring SubtitleText::get_name()
{
	return "Text";
}

/*
 *
 */
Glib::ustring SubtitleText::get_extension()
{
	return "text";
}

/*
 *
 */
bool SubtitleText::check(const std::string &line)
{
	static RegEx ex("^.*?$");

	return ex.exec(line);
}

/*
 *
 */
SubtitleText::SubtitleText(Document *doc)
:SubtitleFormat(doc, get_name())
{
}

/*
 *
 */
SubtitleText::~SubtitleText()
{
}


/*
 *
 */
bool SubtitleText::on_open(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_LOADER);

	std::ifstream file(filename.c_str());
	if(!file)
	{
		return false;
	}

	std::string line;

	Subtitles subtitles = document()->subtitles();

	while(!file.eof() && std::getline(file, line))
	{
		if(!line.empty())
		{
			Subtitle sub = subtitles.append();
			sub.set_text( check_end_char(charset_to_utf8(line)) );
		}
	}
	file.close();

	return true;
}

/*
 *
 */
bool SubtitleText::on_save(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_SAVER);

	std::ofstream file(filename.c_str());
	if(!file)
	{
		return false;
	}

	Glib::ustring line;

	for(Subtitle subtitle = document()->subtitles().get_first(); subtitle; ++subtitle)
	{
		Glib::ustring text = subtitle.get_text();
		newline_to_characters(text, get_newline());

		file << utf8_to_charset(text);
		file << get_newline();
	}
	file.close();

	return true;
}
