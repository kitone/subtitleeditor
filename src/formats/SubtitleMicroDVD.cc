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


#include "SubtitleMicroDVD.h"
#include <iostream>
#include <fstream>
#include <glibmm/ustring.h>
#include "utility.h"
#include "RegEx.h"
#include "SubtitleTags.h"

/*
 *
 */
class MicroDVDTags : public SubtitleTags
{
	class Tag
	{
	public:
		Tag(const std::string &re, const std::string &by)
		:m_re(re), m_by(by)
		{
		}

		Tag(const RegEx &re, const std::string &by)
		:m_re(re), m_by(by)
		{
		}

		void replace(Glib::ustring &text)
		{
			std::string t = text;

			m_re.GlobalReplace(m_by, &t);

			text = t;
		}

	public:
		RegEx m_re;
		std::string m_by;
	};

public:
	MicroDVDTags()
	:SubtitleTags("MicroDVD")
	{
		// MicroDVD to SE
		m_decode_tags.push_back( Tag("\\{y:(b|i|u)\\}(.*?)$", "<\\1>\\2</\\1>") );
		m_decode_tags.push_back( Tag("\\{Y:(b|i|u)\\}(.*?)$", "<\\1>\\2</\\1>") );
		
		// SE to MicroDVD
		m_encode_tags.push_back( Tag("<(b|i|u)>(.*?)</\\1>", "{y:\\1}\\2") );
		m_encode_tags.push_back( Tag("<(b|i|u)>(.*?)(\n+)(.*)</\1>", "{y:\\1}\\2") );
	}

	/*
	 *
	 */
	bool decode(Glib::ustring &text)
	{
		std::vector<Tag>::iterator it;

		for(it = m_decode_tags.begin(); it != m_decode_tags.end(); ++it)
		{
			(*it).replace(text);
		}

		return true;
	}

	/*
	 *
	 */
	bool encode(Glib::ustring &text)
	{
		std::vector<Tag>::iterator it;

		for(it = m_encode_tags.begin(); it != m_encode_tags.end(); ++it)
		{
			(*it).replace(text);
		}
		return true;
	}

protected:
	std::vector<Tag> m_decode_tags;
	std::vector<Tag> m_encode_tags;
};

/*
 *
 */
Glib::ustring SubtitleMicroDVD::get_name()
{
	return "MicroDVD";
}

/*
 *
 */
Glib::ustring SubtitleMicroDVD::get_extension()
{
	return "sub";
}

/*
 *
 */
bool SubtitleMicroDVD::check(const std::string &line)
{
	static RegEx ex("^\\{\\d+\\}\\{\\d+\\}.*?$");

	return ex.exec(line);
}


/*
 *
 */
SubtitleMicroDVD::SubtitleMicroDVD(Document* doc)
:SubtitleFormat(doc, get_name())
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *
 */
SubtitleMicroDVD::~SubtitleMicroDVD()
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}


/*
 *
 */
bool SubtitleMicroDVD::on_open(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_LOADER);

	std::ifstream file(filename.c_str());

	if(!file)
	{
		throw SubtitleException("SubtitleMicroDVD", _("I can't open this file."));
	}

	RegEx ex("^\\{(\\d+)\\}\\{(\\d+)\\}(.*?)$");

	std::string line;
	std::string text;
	int frame_start, frame_end;
  
	document()->set_edit_timing_mode(FRAME);
	document()->set_timing_mode(FRAME);

	Subtitles subtitles = document()->subtitles();

	MicroDVDTags tags;

	while(!file.eof() && std::getline(file, line))
	{
		// on la passe tout de suite en utf8
		// car regex utilise de l'utf8
		Glib::ustring utf8_line = charset_to_utf8(line);

		if(ex.FullMatch(utf8_line.c_str(), &frame_start, &frame_end, &text))
		{
			Subtitle subtitle = subtitles.append();

			Glib::ustring utf8_text = check_end_char(text);

			characters_to_newline(utf8_text, "|");

			tags.decode(utf8_text);

			subtitle.set_text(utf8_text);

			subtitle.set_start_frame(frame_start);
			subtitle.set_end_frame(frame_end);
		}
	}

	file.close();

	return true;
}

/*
 *
 */
bool SubtitleMicroDVD::on_save(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_SAVER);

	std::ofstream file(filename.c_str());
	if(!file)
	{
		throw SubtitleException("SubtitleMicroDVD", _("I can't open this file."));
	}

	Glib::ustring text;
	
	MicroDVDTags tags;

	for(Subtitle subtitle = document()->subtitles().get_first(); subtitle; ++subtitle)
	{
		Glib::ustring text = subtitle.get_text();

		tags.encode(text);

		newline_to_characters(text, "|");

		file << "{"
			<< subtitle.get_start_frame()
			<< "}{"
			<< subtitle.get_end_frame()
			<< "}"
			<< utf8_to_charset(text)
			<< get_newline();
	}
	
	file.close();

	return true;
}



