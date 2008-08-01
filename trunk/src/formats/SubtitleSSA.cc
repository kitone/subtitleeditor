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
 

#include "SubtitleSSA.h"
#include "Color.h"
#include "utility.h"

#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "RegEx.h"


Glib::ustring check_end_char(const Glib::ustring &str);

/*
 * clean source....
 */



/*
 *
 */
Glib::ustring SubtitleSSA::get_name()
{
	return "Sub Station Alpha";
}

/*
 *
 */
Glib::ustring SubtitleSSA::get_extension()
{
	return "ssa";
}

/*
 *
 */
bool SubtitleSSA::check(const std::string &line)
{
	static RegEx ex("^ScriptType:\\s+(v|V)+4.00([^\\+]|$)");
	
	return ex.exec(line);
}


/*
 *	recupere dans un tableau le format a partir de la line
 */
std::vector<std::string> build_format(const std::string &text, int column=-1, bool remove_space=false)
{
	
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
}






/*
 *	construtor
 */
SubtitleSSA::SubtitleSSA(Document* doc)
:SubtitleFormat(doc, get_name())
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);

	alignment_map[1] = 1;
	alignment_map[2] = 2;
	alignment_map[3] = 3;
	alignment_map[9] = 4;
	alignment_map[10]= 5;
	alignment_map[11]= 6;
	alignment_map[5] = 7;
	alignment_map[6] = 8;
	alignment_map[7] = 9;
}

/*
 *
 */
SubtitleSSA::~SubtitleSSA()
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *	read subtitle file
 */
