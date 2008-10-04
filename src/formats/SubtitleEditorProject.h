#ifndef _SubtitleEditorProject_h
#define _SubtitleEditorProject_h

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
#include <libxml++/libxml++.h>

/*
 * TODO:	<subtitleview>
 *					<selection>
 *					<position>
 *				</subtitleview>
 *				<metadata>
 *				video, title, info, comment ...
 *				</metadata>
 */
class SubtitleEditorProject : public SubtitleFormat
{
public:

	/*
	 *
	 */
	static SubtitleFormatInfo get_info()
	{
		SubtitleFormatInfo info;
		info.name = "Subtitle Editor Project";
		info.extension = "xml";
		info.pattern = "^<SubtitleEditorProject\\s.*>$";
		
		return info;
	}

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

			const xmlpp::Node* root = parser.get_document()->get_root_node();
			// 
			{
			}

			// Styles -------------------------------------------
			{
				const xmlpp::Element *xmlstyles = dynamic_cast<const xmlpp::Element*>(root->get_children("styles").front());

				const xmlpp::Node::NodeList list_styles = xmlstyles->get_children("style");
				xmlpp::Node::NodeList::const_iterator it;

				Styles styles = document()->styles();

				for( it = list_styles.begin(); it != list_styles.end();	++it)
				{
					const xmlpp::Element *el = dynamic_cast<const xmlpp::Element*>(*it);

					Style style = styles.append();

					const xmlpp::Element::AttributeList list = el->get_attributes();

					for(xmlpp::Element::AttributeList::const_iterator at = list.begin(); at != list.end(); ++at)
					{
						style.set((*at)->get_name(), (*at)->get_value());
					}
				}
			}

			// Subtitles ----------------------------------------
			const xmlpp::Element *xmlsubtitles = dynamic_cast<const xmlpp::Element*>(root->get_children("subtitles").front());

			const xmlpp::Node::NodeList list_subtitles = xmlsubtitles->get_children("subtitle");
			xmlpp::Node::NodeList::const_iterator it;

			Subtitles subtitles = document()->subtitles();

			for( it = list_subtitles.begin(); it != list_subtitles.end();	++it)
			{
				const xmlpp::Element *el = dynamic_cast<const xmlpp::Element*>(*it);

				Subtitle sub = subtitles.append();

				const xmlpp::Element::AttributeList list = el->get_attributes();

				for(xmlpp::Element::AttributeList::const_iterator at = list.begin(); at != list.end(); ++at)
				{
					sub.set((*at)->get_name(), (*at)->get_value());
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
			xmlpp::Document xmldoc;

			xmlpp::Element *root = xmldoc.create_root_node("SubtitleEditorProject");
			root->set_attribute("version", "1.0");

			// Style ------------------------
			xmlpp::Element *xmlstyles = root->add_child("styles");

			Styles styles = document()->styles();

			for(Style style = styles.first(); style; ++style)
			{
				xmlpp::Element *xml = xmlstyles->add_child("style");

				std::map<Glib::ustring, Glib::ustring> values;
				style.get(values);

				std::map<Glib::ustring, Glib::ustring>::const_iterator it;
				for(it = values.begin(); it != values.end(); ++it)
					xml->set_attribute(it->first, it->second);
			}

			// Subtitle --------------------
			xmlpp::Element *xmlsubtitles = root->add_child("subtitles");

			Subtitles subtitles = document()->subtitles();

			for(Subtitle sub = subtitles.get_first(); sub; ++sub)
			{
				xmlpp::Element *xml = xmlsubtitles->add_child("subtitle");

				std::map<Glib::ustring, Glib::ustring> values;
				sub.get(values);

				std::map<Glib::ustring, Glib::ustring>::const_iterator it;
				for(it = values.begin(); it != values.end(); ++it)
					xml->set_attribute(it->first, it->second);
			}

			file << xmldoc.write_to_string_formatted();
		}
		catch(const std::exception &ex)
		{
			throw IOFileError(_("Failed to save the file for writing."));
		}
	}

};

#endif//_SubtitleEditorProject_h

