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
 


#include "SubtitleASS.h"
#include "Color.h"
#include "utility.h"

#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "RegEx.h"

/*
 *
 */
Glib::ustring SubtitleASS::get_name()
{
	return "Advanced Sub Station Alpha";
}

/*
 *
 */
Glib::ustring SubtitleASS::get_extension()
{
	return "ass";
}

/*
 *
 */
bool SubtitleASS::check(const std::string &line)
{
	static RegEx ex("^ScriptType:\\s+[vV]4.00\\+\\s*$");

	return ex.exec(line);

}


/*
 *	hack !
 */
Glib::ustring SubtitleASS::clean_style_name(const Glib::ustring &name)
{
	Glib::ustring str = name;
	Glib::ustring::size_type n = str.find('*');
	if(n != Glib::ustring::npos)
		str.erase(n,1);

	return str;
}


/*
 *	recupere dans un tableau le format a partir de la line
 */
std::vector<std::string> SubtitleASS::build_format(const std::string &text, int column, bool remove_space)
{
	std::vector<std::string> buf;

	utility::split(text, ',', buf, column);

	if(remove_space)
	{
		for(unsigned int i=0; i<buf.size(); ++i)
		{
			//Str.erase(
      //  std::remove( Str.begin(), Str.end(), C ),
      //  Str.end() ); 

			buf[i].erase(
					std::remove(buf[i].begin(), buf[i].end(), ' '), 
					buf[i].end());
		}
	}

	//for(unsigned int i=0; i<buf.size(); ++i)
	//	std::cout << "[" << buf[i] << "]" << std::endl;
	return buf;
	/*
	std::string line;

	if(remove_space)
	{
		for(unsigned int i=7; i<text.size(); ++i)
		{
			if(text[i]==' ')
				;
			else
				line+=text[i];
		}
	}
	else
		line = text;

	std::vector<std::string> array;

	std::string::size_type s=0, e=0;

	int i=0;
	do
	{
		if(column > 0 && i+1 == column)
		{
			array.push_back(line.substr(s,std::string::npos));
			break;
		}
		e = line.find(",", s);
		
		array.push_back(line.substr(s,e-s));

		s=e+1;

		++i;
	}while(e != std::string::npos);

	return array;
	*/
}






/*
 *	construtor
 */
SubtitleASS::SubtitleASS(Document* doc)
:SubtitleFormat(doc, get_name())
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *
 */
SubtitleASS::~SubtitleASS()
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *	read subtitle file
 */
bool SubtitleASS::on_open(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_LOADER);
	
	std::ifstream file(filename.c_str());
	
	if(!file)
	{
		throw SubtitleException("SubtitleASS", _("I can't open this file."));
	}

	std::string line;

	
	enum TYPE
	{
		UNKNOWN = 0,
		SCRIPT_INFO,
		STYLES,
		EVENTS
	};

	TYPE type = UNKNOWN;

	while(!file.eof() && std::getline(file, line))
	{
		if(line.find("[Script Info]") != std::string::npos)
			type = SCRIPT_INFO;
		else if(line.find("[V4+ Styles]") != std::string::npos)
			type = STYLES;
		else if(line.find("[Events]") != std::string::npos)
			type = EVENTS;

		switch(type)
		{
		case UNKNOWN:
			break;
		case SCRIPT_INFO: readScripInfo(line);
			break;
		case STYLES: readStyles(line);
			break;
		case EVENTS: readEvents(line);
			break;
		}
	}

	file.close();

	return true;
}

/*
 *	Sauvegarde
 */
