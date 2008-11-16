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

#include "ComboBoxEncoding.h"
#include "Encodings.h"
#include "DialogCharacterCodings.h"
#include "Config.h"


/*
 * Constructor
 */
ComboBoxEncoding::ComboBoxEncoding(bool auto_detected)
:m_with_auto_detected(auto_detected)
{
	init_encodings();

	// separator function
	Gtk::ComboBoxText::set_row_separator_func(
			sigc::mem_fun(*this, &ComboBoxEncoding::on_row_separator_func));

	// m_connection_changed is need to disable the signal when the combobox is rebuild.
	m_connection_changed = signal_changed().connect(
			sigc::mem_fun(*this, &ComboBoxEncoding::on_combo_changed));
}

/*
 * Constructor
 */
ComboBoxEncoding::ComboBoxEncoding(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:Gtk::ComboBoxText(cobject), m_with_auto_detected(true)
{
	init_encodings();

	// separator function
	Gtk::ComboBoxText::set_row_separator_func(
			sigc::mem_fun(*this, &ComboBoxEncoding::on_row_separator_func));

	// m_connection_changed is need to disable the signal when the combobox is rebuild.
	m_connection_changed = signal_changed().connect(
			sigc::mem_fun(*this, &ComboBoxEncoding::on_combo_changed));
}

/*
 * Sets current value.
 */
void ComboBoxEncoding::set_value(const Glib::ustring &value)
{
	Glib::ustring label = Encodings::get_label_from_charset(value);

	if(label.empty())
		return;
	// check only with encoding available
	Gtk::TreeIter it = get_model()->children().begin();
	while(it)
	{
		Glib::ustring name = (*it)[m_text_columns.m_column];
		if(name == label)
		{
			set_active(it);
			return;
		}
		++it;
	}
	//set_active_text(label);
}

/*
 * Returns only the charset value.
 * ex: "UTF-8", "ISO-8859-15" ...
 * Return empty charset if it's "Auto Detected".
 */
Glib::ustring ComboBoxEncoding::get_value() const
{
	if(m_with_auto_detected)
	{
		if(get_active_row_number() == 0)
			return Glib::ustring(); // "None"
	}
	// extract the charset 
	// ex:
	// "Unicode (UTF-8)"
	Glib::ustring text = get_active_text();

	Glib::ustring::size_type a = text.find('(');
	Glib::ustring::size_type b = text.find(')', a);

	if(a != Glib::ustring::npos && b != Glib::ustring::npos)
		return text.substr(a+1, b-a-1);

	return Glib::ustring();
}

/*
 * Enable or disable the auto detected mode.
 */
void ComboBoxEncoding::show_auto_detected(bool value)
{
	m_with_auto_detected = value;

	bool state = is_sensitive();
	set_sensitive(false);

	init_encodings();

	set_sensitive(state);
}


/*
 * Rebuild the combobox with encoding user preferences.
 */
void ComboBoxEncoding::init_encodings()
{
	m_connection_changed.block();

	clear();

	bool used_auto_detected = Config::getInstance().get_value_bool("encodings", "used-auto-detected");

	if(m_with_auto_detected)
	{
		append_text(_("Auto Detected"));
		append_text("<separator>");
	}

	std::list<Glib::ustring> encodings = 
		Config::getInstance().get_value_string_list("encodings", "encodings");

	if(encodings.empty())
	{
		std::string charset;
		Glib::get_charset(charset);

		Glib::ustring item;
		item += _("Current Locale");
		item += " (";
		item += charset;
		item += ")";

		append_text(item);
	}
	else
	{
		std::list<Glib::ustring>::const_iterator it;
		for(it = encodings.begin(); it != encodings.end(); ++it)
		{
			Glib::ustring label = Encodings::get_label_from_charset(*it);
			if(!label.empty())
			{
				append_text(label);
			}
		}
	}
	// configure
	append_text("<separator>");
	append_text(_("Add or Remove..."));

	if(m_with_auto_detected)
	{
		if(used_auto_detected)
			set_active(0);
		else
			set_active(2); // auto detected (0), separator (1), first charset (2)
	}
	else
		set_active(0);

	m_connection_changed.unblock();
}

/*
 * Gtk::ComboBox::on_changed
 * Used for intercepte "Add or Remove..."
 */
void ComboBoxEncoding::on_combo_changed()
{
	unsigned int size = get_model()->children().size();
	unsigned int activated = get_active_row_number();

	if(activated == size-1)
	{
		std::auto_ptr<DialogCharacterCodings> dialog = DialogCharacterCodings::create();
		if(dialog->run() == Gtk::RESPONSE_OK)
		{
			init_encodings();
		}
		else if(m_with_auto_detected)
		{
			if(Config::getInstance().get_value_bool("encodings", "used-auto-detected"))
				set_active(0);
			else
				set_active(2);
		}
		else
			set_active(0);
	}
}

/*
 * Used to define the separator.
 * label = "<separator>"
 */
bool ComboBoxEncoding::on_row_separator_func(const Glib::RefPtr<Gtk::TreeModel> &model, const Gtk::TreeModel::iterator &it)
{
	Glib::ustring text = (*it)[m_text_columns.m_column];
	if(text == "<separator>")
		return true;
	return false;
}

