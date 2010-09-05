/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2010, kitone
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

#include <memory>
#include <extension/subtitleformat.h>
#include <gtkmm_utility.h>
#include <gui/dialogutility.h>
#include <utility.h>
#include <subtitleeditorwindow.h>
#include <player.h>

/*
 * BITC (Burnt-in timecode)
 */
class BITC : public SubtitleFormatIO
{
public:

	/*
	 */
	BITC()
	:m_framerate(FRAMERATE_23_976)
	{
		m_framerate_value = get_framerate_value(m_framerate);
	}

	/*
	 */
	void open(FileReader &file)
	{
		// Ask for the framerate value
		FramerateChooserDialog fcd(FramerateChooserDialog::IMPORT);

		// Define the default value of the framerate from the player
		Player* player = SubtitleEditorWindow::get_instance()->get_player();
		if(player->get_state() != Player::NONE)
		{
			float player_framerate = player->get_framerate();
			if(player_framerate > 0)
				fcd.set_default_framerate(get_framerate_from_value(player_framerate));
		}
		FRAMERATE framerate = fcd.execute();
		m_framerate_value = get_framerate_value(framerate);

		document()->set_framerate(framerate);

		Glib::RefPtr<Glib::Regex> re_time = Glib::Regex::create(
				"^(\\d+):(\\d+):(\\d+):(\\d+)\\s(\\d+):(\\d+):(\\d+):(\\d+)$");
		
		Subtitles subtitles = document()->subtitles();

		int start[4], end[4];
		Glib::ustring line;
		Glib::ustring text;

		Subtitle sub;

		while(file.getline(line))
		{
			if(re_time->match(line))
			{
				std::vector<Glib::ustring> group = re_time->split(line);

				start[0] = utility::string_to_int(group[1]);
				start[1] = utility::string_to_int(group[2]);
				start[2] = utility::string_to_int(group[3]);
				start[3] = utility::string_to_int(group[4]);

				end[0] = utility::string_to_int(group[5]);
				end[1] = utility::string_to_int(group[6]);
				end[2] = utility::string_to_int(group[7]);
				end[3] = utility::string_to_int(group[8]);

				file.getline(text);
				// Replace '|' by a newline
				utility::replace(text, "|", "\n");
	
				// last 00 are frame, not time!
				start[3] = start[3] * 1000 / m_framerate_value;
				end[3] = end[3] * 1000 / m_framerate_value;

				// Append a subtitle
				sub = subtitles.append();

				sub.set_text(text);
				sub.set_start_and_end(
								SubtitleTime(start[0], start[1], start[2], start[3]),
								SubtitleTime(end[0], end[1], end[2], end[3]));
			}
		}
	}

	/*
	 */
	void save(FileWriter &file)
	{
		// Ask for the framerate value
		FramerateChooserDialog fcd(FramerateChooserDialog::EXPORT);
		fcd.set_default_framerate(document()->get_framerate());

		m_framerate_value = get_framerate_value(fcd.execute());

		for(Subtitle sub = document()->subtitles().get_first(); sub; ++sub)
		{
			Glib::ustring text =sub.get_text();

			utility::replace(text, "\n", "|");

			file.write(
					Glib::ustring::compose(
						"%1 %2\n%3\n\n",
						to_bitc_time(sub.get_start()),
						to_bitc_time(sub.get_end()),
						text));
		}
	}

	/*
	 * Convert time from SE to BITC
	 * 0:00:00.000 -> 00:00:00:00 (last 00 are frames, not time!)
	 */
	Glib::ustring to_bitc_time(const SubtitleTime &t)
	{
		int frame = (int)(t.mseconds() * m_framerate_value * 0.001);

		return build_message("%02i:%02i:%02i:%02i", t.hours(), t.minutes(), t.seconds(), frame);
	}

protected:
	FRAMERATE m_framerate;
	double m_framerate_value;
};

/*
 *
 */
class BITCPlugin : public SubtitleFormat
{
public:

	/*
	 */
	SubtitleFormatInfo get_info()
	{
		SubtitleFormatInfo info;

		info.name = "BITC (Burnt-in timecode)";
		info.extension = "txt";
		info.pattern = 
			"\\d+:\\d+:\\d+:\\d+\\s\\d+:\\d+:\\d+:\\d+\\R"
			".*\\R";
		return info;
	}

	/*
	 */
	SubtitleFormatIO* create()
	{
		return new BITC;
	}
};

REGISTER_EXTENSION(BITCPlugin)
