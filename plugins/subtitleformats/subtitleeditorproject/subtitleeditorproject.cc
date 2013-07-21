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

#include <extension/subtitleformat.h>
#include <utility.h>
#include <waveformmanager.h>
#include <player.h>
#include <subtitleeditorwindow.h>
#include <debug.h>
#include <i18n.h>
#include <error.h>
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
class SubtitleEditorProject : public SubtitleFormatIO
{
public:

	/*
	 */
	void open(Reader &file)
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

			open_player(root);
			open_waveform(root);
			open_keyframes(root);
			open_styles(root);
			open_subtitles(root);
			open_subtitles_selection(root);
		}
		catch(const std::exception &ex)
		{
			throw IOFileError(_("Failed to open the file for reading."));
		}
	}

	/*
	 */
	void save(Writer &file)
	{
		try
		{
			xmlpp::Document xmldoc;

			xmlpp::Element *root = xmldoc.create_root_node("SubtitleEditorProject");
			root->set_attribute("version", "1.0");

			save_player(root);
			save_waveform(root);
			save_keyframes(root);
			save_styles(root);
			save_subtitles(root);
			save_subtitles_selection(root);

			file.write(xmldoc.write_to_string_formatted());
		}
		catch(const std::exception &ex)
		{
			throw IOFileError(_("Failed to write to the file."));
		}
	}

