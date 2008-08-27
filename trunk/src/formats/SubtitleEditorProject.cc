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
 


#include "SubtitleEditorProject.h"
#include <iostream>
#include <fstream>
#include <glibmm/ustring.h>
#include "utility.h"
#include "RegEx.h"
#include <libxml++/libxml++.h>

/*
 *
 */
Glib::ustring SubtitleEditorProject::get_name()
{
	return "Subtitle Editor Project";
}

/*
 *
 */
Glib::ustring SubtitleEditorProject::get_extension()
{
	return "sep";
}

/*
 *
 */
bool SubtitleEditorProject::check(const std::string &line)
{
	if(line.find("<SubtitleEditorProject") != std::string::npos)
		return true;
	return false;
}


/*
 *
 */
SubtitleEditorProject::SubtitleEditorProject(Document* doc)
:SubtitleFormat(doc, get_name())
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}

/*
 *
 */
SubtitleEditorProject::~SubtitleEditorProject()
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);
}


/*
 *
 */
bool SubtitleEditorProject::on_open(const Glib::ustring &filename)
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
bool SubtitleEditorProject::on_save(const Glib::ustring &filename)
{
	se_debug(SE_DEBUG_SAVER);

	try
	{
		xmlpp::Document xmldoc;

		xmlpp::Element *root = xmldoc.create_root_node("SubtitleEditorProject");
		root->set_attribute("version", "1.0");

		// Document
		{
			//xmlpp::Element *xmlstyles = root->add_child("data");
		}

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

		// States
		{
			//xmlpp::Element *xml = root->add_child("states");
		}

		// TODO UTF-8
		xmldoc.write_to_file_formatted(filename, "UTF-8");

		return true;
	}
	catch(const std::exception &ex)
	{
		throw IOFileError(_("Failed to open the file for writing."));
	}

	return false;
}




