/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2011, kitone
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


class DCSubtitle : public SubtitleFormatIO
{
public:

	/*
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

			// <DCSubtitle> (dcsubtitle)
			const xmlpp::Node* dcsubtitle = parser.get_document()->get_root_node();
			// <Font>
			const xmlpp::Element *font = dynamic_cast<const xmlpp::Element*>(dcsubtitle->get_children("Font").front());
			read_font(font);
		}
		catch(const std::exception &ex)
		{
			throw IOFileError(_("Failed to open the file for reading."));
		}
	}

	/*
	 */
	void save(FileWriter &file)
	{
		try
		{
			xmlpp::Document doc;
			// Comments about date and application
			{
				doc.add_comment(" XML Subtitle File ");
				Glib::Date date;
				date.set_time_current();
				doc.add_comment(date.format_string(" %Y-%m-%d "));
				doc.add_comment(Glib::ustring::compose(" Created by subtitleeditor version %1 ", VERSION));
				doc.add_comment(" http://home.gna.org/subtitleeditor/ ");
			}
			xmlpp::Element* xml_dcsubtitle = doc.create_root_node("DCSubtitle");
			
			xml_dcsubtitle->set_attribute("Version", "1.0");

			// element SubtitleID
			//xmlpp::Element* xml_subtitleid = xml_dcsubtitle->add_child("SubtitleID");
			// element MovieTitle
			xmlpp::Element* xml_movietitle = xml_dcsubtitle->add_child("MovieTitle");
			// element ReelNumber
			xmlpp::Element* xml_reelnumber = xml_dcsubtitle->add_child("ReelNumber");
			xml_reelnumber->set_child_text("1");
			// element Language
			//xmlpp::Element* xml_language = xml_dcsubtitle->add_child("Language");
			// element LoadFont
			//xmlpp::Element* xml_loadfont = xml_dcsubtitle->add_child("LoadFont");
	
			// Font
			xmlpp::Element* xml_font = xml_dcsubtitle->add_child("Font");
			{
				// attribute Id
				// attribute Color
				// attribute Weight
				// attribute Spacing
				// attribute Effect
				// attribute EffectColor
				// attribute Size
				// attribute AspectAdjust
				// attribute Italic
			
				// Write each Subtitle
				for(Subtitle sub = document()->subtitles().get_first(); sub; ++sub)
				{
					write_subtitle(xml_font, sub);
				}
			}
			file.write(doc.write_to_string_formatted("UTF-8"));
		}
		catch(const std::exception &ex)
		{
			throw IOFileError(_("Failed to write to the file."));
		}
	}

	/*
	 */
	void read_font(const xmlpp::Element *xml_font)
	{
		if(xml_font == NULL || xml_font->get_name() != "Font")
			return;
		// attribute Id
		// attribute Color
		// attribute Weight
		// attribute Spacing
		// attribute Effect
		// attribute EffectColor
		// attribute Size
		// attribute AspectAdjust
		// attribute Italic
		
		// Read each Subtitle
		xmlpp::Node::NodeList list = xml_font->get_children("Subtitle");
		for(xmlpp::Node::NodeList::const_iterator it = list.begin(); it!=list.end(); ++it)
		{
			read_subtitle(dynamic_cast<const xmlpp::Element*>(*it));
		}
	}

