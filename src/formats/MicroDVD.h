#ifndef _MicroDVD_h
#define _MicroDVD_h

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
#include "RegEx.h"

/*
 * format:
 *
 * {start_frame}{end_frame}text
 */
class MicroDVD : public SubtitleFormat
{
public:

	/*
	 *
	 */
	static SubtitleFormatInfo get_info()
	{
		SubtitleFormatInfo info;

		info.name = "MicroDVD";
		info.extension = "sub";
		info.pattern = "^\\{\\d+\\}\\{\\d+\\}.*?\\R";
		
		return info;
	}

	/*
	 *
	 */
	void open(FileReader &file)
	{
		RegEx re("^\\{(\\d+)\\}\\{(\\d+)\\}(.*?)$");
		
		// init to frame mode
		document()->set_timing_mode(FRAME);
		document()->set_edit_timing_mode(FRAME);

		Subtitles subtitles = document()->subtitles();

		int frame_start, frame_end;
		Glib::ustring line;
		std::string text;

		while(file.getline(line))
		{
			if(re.FullMatch(line.c_str(), &frame_start, &frame_end, &text))
			{
				utility::replace(text, "|", "\n");
				//FIXME tags.decode(text);

				// Append a subtitle
				Subtitle sub = subtitles.append();

				sub.set_text(text);
				sub.set_start_frame(frame_start);
				sub.set_end_frame(frame_end);
			}
		}
	}

	/*
	 *
	 */
	void save(FileWriter &file)
	{
		for(Subtitle sub = document()->subtitles().get_first(); sub; ++sub)
		{
			Glib::ustring text =sub.get_text();

			//FIXME tags.encode(text);
			utility::replace(text, "\n", "|");

			// {start_frame}{end_frame}text
			file << "{" 
				<< sub.get_start_frame()
				<< "}{"
				<< sub.get_end_frame()
				<< "}"
				<< text
				<< std::endl;
		}
	}
};

#endif//_MicroDVD_h

