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
 

#include "SubtitleMPsub.h"
#include <iostream>
#include <fstream>
#include <glibmm/ustring.h>
#include "utility.h"
#include "RegEx.h"

#include <pcrecpp.h>

/*
 *
 */
Glib::ustring SubtitleMPsub::get_name()
{
	return "MPsub";
}

/*
 *
 */
Glib::ustring SubtitleMPsub::get_extension()
{
	return "sub";
}

/*
 *
 */
bool SubtitleMPsub::check(const std::string &line)
{
	static RegEx ex("^FORMAT=(TIME|[0-9])");	
	return ex.exec(line);
}


/*
 *
 */
SubtitleMPsub::SubtitleMPsub(Document* doc)
:SubtitleFormat(doc, get_name())
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *
 */
SubtitleMPsub::~SubtitleMPsub()
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}


/*
 *
 */
bool SubtitleMPsub::on_open(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_LOADER);

	std::ifstream file(filename.c_str());
	
	//file.imbue(std::locale("C"));

	if(!file)
	{
		throw SubtitleException("SubtitleMPsub", _("I can't open this file."));
	}

	bool use_time;
	
	int d = 0; //FORMAT=%d
	float multiplier = 1.;

	SubtitleTime old_end;

	std::string start, end;

	pcrecpp::RE ex("^(-?\\d+(?:\\.\\d+)?) (-?\\d+(?:\\.\\d+)?)\\s*$", pcrecpp::RE_Options(PCRE_UTF8|PCRE_MULTILINE));

	std::string line;

	Subtitles subtitles = document()->subtitles();

	while(!file.eof() && std::getline(file, line))
	{
		// on la passe tout de suite en utf8
		// car regex utilise de l'utf8
		Glib::ustring utf8_line = charset_to_utf8(line);

		if(ex.FullMatch(utf8_line.c_str(), &start, &end))
		{
			// recupere le temps
			float fs = 0, fe = 0; //start, end

			from_string(start, fs);
			from_string(end, fe);

			// calcul le temps par rapport au sous-titre precedent
			SubtitleTime start_time = old_end + SubtitleTime((long int)(fs * multiplier));
			SubtitleTime end_time = start_time + SubtitleTime((long int)(fe * multiplier));
			
			// recupere le texte
			bool count = false;
			Glib::ustring text;

			while(std::getline(file, line))
			{
				line = check_end_char(charset_to_utf8(line));

				if(line.empty())
					break;
				else
				{
					if(count)
						text += get_newline();

					text += line;
					count = true;
				}
			}


			// ajoute le sous-titre
			Subtitle subtitle = subtitles.append();

			subtitle.set_start_and_end(
					start_time,
					end_time);
			subtitle.set_text(text);

			// pour le prochain sous-titre
			old_end = end_time;
		}
		else if(std::sscanf(line.c_str(), "FORMAT=%d", &d) == 1)
		{
			multiplier = d;
		}
		else if(line.find("FORMAT=TIME") != std::string::npos)
		{
			use_time = true;
			multiplier = 1000;
		}
	}

	file.close();

	return true;
}

/*
 *
 */
bool SubtitleMPsub::on_save(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_SAVER);

	std::ofstream file(filename.c_str());
	if(!file)
	{
		throw SubtitleException("SubtitleMPsub", _("I can't open this file."));
	}

	file << "FORMAT=TIME" << std::endl;
	file << utf8_to_charset("# This script was created by subtitleeditor ") << utf8_to_charset(VERSION) << std::endl;
	file << utf8_to_charset("# http://kitone.free.fr/subtitleeditor/") << std::endl << std::endl;

	
	Glib::ustring text;
	
	SubtitleTime old_end;

	for(Subtitle subtitle = document()->subtitles().get_first(); subtitle; ++subtitle)
	{
		SubtitleTime start = subtitle.get_start();
		SubtitleTime end = subtitle.get_end();

		text = subtitle.get_text();

		// pas besion on utilise déjà \n
		//newline_to_characters(text, "\n");

		double s = (double)((start - old_end).totalmsecs) * 0.001;
		double e = (double)((end - start).totalmsecs) * .001;

		file << s << " " << e << std::endl;
		file << utf8_to_charset(text) << std::endl;
		file << std::endl;

		old_end = end;
	}
	
	file.close();

	return true;
}