bool SubtitleASS::on_save(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_SAVER);

	std::ofstream file(filename.c_str());

	if(!file)
	{
		throw SubtitleException("SubtitleASS", _("I can't open this file."));
	}

	se_debug_message(SE_DEBUG_SAVER, "save ScriptInfo");
	
	// ScriptInfo
	{
		file << utf8_to_charset("[Script Info]") << get_newline();
		file << utf8_to_charset("; This script was created by subtitleeditor ") << utf8_to_charset(VERSION) << get_newline();
		file << utf8_to_charset("; http://home.gna.org/subtitleeditor/") << get_newline();
	
#define CHECK(label, key) if(m_scriptInfo->key.size() > 0) { file << utf8_to_charset(label) << utf8_to_charset(m_scriptInfo->key) << get_newline(); }

		m_scriptInfo->ScriptType="V4.00+";

		CHECK("Title: ", Title);
		CHECK("Original Script: ", OriginalScript);
		CHECK("Original Translation: ", OriginalTranslation);
		CHECK("Original Editing: ", OriginalEditing);
		CHECK("Synch Point: ", SynchPoint);
		CHECK("Script Updated By: ", ScriptUpdatedBy);
		CHECK("Update Details: ", UpdateDetails);
		CHECK("ScriptType: ", ScriptType);
		CHECK("Collisions: ", Collisions);
		CHECK("PlayResX: ", PlayResX);
		CHECK("PlayResY: ", PlayResY);
		CHECK("Timer: ", Timer);
		CHECK("WrapStyle: ", WrapStyle);
		CHECK("Dialogue: ", Dialogue);
		CHECK("Comment: ", Comment);
		CHECK("Picture: ", Picture);
		CHECK("Sound: ", Sound);
		CHECK("Movie: ", Movie);
		
#undef CHECK

		file << get_newline();
	}

	
	se_debug_message(SE_DEBUG_SAVER, "save Style");

	// Style
	{
		StyleColumnRecorder column;
		
		file << "[V4+ Styles]" << get_newline();

		file << "Format: "
				<< "Name, "
				<< "Fontname, "
				<< "Fontsize, "
				
				<< "PrimaryColour, "
				<< "SecondaryColour, "
				<< "OutlineColour, "
				<< "BackColour, "
				
				<< "Bold, "
				<< "Italic, "

				<< "Underline, "
				<< "StrikeOut, "
				<< "ScaleX, "
				<< "ScaleY, "
				<< "Spacing, "
				<< "Angle, "

				
				<< "BorderStyle, "
				<< "Outline, "
				<< "Shadow, "
				<< "Alignment, "
				<< "MarginL, "
				<< "MarginR, "
				<< "MarginV, "
				<< "Encoding" << get_newline();

		Gtk::TreeNodeChildren rows = m_styleModel->children();
		
		//for(Gtk::TreeIter it = rows.begin(); it; ++it)
		for(Style style = document()->styles().first(); style; ++style)
		{
			/*
			std::ostringstream oss;

			oss << "Style: " 
				<< (*it)[column.name] << "," 
				<< (*it)[column.font_name] << "," 
				<< (*it)[column.font_size] << "," 

				<< color_to_ass_color((*it)[column.primary_colour]) << ","
				<< color_to_ass_color((*it)[column.secondary_colour]) << ","
				<< color_to_ass_color((*it)[column.outline_colour]) << ","
				<< color_to_ass_color((*it)[column.shadow_colour]) << ","

				<< bool_to_string((*it)[column.bold]) << "," 
				<< bool_to_string((*it)[column.italic]) << "," 
				<< bool_to_string((*it)[column.underline]) << "," 
				<< bool_to_string((*it)[column.strikeout]) << "," 
				
				<< (*it)[column.scale_x] << "," 
				<< (*it)[column.scale_y] << "," 
				<< (*it)[column.spacing] << "," 
				<< (*it)[column.angle] << "," 
				
				<< (*it)[column.border_style] << "," 
				<< (*it)[column.outline] << "," 
				<< (*it)[column.shadow] << "," 
				<< (*it)[column.alignment] << "," 
				<< (*it)[column.margin_l] << "," 
				<< (*it)[column.margin_r] << "," 
				<< (*it)[column.margin_v] << "," 
				<< (*it)[column.encoding] << get_newline(); 
				
			file << utf8_to_charset(oss.str());
			*/
			file << "Style: " 
				<< style.get("name") << ","
				<< style.get("font-name") << ","
				<< style.get("font-size") << ","
				
				<< color_to_ass_color(style.get("primary-color")) << ","
				<< color_to_ass_color(style.get("secondary-color")) << ","
				<< color_to_ass_color(style.get("outline-color")) << ","
				<< color_to_ass_color(style.get("shadow-color")) << ","
				
				<< bool_to_string(style.get("bold")) << ","
				<< bool_to_string(style.get("italic")) << ","
				<< bool_to_string(style.get("underline")) << ","
				<< bool_to_string(style.get("strikeout")) << ","
				
				<< style.get("scale-x") << ","
				<< style.get("scale-y") << ","
				<< style.get("spacing") << ","
				<< style.get("angle") << ","
				
				<< style.get("border-style") << ","
				<< style.get("outline") << ","
				<< style.get("shadow") << ","
				<< style.get("alignment") << ","
				
				<< style.get("margin-l") << ","
				<< style.get("margin-r") << ","
				<< style.get("margin-v") << ","
				
				<< style.get("encoding") 
				<< get_newline();
		}

		file << get_newline();
	}

	se_debug_message(SE_DEBUG_SAVER, "save Event");

	// Event
	{
		file << "[Events]" << get_newline();
		// format:
		file << 
			"Format: " <<
			"Layer, " <<
			"Start, " <<
			"End, " <<
			"Style, " <<
			"Actor, " <<
			"MarginL, " <<
			"MarginR, " <<
			"MarginV, " <<
			"Effect, " <<
			"Text" << get_newline();
			
		// dialog:
		
		for(Subtitle subtitle = document()->subtitles().get_first(); subtitle; ++subtitle)
		{
			std::ostringstream oss;

			Glib::ustring text = subtitle.get_text();
			newline_to_characters(text, "\\n");

			oss << "Dialogue: "
				<< subtitle.get_layer() << "," 
				<< subtitletime_to_ass_time(subtitle.get_start()) << ","
				<< subtitletime_to_ass_time(subtitle.get_end()) << ","
				<< utf8_to_charset(subtitle.get_style()) << ","
				<< utf8_to_charset(subtitle.get_name()) << ","
				<< std::setw(4) << std::setfill('0') << subtitle.get_margin_l() << ","
				<< std::setw(4) << std::setfill('0') << subtitle.get_margin_r() << ","
				<< std::setw(4) << std::setfill('0') << subtitle.get_margin_v() << ","
				<< utf8_to_charset(subtitle.get_effect()) << ","
				<< utf8_to_charset(text) << get_newline();

			file << oss.str();
		}
	}
	
	file.close();

	return true;
}



