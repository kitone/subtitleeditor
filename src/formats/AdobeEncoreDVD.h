#ifndef _AdobeEncoreDVD_h
#define _AdobeEncoreDVD_h

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
 *      PAL/NTSC version.
 *
 *	Adobe Encore DVD text script support by Laurens Keek
 *      Created using following documentation:
 *	http://www.adobe.com/support/techdocs/329569.html
 */

#include "SubtitleFormat.h"
#include "RegEx.h"


class AdobeEncoreDVD : public SubtitleFormat
{
public:

	/*
	 *
	 */
	AdobeEncoreDVD(FRAMERATE framerate)
	:m_framerate(framerate)
	{
		m_framerate_value = get_framerate_value(m_framerate);
	}

	/*
	 *
	 */
	void open(FileReader &file)
	{
		RegEx re("\\d+\\s(\\d+)[:;](\\d+)[:;](\\d+)[:;](\\d+)\\s(\\d+)[:;](\\d+)[:;](\\d+)[:;](\\d+)\\s(.*?)$");
		
		Subtitles subtitles = document()->subtitles();

		int start[4], end[4];
		Glib::ustring line;
		std::string text;


		while(file.getline(line))
		{
			if(re.FullMatch( 
							line.c_str(), 
							&start[0], &start[1], &start[2], &start[3], 
							&end[0], &end[1], &end[2], &end[3],
							&text))
			{
				// last 00 are frame, not time!
				start[3] = start[3] * 1000 / m_framerate_value;
				end[3] = end[3] * 1000 / m_framerate_value;
	
				// Append a subtitle
				Subtitle sub = subtitles.append();

				sub.set_text(text);
				sub.set_start_and_end(
								SubtitleTime(start[0], start[1], start[2], start[3]),
								SubtitleTime(end[0], end[1], end[2], end[3]));
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

			file 
				<< sub.get_num() 
				<< " " 
				<< to_encore_dvd_time(sub.get_start())
				<< " "
				<< to_encore_dvd_time(sub.get_end())
				<< " "
				<< text
				<< std::endl;
		}
	}

	/*
	 * Convert time from SE to Encore DVD
	 * 0:00:00.000 -> 00[:;]00[:;]00[:;]00 (last 00 are frames, not time!)
	 */
	Glib::ustring to_encore_dvd_time(const SubtitleTime &t)
	{
		int frame = (int)(t.mseconds() * m_framerate_value * 0.001);

		return build_message(
							(m_framerate == FRAMERATE_25) ? "%02i:%02i:%02i:%02i" : "%02i;%02i;%02i;%02i" ,
							t.hours(), t.minutes(), t.seconds(), frame);
	}

protected:
	FRAMERATE m_framerate;
	double m_framerate_value;
};

/*
 *
 */
class AdobeEncoreDVDPAL : public AdobeEncoreDVD
{
public:
	AdobeEncoreDVDPAL()
	:AdobeEncoreDVD(FRAMERATE_25)
	{
	}

	/*
	 * First line should simply be:
	 * number start_time stop_time some_text
	 *
	 * 1 00:00:00:1 00:00:10:5 text  (PAL)
	 */
	static SubtitleFormatInfo get_info()
	{
		SubtitleFormatInfo info;

		info.name = "Adobe Encore DVD (PAL)";
		info.extension = "txt";
		info.pattern = "^\\d+\\s(\\d+(:)\\d+\\2\\d+\\2\\d+ ){2}.*?\\R";

		return info;
	}
};

/*
 *
 */
class AdobeEncoreDVDNTSC : public AdobeEncoreDVD
{
public:
	AdobeEncoreDVDNTSC()
	:AdobeEncoreDVD(FRAMERATE_29_97)
	{
	}

	/*
	 * First line should simply be:
	 * number start_time stop_time some_text
	 *
	 * 1 00;00;00;1 00;00;10;5 text	 (NTSC)
	 */
	static SubtitleFormatInfo get_info()
	{
		SubtitleFormatInfo info;

		info.name = "Adobe Encore DVD (NTSC)";
		info.extension = "txt";
		info.pattern = "^\\d+\\s(\\d+(;)\\d+\\2\\d+\\2\\d+ ){2}.*?\\R";

		return info;
	}
};


#endif//_AdobeEncoreDVD_h

