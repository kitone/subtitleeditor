#ifdef ENABLE_TTXT

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


#include "SubtitleTTXT.h"
#include <libxml++/libxml++.h>
#include <iostream>
#include <glib.h>
#include "utility.h"

/*
 *
 */
Glib::ustring SubtitleTTXT::get_name()
{
	return "TTXT";
}

/*
 *
 */
Glib::ustring SubtitleTTXT::get_extension()
{
	return "ttxt";
}

/*
 *
 */
bool SubtitleTTXT::check(const std::string &line)
{
	if(line.find("<TextStream") != std::string::npos)
		return true;
	return false;
}



/*
 *
 */
SubtitleTTXT::SubtitleTTXT(Document* doc)
:SubtitleFormat(doc, get_name())
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *
 */
SubtitleTTXT::~SubtitleTTXT()
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}


bool SubtitleTTXT::on_open(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_LOADER);

	try
	{
		xmlpp::DomParser parser;
		//parser.set_validate();
		parser.set_substitute_entities();
		parser.parse_file(filename);

		if(!parser)
			throw IOFileError(_("Failed to open the file for reading."));

		const xmlpp::Node* root = parser.get_document()->get_root_node();
		xmlpp::Node::NodeList list = root->get_children();

		Subtitles subtitles = document()->subtitles();

		for(xmlpp::Node::NodeList::const_iterator it = list.begin(); it!=list.end(); ++it)
		{

			if((*it)->get_name() == "TextSample")
			{
				const xmlpp::Element *element = dynamic_cast<const xmlpp::Element*>(*it);

				Subtitle subtitle = subtitles.append();
					
				// text
				const xmlpp::Attribute *att_text = element->get_attribute("text");
				if(att_text)
				{
					Glib::ustring text = att_text->get_value();

					subtitle.set_text(text);
				}
				// time
				const xmlpp::Attribute *att_time =  element->get_attribute("sampleTime");
				if(att_time)
				{
					Glib::ustring time = att_time->get_value();

					subtitle.set_start(time);
				}
			}
		}
		return true;
	}
	catch(const std::exception &ex)
	{
		throw IOFileError(_("Failed to open the file for reading."));
	}

	return false;
}


/*
 *
 */
bool SubtitleTTXT::on_save(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_SAVER);
	
	try
	{
		xmlpp::Document doc;

		xmlpp::Element* root = doc.create_root_node("TextStream");

		
		for(Subtitle subtitle = document()->subtitles().get_first(); subtitle; ++subtitle)
		{
			xmlpp::Element* sub = root->add_child("TextSample");

			SubtitleTime start = subtitle.get_start();
			SubtitleTime end = subtitle.get_end();
			Glib::ustring text = "'" + subtitle.get_text() + "'";


			sub->set_attribute("sampleTime", get_time(start));
			//sub->set_attribute("sampleTime", get_time(end));
			
			newline_to_characters(text, "''");

			sub->set_attribute("text", text);
		}

		doc.write_to_file(filename, "UTF-8");
	}
	catch(const std::exception &ex)
	{
		throw IOFileError(_("Failed to open the file for writing."));
	}

	return true;
}

/*
 *
 */
Glib::ustring SubtitleTTXT::get_time(const SubtitleTime &time)
{
	gchar* tmp = g_strdup_printf("%.2i:%.2i:%.2i.%.3i",
			time.hours(), time.minutes(), time.seconds(), time.mseconds());
	Glib::ustring str(tmp);
	g_free(tmp);

	return str;
}

#endif//ENABLE_TTXT
