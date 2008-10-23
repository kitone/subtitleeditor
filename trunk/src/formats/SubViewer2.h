#ifndef _SubViewer2_h
#define _SubViewer2_h

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

#include "SubtitleFormat.h"


class SubViewer2 : public SubtitleFormat
{
public:

	/*
	 *
	 */
	static SubtitleFormatInfo get_info()
	{
		SubtitleFormatInfo info;
		info.name = "SubViewer 2.0";
		info.extension = "sub";

		info.pattern = "\\d{2}:\\d{2}:\\d{2}.\\d+,\\d{2}:\\d{2}:\\d{2}.\\d+\\s*\\R";
		
		return info;
	}

	/*
	 *
	 */
	void open(FileReader &file)
	{
		Glib::RefPtr<Glib::Regex> re_time = Glib::Regex::create(
				"^(\\d+):(\\d+):(\\d+)\\.(\\d+),(\\d+):(\\d+):(\\d+)\\.(\\d+)");

		Subtitles subtitles = document()->subtitles();

		Glib::ustring line;
		int start[4], end[4];

		while(file.getline(line))
		{
			// Read the subtitle time "start --> end"
			if(!re_time->match(line))
				continue;

			std::vector<Glib::ustring> group = re_time->split(line);
			//if(group.size() == 1)
			//	continue;

			start[0] = utility::string_to_int(group[1]);
			start[1] = utility::string_to_int(group[2]);
			start[2] = utility::string_to_int(group[3]);
			start[3] = utility::string_to_int(group[4]);

			end[0] = utility::string_to_int(group[5]);
			end[1] = utility::string_to_int(group[6]);
			end[2] = utility::string_to_int(group[7]);
			end[3] = utility::string_to_int(group[8]);

			if(file.getline(line))
			{
				utility::replace(line, "[br]", "\n");
						
				// Append a subtitle
				Subtitle sub = subtitles.append();

				sub.set_text(line);
				sub.set_start_and_end(
							SubtitleTime(start[0], start[1], start[2], start[3] * 10),
							SubtitleTime(end[0], end[1], end[2], end[3] *10));
			}
		}
	}

	/*
	 *
	 */
	void save(FileWriter &file)
	{
		ScriptInfo& script = document()->get_script_info();

		file << "[INFORMATION]" << std::endl;
		file << "[TITLE]" << script.data["Title"] << std::endl;
		file << "[AUTHOR]" << script.data["OriginalEditing"] << std::endl;
		file << "[COMMENT]" << script.data["Comment"] << std::endl;
		file << "[END INFORMATION]" << std::endl;
		file << "[SUBTITLE]" << std::endl;

		for(Subtitle sub = document()->subtitles().get_first(); sub; ++sub)
		{
			Glib::ustring text = sub.get_text();

			utility::replace(text, "\n", "[br]");

			file << to_subviewer_time(sub.get_start()) << "," << to_subviewer_time(sub.get_end()) << std::endl;
			file << text << std::endl;
			file << std::endl; // empty line
		}
	}

	/*
	 *
	 */
	Glib::ustring to_subviewer_time(const SubtitleTime &t)
	{
		return build_message(
							"%02i:%02i:%02i.%02i",
							t.hours(), t.minutes(), t.seconds(), (int)((t.mseconds() +5 )/10));
	}
};

#endif//_SubViewer2_h

