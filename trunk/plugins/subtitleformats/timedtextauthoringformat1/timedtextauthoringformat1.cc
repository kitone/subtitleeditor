/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2009, kitone
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
#include <error.h>
#include <libxml++/libxml++.h>


class TimedTextAuthoringFormat1 : public SubtitleFormatIO
{
public:

	/*
	 *
	 */
	void open(FileReader &file)
	{
		try
		{
			xmlpp::DomParser parser;
			//parser.set_validate();
			parser.set_substitute_entities();
			parser.parse_memory(file.get_data());

			if(!parser)
				throw IOFileError(_("Failed to open the file for reading."));

			// <tt> (root)
			const xmlpp::Node* root = parser.get_document()->get_root_node();

			// <body>
			const xmlpp::Element *body = dynamic_cast<const xmlpp::Element*>(root->get_children("body").front());
			if(body)
			{
				// <div>
				const xmlpp::Element *div = dynamic_cast<const xmlpp::Element*>(body->get_children("div").front());

				if(div)
				{
					xmlpp::Node::NodeList list = div->get_children();
					for(xmlpp::Node::NodeList::const_iterator it = list.begin(); it!=list.end(); ++it)
					{
						read_subtitle(dynamic_cast<const xmlpp::Element*>(*it));
					}
				}
			}
		}
		catch(const std::exception &ex)
		{
			throw IOFileError(_("Failed to open the file for reading."));
		}
	}

	/*
	 *
	 */
	void save(FileWriter &file)
	{
		try
		{
			xmlpp::Document doc;

			xmlpp::Element* tt = doc.create_root_node("tt");
			tt->set_attribute("xml:lang", "");
			tt->set_attribute("xmlns", "http://www.w3.org/2006/10/ttaf1");

			xmlpp::Element* body = tt->add_child("body");

			// div subtitles
			xmlpp::Element* div = body->add_child("div");
		
			div->set_attribute("xml:lang", "en");

			for(Subtitle sub = document()->subtitles().get_first(); sub; ++sub)
			{
				write_subtitle(div, sub);
			}

			file.write(doc.write_to_string_formatted());
		}
		catch(const std::exception &ex)
		{
			throw IOFileError(_("Failed to write to the file."));
		}
	}

	/*
	 *
	 */
	void read_subtitle(const xmlpp::Element *p)
	{
		if(p == NULL || p->get_name() != "p")
			return;

		Subtitle subtitle = document()->subtitles().append();

		// begin
		const xmlpp::Attribute *att_begin = p->get_attribute("begin");
		if(att_begin)
		{
			Glib::ustring begin = att_begin->get_value(); 

			subtitle.set_start(time_to_se(begin));
		}

		// end
		const xmlpp::Attribute *att_end = p->get_attribute("end");
		if(att_end)
		{
			Glib::ustring end = att_end->get_value(); 

			subtitle.set_end(time_to_se(end));
		}
		else //dur only if end failed
		{
			const xmlpp::Attribute *att_dur = p->get_attribute("dur");
			if(att_dur)
			{
				Glib::ustring dur = att_dur->get_value(); 

				subtitle.set_duration(time_to_se(dur));
			}
		}

		// text
		if(p->has_child_text())
		{
			Glib::ustring text = p->get_child_text()->get_content();

			utility::replace(text, "<br/>", "\n");

			subtitle.set_text(text);
		}
	}

	/*
	 *
	 */
	void write_subtitle(xmlpp::Element* root, const Subtitle &sub)
	{
		Glib::ustring text = sub.get_text();
	
		utility::replace(text, "\n", "<br/>");

		xmlpp::Element* p = root->add_child("p");

		p->set_attribute("begin", time_to_ttaf1(sub.get_start()));
		p->set_attribute("end", time_to_ttaf1(sub.get_end()));
		p->set_attribute("dur", time_to_ttaf1(sub.get_duration()));
		p->set_child_text(text);
	}

	/*
	 * Convert SE time to TT time.
	 */
	Glib::ustring time_to_ttaf1(const SubtitleTime &time)
	{
		return build_message("%.2i:%.2i:%.2i.%.3i",
				time.hours(), time.minutes(), time.seconds(), time.mseconds());
	}

	/*
	 * Convert TT time to SE time.
	 */
	SubtitleTime time_to_se(const Glib::ustring &value)
	{
		if(SubtitleTime::validate(value))
			return SubtitleTime(value);

		return SubtitleTime();
	}

};

class TimedTextAuthoringFormat1Plugin : public SubtitleFormat
{
public:

	/*
	 *
	 */
	SubtitleFormatInfo get_info()
	{
		SubtitleFormatInfo info;
		info.name = "Timed Text Authoring Format 1.0";
		info.extension = "xml";
		info.pattern = "^<tt";
		
		return info;
	}

	/*
	 *
	 */
	SubtitleFormatIO* create()
	{
		TimedTextAuthoringFormat1 *sf = new TimedTextAuthoringFormat1();
		return sf;
	}
};

REGISTER_EXTENSION(TimedTextAuthoringFormat1Plugin)