bool SubtitleSSA::on_open(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_LOADER);
	
	std::ifstream file(filename.c_str());
	if(!file)
	{
		throw SubtitleException("SubtitleSSA", _("I can't open this file."));
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
		else if(line.find("[V4 Styles]") != std::string::npos)
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



bool SubtitleSSA::on_save(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_SAVER);

	std::ofstream file(filename.c_str());
	if(!file)
		throw SubtitleException("SubtitleSSA", _("I can't open this file."));




	// ScriptInfo
	{
		se_debug_message(SE_DEBUG_SAVER, "save ScriptInfo");

		file << utf8_to_charset("[Script Info]") << get_newline();
		file << utf8_to_charset("; This script was created by subtitleeditor ") << utf8_to_charset(VERSION) << get_newline();
		file << utf8_to_charset("; http://home.gna.org/subtitleeditor/") << get_newline();
	
#define CHECK(label, key) if(m_scriptInfo->key.size() > 0) \
		{ file << utf8_to_charset(label) << utf8_to_charset(m_scriptInfo->key) << get_newline(); }

		m_scriptInfo->ScriptType="V4.00";

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

	
	// Style
	{
		se_debug_message(SE_DEBUG_SAVER, "save Styles");
		// alignment utiliser par SSA
		// 5	-  6 -	7
		// 9	- 10 - 11
		// 1	-  2 -	3
		std::map<unsigned int, unsigned int> alignment_map;
		alignment_map[1] = 1;
		alignment_map[2] = 2;
		alignment_map[3] = 3;
		alignment_map[4] = 9;
		alignment_map[5] =10;
		alignment_map[6] =11;
		alignment_map[7] = 5;
		alignment_map[8] = 6;
		alignment_map[9] = 7;

		StyleColumnRecorder column;
		
		file << "[V4 Styles]" << get_newline();

		file << "Format: "
				<< "Name, "
				<< "Fontname, "
				<< "Fontsize, "
				
				<< "PrimaryColour, "
				<< "SecondaryColour, "
				<< "TertiaryColour, "
				<< "BackColour, "
				
				<< "Bold, "
				<< "Italic, "
				<< "BorderStyle, "
				<< "Outline, "
				<< "Shadow, "
				<< "Alignment, "
				<< "MarginL, "
				<< "MarginR, "
				<< "MarginV, "
				<< "AlphaLevel, "
				<< "Encoding" << get_newline();

		Gtk::TreeNodeChildren rows = m_styleModel->children();
		
		for(Gtk::TreeIter it = rows.begin(); it; ++it)
		{
			std::ostringstream oss;

			oss << "Style: " 
				<< utf8_to_charset((*it)[column.name]) << "," 
				<< utf8_to_charset((*it)[column.font_name]) << "," 
				<< (*it)[column.font_size] << ","
			
				<< color_to_ssa_color((*it)[column.primary_colour]) << "," 
				<< color_to_ssa_color((*it)[column.secondary_colour]) << "," 
				<< color_to_ssa_color((*it)[column.outline_colour]) << "," 
				<< color_to_ssa_color((*it)[column.shadow_colour]) << "," 

				<< bool_to_string((*it)[column.bold]) << "," 
				<< bool_to_string((*it)[column.italic]) << "," 
				
				<< (*it)[column.border_style] << "," 
				<< (*it)[column.outline] << "," 
				<< (*it)[column.shadow] << "," 
				<< alignment_map[ (*it)[column.alignment] ] << "," 
				<< (*it)[column.margin_l] << "," 
				<< (*it)[column.margin_r] << "," 
				<< (*it)[column.margin_v] << "," 
				<< (*it)[column.alpha_level] << "," 
				<< (*it)[column.encoding] << get_newline(); 
				
			file << oss.str();
		}

		file << get_newline();
	}

	// Event
	{
		se_debug_message(SE_DEBUG_SAVER, "save Events");

		file << "[Events]" << get_newline();
		// format:
		file << "Format: " <<
			"Marked, " <<
			"Start, " <<
			"End, " <<
			"Style, " <<
			"Name, " <<
			"MarginL, " <<
			"MarginR, " <<
			"MarginV, " <<
			"Effect, " <<
			"Text" << get_newline();
			
		// dialog:
		for(Subtitle subtitle = document()->subtitles().get_first(); subtitle; ++subtitle)
		{
			Glib::ustring text = subtitle.get_text();

			newline_to_characters(text, "\\n");

			std::ostringstream oss;

			oss << "Dialogue: "
				<< "Marked=" << 0/*(*it)[column.flag]*/ << "," 
				
				<< subtitletime_to_ssa_time(subtitle.get_start()) << "," 
				<< subtitletime_to_ssa_time(subtitle.get_end()) << ","
				
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
 *
 */
bool SubtitleSSA::readScripInfo(const std::string &_line)
{
	se_debug_message(SE_DEBUG_LOADER, "read ScriptInfo");
	
	std::string line = _line;

#define CHECK(label, key) \
		if(line.find(label) != std::string::npos) \
		{	line.erase(0, (std::string(label)).size()); m_scriptInfo->key = check_end_char(charset_to_utf8(line)); return true; }
//		else std::cerr << "CHECK not found: " << label << std::endl;

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
 *	read Style
 */
bool SubtitleSSA::readStyles(const std::string &_line)
{
	se_debug_message(SE_DEBUG_LOADER, "read Styles");
	
	
	std::string line = check_end_char(_line);

	
	if(line.find("Style: ") != std::string::npos)
	{
		StyleColumnRecorder column;
		// on donne la ligne sans "Style: " = size - 7
		std::vector<std::string> fmt = build_format(line.substr(7, line.size()-7), formats.size(), false);

		Gtk::TreeIter it = m_styleModel->append();

		// on supprime '*' si il existe
		(*it)[column.name]							= clean_style_name( charset_to_utf8(fmt[ map["Name"] ]) );
			
		(*it)[column.font_name]					= charset_to_utf8(fmt[ map["Fontname"] ]);
		(*it)[column.font_size]					= string_to_int( fmt[ map["Fontsize"] ] );
/*
		// color
		(*it)[column.primary_colour] = ssa_color_to_color( string_to_int( fmt[ map["PrimaryColour"] ] ));
		(*it)[column.secondary_colour] = ssa_color_to_color( string_to_int( fmt[ map["SecondaryColour"] ] ));
		(*it)[column.outline_colour] = ssa_color_to_color( string_to_int( fmt[ map["TertiaryColour"] ] ));
		(*it)[column.shadow_colour] = ssa_color_to_color( string_to_int( fmt[ map["BackColour"] ] ));
*/
		(*it)[column.bold]							= string_to_bool( fmt[ map["Bold"] ] );
		(*it)[column.italic]						= string_to_bool( fmt[ map["Italic"] ] );
		(*it)[column.border_style]			= string_to_int( fmt[ map["BorderStyle"] ] );
		(*it)[column.outline]						= string_to_int( fmt[ map["Outline"] ] );

		(*it)[column.shadow]						= string_to_int( fmt[ map["Shadow"] ]);
		(*it)[column.alignment]					= alignment_map[ string_to_int( fmt[ map["Alignment"] ]) ];
		(*it)[column.margin_l]					= string_to_int( fmt[ map["MarginL"] ]);
		(*it)[column.margin_r]					= string_to_int( fmt[ map["MarginR"] ]);
		(*it)[column.margin_v]					= string_to_int( fmt[ map["MarginV"] ]);
			
		(*it)[column.alpha_level]				= string_to_int( fmt[ map["AlphaLevel"] ]);
		(*it)[column.encoding]					= string_to_int( fmt[ map["Encoding"] ]);

		return true;
	}
	else if(line.find("Format: ") != std::string::npos)
	{
		formats = build_format(line, -1, true);

		for(unsigned int i=0; i<formats.size(); ++i)
			map[formats[i]] = i;

		return true;
	}
	return false;
}

/*
 *
 */
bool SubtitleSSA::readEvents(const std::string &_line)
{
	se_debug_message(SE_DEBUG_LOADER, "read Events");
	
	std::string line = _line;

	if(line.find("Dialogue: ") != std::string::npos)
	{
		line.erase(0,10);

		std::vector<std::string> array = build(line, 10);

		Subtitle subtitle = document()->subtitles().append();
			
		// marked/layer
		subtitle.set_start_and_end(
				ass_time_to_subtitletime(array[1]),
				ass_time_to_subtitletime(array[2]));


		subtitle.set_style( charset_to_utf8(clean_style_name( array[3] )));
		subtitle.set_name( charset_to_utf8(array[4]));
			
		subtitle.set_margin_l( array[5]);
		subtitle.set_margin_r( array[6]);
		subtitle.set_margin_v( array[7]);
		subtitle.set_effect( charset_to_utf8(array[8]));

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
std::vector< std::string > SubtitleSSA::build(const std::string &line, unsigned int column)
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
 *	convertir le temps utiliser par subtitle editor
 *	en temps valide pour le format SSA
 *	0:00:00.000 -> 0:00:00.00
 */
Glib::ustring SubtitleSSA::subtitletime_to_ssa_time(const SubtitleTime &time)
{
	int msecs = (int)(time.mseconds() * 0.1/* + 0.5*/);

	gchar *tmp = g_strdup_printf("%01i:%02i:%02i.%02i",
			time.hours(), time.minutes(), time.seconds(), msecs);

	Glib::ustring res(tmp);

	g_free(tmp);

	return res;
}

/*
 *	convertir le temps SSA en SubtitleTime (string)
 *	0:00:00.00 -> 0:00:00.000
 */
Glib::ustring SubtitleSSA::ass_time_to_subtitletime(const Glib::ustring &text)
{
	if(SubtitleTime::validate(text))
		return SubtitleTime(text).str();

	std::cerr << "SubtitleSSA::ass_time_to_subtitletime error > " << text << std::endl;
	return "";
}

/*
 *	hack !
 */
Glib::ustring SubtitleSSA::clean_style_name(const Glib::ustring &name)
{
	Glib::ustring str = name;
	Glib::ustring::size_type n = str.find('*');
	if(n != Glib::ustring::npos)
		str.erase(n,1);

	return str;
}

/*
 *	convertir une couleur en format SSA
 */
Glib::ustring SubtitleSSA::color_to_ssa_color(const Glib::ustring &color)
{
	Color c(color);
	unsigned int r = c.getR();
	unsigned int g = c.getG();
	unsigned int b = c.getB();

	std::ostringstream oss;
	oss << (b << 16 | g << 8 | r << 0) ;
	return oss.str();
}

/*
 *	convertir une couleur SSA en Color (interne)
 */
Color SubtitleSSA::ssa_color_to_color(const unsigned int &ssa)
{
	unsigned int r = (ssa & 0x0000FF) >> 0;
	unsigned int g = (ssa & 0x00FF00) >> 8;
	unsigned int b = (ssa & 0xFF0000) >> 16;

	Color color;
	color.set(r, g, b, 255);
	return color;
}

/*
 *
 */
std::string SubtitleSSA::bool_to_string(const bool &state)
{
	se_debug_message(SE_DEBUG_LOADER | SE_DEBUG_SAVER, "bool=[%d] return=[%s]", state, (state) ? "-1" : "0");

	return state ? "-1" : "0";
}

/*
 *
 */
bool SubtitleSSA::string_to_bool(const std::string &string)
{
	se_debug_message(SE_DEBUG_LOADER | SE_DEBUG_SAVER, "string=[%s] return=[%d]", string.c_str(), (string == "0") ? false : true);

	return (string == "0") ? false : true;
}

/*
 *
 */
int SubtitleSSA::string_to_int(const std::string &string)
{
	int res;
	from_string(string, res);

	se_debug_message(SE_DEBUG_LOADER | SE_DEBUG_SAVER, "string=[%s] return=[%d]", string.c_str(), res);

	return res;
}