	/*
	 */
	void read_subtitle(const xmlpp::Element *xml_subtitle)
	{
		if(xml_subtitle == NULL || xml_subtitle->get_name() != "Subtitle")
			return;

		Subtitle subtitle = document()->subtitles().append();

		// attribute SpotNumber ignored, not usefull
		// attribute FadeUpTime, ignored
		// attribute FadeDownTime, ignored

		// TimeIn
		const xmlpp::Attribute *att_timein = xml_subtitle->get_attribute("TimeIn");
		if(att_timein)
		{
			subtitle.set_start(
					time_to_se(
						Glib::ustring(att_timein->get_value())));
		}

		// TimeOut
		const xmlpp::Attribute *att_timeount = xml_subtitle->get_attribute("TimeOut");
		if(att_timeount)
		{
			subtitle.set_end(
					time_to_se(
						Glib::ustring(att_timeount->get_value())));
		}

		// Text (children)
		xmlpp::Node::NodeList children = xml_subtitle->get_children("Text");
		for(xmlpp::Node::NodeList::const_iterator it = children.begin(); it != children.end(); ++it)
		{
			const xmlpp::Element* xml_text = dynamic_cast<const xmlpp::Element*>(*it);

			// attribute Direction
			// attribute HAlign
			// attribute HPosition
			// attribute VAlign
			// attribute VPosition

			// text (child)
			Glib::ustring text = xml_text->get_child_text()->get_content();

			if(!subtitle.get_text().empty()) // Add break line if needs
				text = "\n" + text;

			subtitle.set_text(subtitle.get_text() + text);
		}
	}

	/*
	 */
	void write_subtitle(xmlpp::Element* xml_root, const Subtitle &sub)
	{
		Glib::ustring SpotNumber = to_string(sub.get_num());
		Glib::ustring TimeIn = time_to_dcsubtitle(sub.get_start());
		Glib::ustring TimeOut = time_to_dcsubtitle(sub.get_end());
		Glib::ustring FadeUpTime = "0";
		Glib::ustring FadeDownTime = "0";

		xmlpp::Element* xml_subtitle = xml_root->add_child("Subtitle");

		xml_subtitle->set_attribute("SpotNumber", SpotNumber);
		xml_subtitle->set_attribute("TimeIn", TimeIn);
		xml_subtitle->set_attribute("TimeOut", TimeOut);
		xml_subtitle->set_attribute("FadeUpTime", FadeUpTime);
		xml_subtitle->set_attribute("FadeDownTime", FadeDownTime);

		std::vector<Glib::ustring> lines;
		utility::usplit(sub.get_text(), '\n', lines);

		for(guint i=0; i<lines.size(); ++i)
		{
			Glib::ustring Direction = "horizontal";
			Glib::ustring HAlign = "center";
			Glib::ustring HPosition = "0.0";
			Glib::ustring VAlign = "bottom";
			Glib::ustring VPosition = "0.0"; // FIXME ?

			xmlpp::Element* xml_text = xml_subtitle->add_child("Text");

			xml_text->set_attribute("Direction", Direction);
			xml_text->set_attribute("HAlign", HAlign);
			xml_text->set_attribute("HPosition", HPosition);
			xml_text->set_attribute("VAlign", VAlign);
			xml_text->set_attribute("VPosition", VPosition);
			xml_text->set_child_text(lines[i]);
		}
	}

	/*
	 * Convert SE time to DCSubtitle time.
	 */
	Glib::ustring time_to_dcsubtitle(const SubtitleTime &time)
	{
		// FIXME
		return build_message("%.2i:%.2i:%.2i:%.3i",
				time.hours(), time.minutes(), time.seconds(), time.mseconds() / 4);
	}

	/*
	 * Convert DCSubtitle time to SE time.
	 */
	SubtitleTime time_to_se(const Glib::ustring &value)
	{
		int h,m,s,ms;
		if(sscanf(value.c_str(), "%d:%d:%d:%d", &h, &m, &s, &ms) == 4)
			return SubtitleTime(h,m,s,ms * 4); // FIXME
		return SubtitleTime();
	}

};

class DCSubtitlePlugin : public SubtitleFormat
{
public:

	/*
	 */
	SubtitleFormatInfo get_info()
	{
		SubtitleFormatInfo info;
		info.name = "DCSubtitle";
		info.extension = "xml";
		info.pattern = "<DCSubtitle";

		return info;
	}

	/*
	 */
	SubtitleFormatIO* create()
	{
		DCSubtitle *sf = new DCSubtitle();
		return sf;
	}
};

REGISTER_EXTENSION(DCSubtitlePlugin)