private:

	/*
	 */
	const xmlpp::Element* get_unique_children(const xmlpp::Node *root, const Glib::ustring &name)
	{
		const xmlpp::Node::NodeList children = root->get_children(name);
		if(children.empty())
			return NULL;
		return dynamic_cast<const xmlpp::Element*>(children.front());
	}

	/*
	 */
	void open_player(const xmlpp::Node *root)
	{
		const xmlpp::Element *xml_pl = get_unique_children(root, "player");
		if(xml_pl == NULL)
			return;

		Glib::ustring uri = xml_pl->get_attribute_value("uri");

		Player *pl = SubtitleEditorWindow::get_instance()->get_player();

		if(pl->get_uri() != uri)
			pl->open(uri);
	}

	/*
	 */
	void save_player(xmlpp::Element *root)
	{
		Player* pl = SubtitleEditorWindow::get_instance()->get_player();
		if(pl == NULL)
			return;

		Glib::ustring uri = pl->get_uri();
		if(uri.empty())
			return;

		xmlpp::Element* xmlpl = root->add_child("player");
		xmlpl->set_attribute("uri", uri);
	}

	/*
	 */
	void open_waveform(const xmlpp::Node* root)
	{
		const xmlpp::Element *xml_wf = get_unique_children(root, "waveform");
		if(xml_wf == NULL)
			return;

		Glib::ustring uri = xml_wf->get_attribute_value("uri");
		if(!uri.empty())
			SubtitleEditorWindow::get_instance()->get_waveform_manager()->open_waveform(uri);
	}

	/*
	 */
	void save_waveform(xmlpp::Element *root)
	{
		WaveformManager* wm = SubtitleEditorWindow::get_instance()->get_waveform_manager();
		if(wm->has_waveform() == false)
			return; // don't need to save without Waveform... 
		
		Glib::RefPtr<Waveform> wf = wm->get_waveform();
		if(!wf)
			return;

		xmlpp::Element *xmlwf = root->add_child("waveform");

		xmlwf->set_attribute("uri", wf->get_uri());
	}

	/*
	 */
	void open_keyframes(const xmlpp::Node* root)
	{
		const xmlpp::Element *xml_kf = get_unique_children(root, "keyframes");
		if(xml_kf == NULL)
			return;

		Glib::ustring uri = xml_kf->get_attribute_value("uri");
		if(uri.empty())
			return;
		Glib::RefPtr<KeyFrames> kf = KeyFrames::create_from_file(uri);
		if(!kf)
			return;
		SubtitleEditorWindow::get_instance()->get_player()->set_keyframes(kf);
	}

	/*
	 */
	void save_keyframes(xmlpp::Element *root)
	{
		Glib::RefPtr<KeyFrames> kf = SubtitleEditorWindow::get_instance()->get_player()->get_keyframes();
		if(!kf)
			return; // don't need to save without KeyFrames... 

		xmlpp::Element *xmlwf = root->add_child("keyframes");

		xmlwf->set_attribute("uri", kf->get_uri());
	}

	/*
	 */
	void open_styles(const xmlpp::Node *root)
	{
		const xmlpp::Element *xmlstyles = get_unique_children(root, "styles");
		if(xmlstyles == NULL)
			return;

		Styles styles = document()->styles();

		const xmlpp::Node::NodeList list_styles = xmlstyles->get_children("style");

		for(xmlpp::Node::NodeList::const_iterator it = list_styles.begin(); it != list_styles.end();	++it)
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

	/*
	 */
	void save_styles(xmlpp::Element *root)
	{
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
	}

	/*
	 */
	void open_subtitles(const xmlpp::Node* root)
	{
		const xmlpp::Element *xmlsubtitles = get_unique_children(root, "subtitles");
		if(xmlsubtitles == NULL)
			return;

		Glib::ustring timing_mode = xmlsubtitles->get_attribute_value("timing_mode");
		if(!timing_mode.empty())
		{
			if(timing_mode == "TIME")
				document()->set_timing_mode(TIME);
			else if(timing_mode == "FRAME")
				document()->set_timing_mode(FRAME);
		}

		Glib::ustring edit_timing_mode = xmlsubtitles->get_attribute_value("edit_timing_mode");
		if(!edit_timing_mode.empty())
		{
			if(edit_timing_mode == "TIME")
				document()->set_edit_timing_mode(TIME);
			else if(edit_timing_mode == "FRAME")
				document()->set_edit_timing_mode(FRAME);
		}

		Glib::ustring framerate = xmlsubtitles->get_attribute_value("framerate");
		if(!framerate.empty())
		{
			float value = (float)utility::string_to_double(framerate);
			if(value > 0)
				document()->set_framerate(get_framerate_from_value(value));
		}

		const xmlpp::Node::NodeList list_subtitles = xmlsubtitles->get_children("subtitle");

		Subtitles subtitles = document()->subtitles();

		for(xmlpp::Node::NodeList::const_iterator it = list_subtitles.begin(); it != list_subtitles.end();	++it)
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

	/*
	 */
	void save_subtitles(xmlpp::Element *root)
	{
		xmlpp::Element* xmlsubtitles = root->add_child("subtitles");
		
		// document property
		xmlsubtitles->set_attribute("timing_mode", (document()->get_timing_mode() == TIME) ? "TIME" : "FRAME");
		xmlsubtitles->set_attribute("edit_timing_mode", (document()->get_edit_timing_mode() == TIME) ? "TIME" : "FRAME");
		xmlsubtitles->set_attribute("framerate", to_string(get_framerate_value(document()->get_framerate())));

		// subtitles
		Subtitles subtitles = document()->subtitles();

		for(Subtitle sub = subtitles.get_first(); sub; ++sub)
		{
			xmlpp::Element *xmlsub = xmlsubtitles->add_child("subtitle");

			std::map<Glib::ustring, Glib::ustring> values;
			sub.get(values);

			std::map<Glib::ustring, Glib::ustring>::const_iterator it;
			for(it = values.begin(); it != values.end(); ++it)
				xmlsub->set_attribute(it->first, it->second);
		}
	}

	/*
	 */
	void open_subtitles_selection(const xmlpp::Node *root)
	{
		const xmlpp::Element *xmlsubtitles = get_unique_children(root, "subtitles-selection");
		if(xmlsubtitles == NULL)
			return;

		const xmlpp::Node::NodeList list_subtitles = xmlsubtitles->get_children("subtitle");

		std::vector<Subtitle> selection(list_subtitles.size());

		Subtitles subtitles = document()->subtitles();

		unsigned int i=0;
		for(xmlpp::Node::NodeList::const_iterator it = list_subtitles.begin(); it != list_subtitles.end();	++it, ++i)
		{
			const xmlpp::Element *el = dynamic_cast<const xmlpp::Element*>(*it);
			long path = utility::string_to_long(el->get_attribute_value("path"));

			selection[i] = subtitles.get(path+1); // /!\ warning: PATH is not NUM
		}
		subtitles.select(selection);
	}

	/*
	 */
	void save_subtitles_selection(xmlpp::Element *root)
	{
		xmlpp::Element* xml = root->add_child("subtitles-selection");

		std::vector<Subtitle> selection = document()->subtitles().get_selection();

		for(unsigned int i=0; i < selection.size(); ++i)
		{
			xmlpp::Element* xmlsub = xml->add_child("subtitle");
			xmlsub->set_attribute("path", selection[i].get("path"));
		}
	}
};

class SubtitleEditorProjectPlugin : public SubtitleFormat
{
public:

	/*
	 *
	 */
	SubtitleFormatInfo get_info()
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
	SubtitleFormatIO* create()
	{
		SubtitleEditorProject *sf = new SubtitleEditorProject();
		return sf;
	}
};

REGISTER_EXTENSION(SubtitleEditorProjectPlugin)