/*
 *	READ BLOCK
 */


/*
 *	lecture du block [ScriptInfo]
 */
bool SubtitleASS::readScripInfo(const std::string &_line)
{
	se_debug_message(SE_DEBUG_LOADER, "read ScriptInfo");

	std::string line = check_end_char(_line);

#define CHECK(label, key) \
	if(line.find(label) != std::string::npos) \
	{	line.erase(0, (std::string(label)).size()); m_scriptInfo->key = check_end_char(charset_to_utf8(line)); return true; }
		//else std::cerr << "CHECK not found: " << label << std::endl;

	CHECK("Title: ", Title);
	CHECK("Original Script: ", OriginalScript);
	CHECK("Original Translation: ", OriginalTranslation);
	CHECK("Original Editing: ", OriginalEditing);
	CHECK("Synch Point: ", SynchPoint);
	CHECK("Script Updated By: ", ScriptUpdatedBy);
	CHECK("Update Details: ", UpdateDetails);
	CHECK("ScriptType: ", ScriptType);
	CHECK("Collisions: ", Collisions);
	CHECK("PlayResX: ", PlayResX);
	CHECK("PlayResY: ", PlayResY);
	CHECK("Timer: ", Timer);
	CHECK("WrapStyle: ", WrapStyle);
	CHECK("Dialogue: ", Dialogue);
	CHECK("Comment: ", Comment);
	CHECK("Picture: ", Picture);
	CHECK("Sound: ", Sound);
	CHECK("Movie: ", Movie);
#undef CHECK
	
	return false;
}

/*
 *	lecture du block [V4+ Styles]
 */
