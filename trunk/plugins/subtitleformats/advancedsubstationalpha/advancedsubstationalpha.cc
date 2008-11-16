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

#include <extension/subtitleformat.h>
#include <utility.h>
#include <iomanip>
#include <document.h>

/*
 *
 */
class AdvancedSubStationAlpha : public SubtitleFormatIO
{
	int m_line_break_policy;
public:

	AdvancedSubStationAlpha()
	:m_line_break_policy(3)
	{
		read_config_line_break_policy();
	}

	/*
	 * soft:1
	 * hard:2
	 * intelligent:3 (default)
	 */
	void read_config_line_break_policy()
	{
		if(Config::getInstance().has_key("AdvancedSubStationAlpha", "line-break-policy") == false)
		{
			Config::getInstance().set_value_string(
				"AdvancedSubStationAlpha", 
				"line-break-policy",
				"intelligent",
				"determine the policy of the line break, 3 options: 'soft', 'hard' or 'intelligent' "
				"(without quote, the default value is 'intelligent')");
		}

		Glib::ustring policy = Config::getInstance().get_value_string("AdvancedSubStationAlpha", "line-break-policy");
		if(policy == "soft")
			m_line_break_policy = 1;
		else if(policy == "hard")
			m_line_break_policy = 2;
		else if(policy == "intelligent")
			m_line_break_policy = 3;
		else
		{
			Config::getInstance().set_value_string(
				"AdvancedSubStationAlpha", 
				"line-break-policy",
				"intelligent",
				"determine the policy of the line break, 3 options: 'soft', 'hard' or 'intelligent' "
				"(without quote, the default value is 'intelligent')");
			m_line_break_policy = 3;
		}
	}

	/*
	 *
	 */
	void open(FileReader &file)
	{
		Glib::ustring line;

		while(file.getline(line))
		{
			if(line.find("[Script Info]") != Glib::ustring::npos)
				read_script_info(file);
			else if(line.find("[V4+ Styles]") != Glib::ustring::npos)
				read_styles(file);
			else if(line.find("[Events]") != Glib::ustring::npos)
				read_events(file);
		}
	}

	/*
	 *
	 */
	void save(FileWriter &file)
	{
		write_script_info(file);

		write_styles(file);

		write_events(file);
	}

	/*
	 * Read the block [Script Info]
	 */
	void read_script_info(FileReader &file)
	{
		se_debug_message(SE_DEBUG_IO, "read script info...");

		ScriptInfo &script_info = document()->get_script_info();
		
		Glib::RefPtr<Glib::Regex> re = Glib::Regex::create(
				"^(.*?):\\s(.*?)$");

		Glib::ustring line;
		while(file.getline(line) && !line.empty())
		{
			std::vector<Glib::ustring> group = re->split(line);

			if(group.size() == 1)
				continue;

			Glib::ustring key = group[1];
			Glib::ustring value = group[2];

			script_info.data[key]=value;
		}	
	}

	/*
	 * Read the block [V4+ Styles]
	 */
	void read_styles(FileReader &file)
	{
		se_debug_message(SE_DEBUG_IO, "read style...");

		Styles styles = document()->styles();

		Glib::RefPtr<Glib::Regex> re = Glib::Regex::create(
				"^Style:\\s*"
				"([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),"
				"([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),"
				"([^,]*),([^,]*),([^,]*)$");

		Glib::ustring line;
		while(file.getline(line) && !line.empty())
		{
			std::vector<Glib::ustring> group = re->split(line);

			if(group.size() == 1)
				continue;

			Style style = styles.append();
			
			style.set("name", group[1]);

			style.set("font-name", group[2]);
			style.set("font-size", group[3]);
		
			style.set("primary-color", from_ass_color(group[4]));
			style.set("secondary-color", from_ass_color(group[5]));
			style.set("outline-color", from_ass_color(group[6]));
			style.set("shadow-color", from_ass_color(group[7]));

			style.set("bold", from_ass_bool(group[8]));
			style.set("italic", from_ass_bool(group[9]));
			style.set("underline", from_ass_bool(group[10]));
			style.set("strikeout", from_ass_bool(group[11]));

			style.set("scale-x", group[12]);
			style.set("scale-y", group[13]);

			style.set("spacing", group[14]);
			style.set("angle", group[15]);

			style.set("border-style", group[16]);
			style.set("outline", group[17]);
			style.set("shadow", group[18]);

			style.set("alignment", group[19]);

			style.set("margin-l", group[20]);
			style.set("margin-r", group[21]);
			style.set("margin-v", group[22]);

			style.set("encoding", group[23]);	
		}
	}

