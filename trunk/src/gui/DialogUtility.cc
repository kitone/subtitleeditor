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
 
#include "DialogUtility.h"
#include "utility.h"
#include "SubtitleSystem.h"
#include "Config.h"
#include "Encodings.h"

/*
 *
 */
DialogActionMultiDoc::DialogActionMultiDoc(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:Gtk::Dialog(cobject)
{
	refGlade->get_widget("radio-current-document", m_radioCurrentDocument);
	refGlade->get_widget("radio-all-documents", m_radioAllDocuments);
}

/*
 *
 */
bool DialogActionMultiDoc::apply_to_all_documents()
{
	return m_radioAllDocuments->get_active();
}

/*
 *	retourne la list des documents à modifier
 *	selon qu'on utilise "current document" ou "All documents"
 */
DocumentList DialogActionMultiDoc::get_documents_to_apply()
{
	DocumentList docs;

	if(apply_to_all_documents())
		docs = DocumentSystem::getInstance().getAllDocuments();
	else
		docs.push_back( DocumentSystem::getInstance().getCurrentDocument() );
	
	return docs;
}





class FileChooserExtra : public Gtk::Table
{
public:
	FileChooserExtra(int extra)
	:Gtk::Table(2,2, false), 
		m_extra(extra),
		m_labelEncodings(_("Character Coding:"), 0.0,0.5), 
		m_labelFormat(_("Format:"), 0.0,0.5), 
		m_labelNewLine(_("NewLine:"), 0.0,0.5)
	{
		set_col_spacings(3);
		set_row_spacings(3);

		attach(m_labelFormat, 0,1,0,1, (Gtk::SHRINK | Gtk::FILL));
		attach(m_comboFormat,1,2,0,1);
		
		attach(m_labelEncodings, 0,1,1,2,  (Gtk::SHRINK | Gtk::FILL));
		attach(m_comboEncodings,1,2,1,2);

		attach(m_labelNewLine, 0,1,2,3,  (Gtk::SHRINK | Gtk::FILL));
		attach(m_comboNewLine,1,2,2,3);

		// s'il y a une modifications dans la configuration
		Config::getInstance().signal_changed("encodings").connect(
				sigc::mem_fun(*this, &FileChooserExtra::on_config_changed));

		//
		std::list<Glib::ustring> formats = SubtitleSystem::getInstance().get_formats();

		for(std::list<Glib::ustring>::const_iterator it=formats.begin(); it != formats.end(); ++it)
			m_comboFormat.append_text(*it);

		m_comboFormat.set_active(0);
		//
		init_combo_encodgins();
		
		m_comboNewLine.append_text("Unix");
		m_comboNewLine.append_text("Windows");
		m_comboNewLine.set_active_text("Unix");

		show_all();

		if(!(extra & EXTRA_ENCODING) && !(extra & EXTRA_ENCODING_WITH_AUTO_DETECTED))
		{
			m_labelEncodings.hide();
			m_comboEncodings.hide();
		}
		if((extra & EXTRA_FORMAT) == false)
		{
			m_labelFormat.hide();
			m_comboFormat.hide();
		}

		if((extra & EXTRA_NEWLINE) == false)
		{
			m_labelNewLine.hide();
			m_comboNewLine.hide();
		}
	}

	/*
	 *
	 */
	void init_combo_encodgins()
	{
		m_comboEncodings.clear();
		// TODO : use clear_items for GTK+ 2.8
		//m_comboEncodings.clear_items();

		if(m_extra & EXTRA_ENCODING_WITH_AUTO_DETECTED)
			m_comboEncodings.append_text(_("Auto Detected"));

		Config &cfg = Config::getInstance();

		std::list<Glib::ustring> list_encodgins;
		if(cfg.get_value_string_list("encodings", "encodings", list_encodgins))
		{
			std::list<Glib::ustring>::const_iterator it;
			for(it = list_encodgins.begin(); it!=list_encodgins.end(); ++it)
			{
				EncodingInfo *info= Encodings::get_from_charset(*it);
				if(info)
				{
					gchar *name = g_strdup_printf("%s (%s)", info->name, info->charset);		
					m_comboEncodings.append_text(name);
					g_free(name);
				}
			}
		}

		if(m_extra & EXTRA_ENCODING_WITH_AUTO_DETECTED)
		{
			bool used_auto_detected = false;
			if(cfg.get_value_bool("encodings", "used-auto-detected", used_auto_detected))
			{
				if(used_auto_detected)
					m_comboEncodings.set_active(0);
				else
					m_comboEncodings.set_active(1);
			}
		}
		else
		{
			m_comboEncodings.set_active(0);
		}
	}

	/*
	 *
	 */
	Glib::ustring getEncoding()
	{
		if(m_comboEncodings.get_active() == 0)
			return "";

		Glib::ustring text = m_comboEncodings.get_active_text();
		
		Glib::ustring::size_type a = text.find('(');
		Glib::ustring::size_type b = text.find(')', a);

		if(a == Glib::ustring::npos || b == Glib::ustring::npos)
			return "";
		
		return text.substr(a+1,b-a-1);
	}

	/*
	 *
	 */
	Glib::ustring getFormat()
	{
		return m_comboFormat.get_active_text();
	}

	/*
	 *
	 */
	Glib::ustring getNewLine()
	{
		return m_comboNewLine.get_active_text();
	}

	/*
	 *
	 */
	void on_config_changed(const Glib::ustring &key, const Glib::ustring &value)
	{
		init_combo_encodgins();
	}
protected:
	int	m_extra;
	Gtk::Label				m_labelEncodings;
	Gtk::ComboBoxText m_comboEncodings;

	Gtk::Label				m_labelFormat;
	Gtk::ComboBoxText	m_comboFormat;

	Gtk::Label				m_labelNewLine;
	Gtk::ComboBoxText m_comboNewLine;
};



 

/*
 *
 */
DialogFileChooser::DialogFileChooser(const Glib::ustring &title, const Glib::ustring &dialog_name, 
		Gtk::FileChooserAction action, int ext)
:Gtk::FileChooserDialog(title, action), m_name(dialog_name)
{
	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	set_default_response(Gtk::RESPONSE_OK);

	m_fileChooserExtra = manage(new FileChooserExtra(ext));
	set_extra_widget(*m_fileChooserExtra);

	loadConfig(m_name);
}

DialogFileChooser::~DialogFileChooser()
{
	saveConfig(m_name);
}

/*
 *
 */
Glib::ustring DialogFileChooser::getEncoding()
{
	Glib::ustring text = m_fileChooserExtra->getEncoding();
	// TODO : remove
	if(text == "Auto Detected")
		return "";
	
	return text;
}

/*
 * return format saving (SSA,ASS,SUBRIP,...)
 */
Glib::ustring DialogFileChooser::getFormat()
{
	return m_fileChooserExtra->getFormat();
}

/*
 *
 */
Glib::ustring DialogFileChooser::getNewLine()
{
	return m_fileChooserExtra->getNewLine();
}

/*
 * exemple :
 * name = "All Format Supported"
 * patterns = "*.ssa;*.ass;*.sub;*.txt"
 */
void DialogFileChooser::addFilter(const Glib::ustring &name, const Glib::ustring &patterns)
{
	Gtk::FileFilter filter;
	
	filter.set_name(name);
	
	std::istringstream iss(patterns);
	std::string word;
	while( std::getline(iss, word, ';') )
	{
		filter.add_pattern(word);
	}

	add_filter(filter);
}

/*
 *
 */
void DialogFileChooser::show_filter()
{
	std::list<Glib::ustring> formats = SubtitleSystem::getInstance().get_formats();

	std::list<Glib::ustring>::const_iterator it;
	
	// all supported formats
	Glib::ustring all_supported;
	for(it=formats.begin(); it!=formats.end(); ++it)
	{
		Glib::ustring ext = SubtitleSystem::getInstance().get_extension(*it);
		all_supported += "*."+ext+";";
	}
	
	addFilter("All supported formats (*.ass, *.ssa, *.srt, ...)", all_supported);

	// format
	for(it=formats.begin(); it!=formats.end(); ++it)
	{
		Glib::ustring ext = "*." + SubtitleSystem::getInstance().get_extension(*it);
		
		Glib::ustring name = (*it) + " (" + ext + ")";

		addFilter(name, ext);
	}

	// all files
	addFilter("All files (*.*)", "*");
}

/*
 *
 */
void DialogFileChooser::loadConfig(const Glib::ustring &name)
{
	Config &cfg = Config::getInstance();

	Glib::ustring floder;
	if(cfg.get_value_string("dialog-last-folder", name, floder))
		set_current_folder_uri(floder);
}

/*
 *
 */
void DialogFileChooser::saveConfig(const Glib::ustring &name)
{
	Config &cfg = Config::getInstance();

	Glib::ustring floder = get_current_folder_uri();
	
	cfg.set_value_string("dialog-last-folder", name, floder);
}