bool SubtitleASS::readStyles(const std::string &_line)
{
	se_debug_message(SE_DEBUG_LOADER, "read Style");
	
	std::string line = check_end_char(_line);

	if(line.find("Style: ") != std::string::npos)
	{		
		StyleColumnRecorder column;
		// on donne la ligne sans "Style: " = size - 7
		std::string data = line;
		data.erase(0, 7);
			
		std::vector<std::string> fmt = build_format(data, formats.size(), false);

		Gtk::TreeIter it = m_styleModel->append();

		// on supprime '*' si il existe
		(*it)[column.name]							= clean_style_name( charset_to_utf8( fmt[ map["Name"] ] ) );
			
		(*it)[column.font_name]					= fmt[ map["Fontname"] ];
		(*it)[column.font_size]					= string_to_int( fmt[ map["Fontsize"] ] );

		// color
		(*it)[column.primary_colour] = ass_color_to_color( fmt[ map["PrimaryColour"] ]).to_string();
		(*it)[column.secondary_colour] = ass_color_to_color( fmt[ map["SecondaryColour"] ]).to_string();
		(*it)[column.outline_colour] = ass_color_to_color( fmt[ map["OutlineColour"] ]).to_string();
		(*it)[column.shadow_colour] = ass_color_to_color( fmt[ map["BackColour"] ]).to_string();
	
#define INIT_TO_BOOL(col, col_text) \
		{ \
			unsigned int id = map[col_text]; \
			std::string str = fmt[id]; \
			bool val = string_to_bool(str); \
			(*it)[column.col] = val; \
			se_debug_message(SE_DEBUG_LOADER, "id=[%d] column=[%s] string=[%s] result=[%d] error=[%s]", \
					id, col_text, str.c_str(), val, (map.find(col_text) == map.end()) ? "Column Not Found" : "NONE"); \
		}

#define INIT_TO_INT(col, col_text) \
		{ \
			if(map.find(col_text) == map.end()) std::cerr << "## Column Not Found:" << col_text << std::endl; \
			unsigned int id = map[col_text]; \
			std::string str = fmt[id]; \
			int val; from_string(str, val); \
			(*it)[column.col] = val; \
			se_debug_message(SE_DEBUG_LOADER, "id=[%d] column=[%s] string=[%s] result=[%d] error=[%s]", \
					id, col_text, str.c_str(), val, (map.find(col_text) == map.end()) ? "Column Not Found" : "NONE"); \
		}

		INIT_TO_BOOL(bold, "Bold");
		INIT_TO_BOOL(italic, "Italic");
		INIT_TO_BOOL(underline, "Underline");
		INIT_TO_BOOL(strikeout, "StrikeOut");

		INIT_TO_INT(scale_x, "ScaleX");
		INIT_TO_INT(scale_y, "ScaleY");

		INIT_TO_INT(spacing, "Spacing");
		INIT_TO_INT(angle, "Angle");

		INIT_TO_INT(border_style, "BorderStyle");
		INIT_TO_INT(outline, "Outline");

		INIT_TO_INT(shadow, "Shadow");
		INIT_TO_INT(alignment, "Alignment");
		INIT_TO_INT(margin_l, "MarginL");
		INIT_TO_INT(margin_r, "MarginR");
		INIT_TO_INT(margin_v, "MarginV");
	
		INIT_TO_INT(encoding, "Encoding");

		return true;
	}
	else if(line.find("Format: ") != std::string::npos)
	{
		std::string data = line;
		data.erase(0, std::string("Format: ").size());

		formats = build_format(data, -1, true);

		for(unsigned int i=0; i<formats.size(); ++i)
		{
			map[formats[i]] = i;
		}

		return true;
	}
	return false;
}


/*
 *	lecture du block [Events]
 */