	/*
	 * Read the block [Events]
	 */
	void read_events(FileReader &file)
	{
		se_debug_message(SE_DEBUG_IO, "read events...");

		Subtitles subtitles = document()->subtitles();

		Glib::RefPtr<Glib::Regex> re = Glib::Regex::create(
				"^Dialogue:\\s*([^,]*),([^,]*),([^,]*),\\**([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),(.*)$");

		Glib::ustring line;
		while(file.getline(line) && !line.empty())
		{
			std::vector<Glib::ustring> group = re->split(line);

			if(group.size() == 1)
				continue;

			Subtitle sub = subtitles.append();

			// start, end times
			sub.set_start_and_end(
					from_ass_time(group[2]),
					from_ass_time(group[3]));

			// style
			sub.set_style(group[4]);

			// name
			sub.set_name(group[5]);
			
			// margin lrv
			sub.set_margin_l(group[6]);
			sub.set_margin_r(group[7]);
			sub.set_margin_v(group[8]);
		
			// effect
			sub.set_effect( (group[9]));

			// text
			utility::replace(group[10], "\\n", "\n");
			utility::replace(group[10], "\\N", "\n");

			sub.set_text(group[10]);
		}
	}


	/*
	 * Write the block [Script Info]
	 */
	void write_script_info(FileWriter &file)
	{
		file.write(
			Glib::ustring::compose(
				"[Script Info]\n"
				"; This script was created by subtitleeditor (%1)\n"
				"; http://home.gna.org/subtitleeditor/\n",
				Glib::ustring(VERSION)));

		ScriptInfo& scriptInfo = document()->get_script_info();

		scriptInfo.data["ScriptType"] = "V4.00+"; // Set ASS format

		for(std::map<Glib::ustring, Glib::ustring>::const_iterator it = scriptInfo.data.begin();
				it != scriptInfo.data.end(); 
				++it)
		{
			file.write(it->first + ": " + it->second + "\n");
		}

		// End of block, empty line
		file.write("\n");
	}

	/*
	 * Write the block [V4+ Styles]
	 */
	void write_styles(FileWriter &file)
	{
		file.write("[V4+ Styles]\n");
		file.write(
				"Format: "
				"Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, "
				"BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, "
				"BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n");

		// Default style if it's empty
		if(document()->styles().size() == 0)
		{
			// write without changing the document
			Glib::ustring default_style = "Default,Sans,18,&H00FFFFFF,&H0000FFFF,&H000078B4,&H00000000,0,0,0,0,100,100,0,0,1,0,0,2,20,20,20,0";
			file.write("Style: " + default_style + "\n");
			
			//Style style = document()->styles().append();
			//style.set("name", "Default");
		}

		for(Style style = document()->styles().first(); style; ++style)
		{
			file.write( Glib::ustring::compose("Style: %1,%2,%3,%4,%5,%6,%7\n",
				Glib::ustring::compose("%1,%2,%3",
					style.get("name"),
					style.get("font-name"),
					style.get("font-size")),
				
				Glib::ustring::compose("%1,%2,%3,%4",
					to_ass_color(style.get("primary-color")),
					to_ass_color(style.get("secondary-color")),
					to_ass_color(style.get("outline-color")),
					to_ass_color(style.get("shadow-color"))),
				
				Glib::ustring::compose("%1,%2,%3,%4",
					to_ass_bool(style.get("bold")),
					to_ass_bool(style.get("italic")),
					to_ass_bool(style.get("underline")),
					to_ass_bool(style.get("strikeout"))),
				
				Glib::ustring::compose("%1,%2,%3,%4",
					style.get("scale-x") ,
					style.get("scale-y"),
					style.get("spacing"),
					style.get("angle")),
				
				Glib::ustring::compose("%1,%2,%3,%4",
					style.get("border-style"),
					style.get("outline"),
					style.get("shadow"),
					style.get("alignment")),
				
				Glib::ustring::compose("%1,%2,%3",
					style.get("margin-l"),
					style.get("margin-r"),
					style.get("margin-v")),
				
					style.get("encoding")));
		}

		// End of block, empty line
		file.write("\n");
	}