bool SubtitleASS::readEvents(const std::string &_line)
{
	se_debug_message(SE_DEBUG_LOADER, "read Events");
	
	std::string line = _line;

	if(line.find("Dialogue: ") != std::string::npos)
	{
		line.erase(0,10);

		std::vector<std::string> array = build(line, 10);

		Subtitle subtitle = document()->subtitles().append();
			
		// marked/layer
		subtitle.set_layer(array[0]);
		subtitle.set_start_and_end(
				ass_time_to_subtitletime(array[1]) ,
				ass_time_to_subtitletime(array[2]) );

		subtitle.set_style(clean_style_name( charset_to_utf8(array[3]) ));
		subtitle.set_name(charset_to_utf8(array[4]));
			
		subtitle.set_margin_l(array[5]);
		subtitle.set_margin_r(array[6]);
		subtitle.set_margin_v(array[7]);

		subtitle.set_effect(charset_to_utf8(array[8]));

		Glib::ustring text = check_end_char(charset_to_utf8(array[9]));

		characters_to_newline(text, "\\n");
		characters_to_newline(text, "\\N");

		subtitle.set_text(text);

		return true;
	}
	return false;
}


/*
 *
 */
std::vector< std::string > SubtitleASS::build(const std::string &line, unsigned int column)
{
	std::vector< std::string > array;

	std::string::size_type s=0, e=0;

	do
	{
		if(column > 0 && array.size()+1 == column)
		{
			array.push_back(line.substr(s,std::string::npos));
			break;
		}

		e = line.find(",", s);
		
		array.push_back(line.substr(s,e-s));

		s=e+1;
	}while(e != std::string::npos);
	return array;
}


/*
 *	convertir le temps utiliser par subtitle editor en temps valide pour le format SSA
 *	0:00:00.000 -> 0:00:00.00
 */
Glib::ustring SubtitleASS::subtitletime_to_ass_time(const SubtitleTime &time)
{
	int msecs = (int)(time.msecs * 0.1/* + 0.5*/);

	gchar *tmp = g_strdup_printf("%01i:%02i:%02i.%02i",
			time.hours, time.mins, time.secs, msecs);

	Glib::ustring res(tmp);

	g_free(tmp);

	return res;
}

/*
 *	convertir un temps ASS en SubtitleTime (string)
 */
Glib::ustring SubtitleASS::ass_time_to_subtitletime(const Glib::ustring &text)
{
	if(SubtitleTime::validate(text))
		return SubtitleTime(text).str();
	
	std::cerr << "SubtitleASS::ass_time_to_subtitletime error > " << text << std::endl;

	return SubtitleTime::null();
}

/*
 *	convertir une couleur en couleur ASS pour la sauvegarde
 */
Glib::ustring SubtitleASS::color_to_ass_color(const Glib::ustring& color)
{
	Color c(color);

	unsigned int r = c.getR();
	unsigned int g = c.getG();
	unsigned int b = c.getB();
	unsigned int a = c.getA();

	unsigned int abgr = /*a << 24 |*/ b << 16 | g << 8 | r << 0;

	gchar *tmp = g_strdup_printf("&H%08X", abgr);
	
	Glib::ustring str(tmp);
	g_free(tmp);

	return str;
}

/*
 *	convertir une couleur ASS en Color
 */
Color SubtitleASS::ass_color_to_color(const Glib::ustring &str)
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
		return Color(temp[0], temp[1], temp[2], 255);// temp[3]);
	}
	catch(...)
	{
		//std::cerr << "SubtitleASS::ass_color_to_color error > " << str << std::endl;
	}

	return Color(0,0,0,255);
}

/*
 *
 */
std::string SubtitleASS::bool_to_string(const Glib::ustring &state)
{
	//se_debug_message(SE_DEBUG_LOADER | SE_DEBUG_SAVER, "bool=[%d] return=[%s]", state, (state) ? "-1" : "0");

	//return state ? "-1" : "0";
	if(state == "1")
		return "-1";
	return "0";
}

/*
 *
 */
bool SubtitleASS::string_to_bool(const std::string &string)
{
	se_debug_message(SE_DEBUG_LOADER | SE_DEBUG_SAVER, "string=[%s] return=[%d]", string.c_str(), (string == "0") ? false : true);

	return (string == "0") ? false : true;
}

/*
 *
 */
int SubtitleASS::string_to_int(const std::string &string)
{
	int res;
	from_string(string, res);

	se_debug_message(SE_DEBUG_LOADER | SE_DEBUG_SAVER, "string=[%s] return=[%d]", string.c_str(), res);

	return res;
}