	/*
	 * Write the block [Events]
	 */
	void write_events(FileWriter &file)
	{
		file.write("[Events]\n");
		// format:
		file.write("Format: Layer, Start, End, Style, Actor, MarginL, MarginR, MarginV, Effect, Text\n");

		Glib::RefPtr<Glib::Regex> re_intelligent_linebreak = Glib::Regex::create(
				"\n(?=-\\s.*)", Glib::REGEX_MULTILINE);

		// line break policy: 1 = soft, 2=hard, 3=intelligent

		for(Subtitle sub = document()->subtitles().get_first(); sub; ++sub)
		{
			Glib::ustring text = sub.get_text();

			if(m_line_break_policy == 1)
				utility::replace(text, "\n", "\\n");
			else if(m_line_break_policy == 2)
				utility::replace(text, "\n", "\\N");
			else if(m_line_break_policy == 3)
			{
				if(re_intelligent_linebreak->match(text))
					utility::replace(text, "\n", "\\N");
				else
					utility::replace(text, "\n", "\\n");
			}

			file.write(
				Glib::ustring::compose(
					"Dialogue: %1,%2,%3,%4,%5,%6,%7,%8\n",
					sub.get_layer(), 
					to_ass_time(sub.get_start()), 
					to_ass_time(sub.get_end()), 
					sub.get_style(), 
					sub.get_name(), 
					Glib::ustring::compose("%1,%2,%3", 
						Glib::ustring::format( std::setw(4), std::setfill(L'0'), sub.get_margin_l()),
						Glib::ustring::format( std::setw(4), std::setfill(L'0'), sub.get_margin_r()),
						Glib::ustring::format( std::setw(4), std::setfill(L'0'), sub.get_margin_v())),
					sub.get_effect(),
					text));
		}

		// End of block, empty line
		//file << std::endl;
	}


	/*
	 * Convert time from SE to ASS
	 */
	Glib::ustring to_ass_time(const SubtitleTime &time)
	{
		return build_message("%01i:%02i:%02i.%02i",
			time.hours(), time.minutes(), time.seconds(), (int)((time.mseconds() + 0.5) / 10));
	}

	/*
	 * Convert time from ASS to SE
	 */
	SubtitleTime from_ass_time(const Glib::ustring &t)
	{
		int h, m, s, ms;
		if(std::sscanf(t.c_str(), "%d:%d:%d.%d", &h, &m, &s, &ms) == 4)
			return SubtitleTime(h, m, s, ms * 10);

		return SubtitleTime::null();
	}

	/*
	 * Convert bool from SE to ASS
	 * ASS: false == 0, true == -1
	 */
	Glib::ustring to_ass_bool(const Glib::ustring &value)
	{
		return (value == "0") ? "0" : "-1";
	}

	/*
	 * Convert bool from ASS to SE
	 * ASS: 0 == false, -1 == true
	 */
	Glib::ustring from_ass_bool(const Glib::ustring &value)
	{
		return (value == "0") ? "0" : "1";
	}

	/*
	 * Convert color from SE to ASS
	 */
	Glib::ustring to_ass_color(const Color &color)
	{
		Color c(color);

		unsigned int r = c.getR();
		unsigned int g = c.getG();
		unsigned int b = c.getB();
		unsigned int a = 255 - c.getA();

		unsigned int abgr = a << 24 | b << 16 | g << 8 | r << 0;

		return build_message("&H%08X", abgr);
	}

	/*
	 * Convert color from ASS to SE
	 */
	Glib::ustring from_ass_color(const Glib::ustring &str)
	{
		try
		{
			Glib::ustring value = str;

			if(value.size() > 2)
			{
				if(value[0] == '&')
					value.erase(0,1);
				if(value[0] == 'h' || value[0] == 'H')
					value.erase(0,1);
				if(value[value.size() ] == '&')
					value.erase(value.size() -1, 1);
			}

			long temp[4] = {0,0,0,0};

			for(int i=0; i<4; ++i)
			{
				if(value.size() > 0)
				{
					Glib::ustring tmp = value.substr(value.size() - 2, 2);

					temp[i] = strtoll(tmp.c_str(), NULL, 16);

					value = value.substr(0, value.size() -2);
				}
			}
			return Color(temp[0], temp[1], temp[2], 255 - temp[3]).to_string();
		}
		catch(...)
		{
		}

		return Color(255,255,255,255).to_string();
	}

	/*
	 * Convert time from SE to SSA
	 */
	Glib::ustring to_ssa_time(const SubtitleTime &t)
	{
		return build_message(
							"%01i:%02i:%02i.%02i",
							t.hours(), t.minutes(), t.seconds(), (t.mseconds()+5)/10);
	}
};

class AdvancedSubStationAlphaPlugin : public SubtitleFormat
{
public:

	/*
	 *
	 */
	SubtitleFormatInfo get_info()
	{
		SubtitleFormatInfo info;
		info.name = "Advanced Sub Station Alpha";
		info.extension = "ass";
		info.pattern = "^ScriptType:\\s*[vV]4.00\\+$";
		
		return info;
	}

	/*
	 *
	 */
	virtual SubtitleFormatIO* create()
	{
		AdvancedSubStationAlpha *sf = new AdvancedSubStationAlpha();
		return sf;
	}
};

REGISTER_EXTENSION(AdvancedSubStationAlphaPlugin)
