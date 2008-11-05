#ifndef _ErrorCheckingPreferences_h
#define _ErrorCheckingPreferences_h

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

#include "ErrorChecking.h"
#include <memory>

/*
 *
 */
class DialogErrorCheckingPreferences : public Gtk::Dialog
{
	class Column : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Column()
		{
			add(enabled);
			add(label);
			add(name);
			add(checker);
		}
		Gtk::TreeModelColumn<bool> enabled; // is activated ?
		Gtk::TreeModelColumn<Glib::ustring> label; // human label
		Gtk::TreeModelColumn<Glib::ustring> name; // internal name
		Gtk::TreeModelColumn<ErrorChecking*> checker; // internal name
	};

public:
	DialogErrorCheckingPreferences(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::Dialog(cobject)
	{
		utility::set_transient_parent(*this);

		refGlade->get_widget("treeview-plugins", m_treeviewPlugins);
		refGlade->get_widget("button-about", m_buttonAbout);
		refGlade->get_widget("button-preferences", m_buttonPreferences);

		get_and_init_widget(refGlade, "spin-min-characters-per-second", "timing", "min-characters-per-second");
		get_and_init_widget(refGlade, "spin-max-characters-per-second", "timing", "max-characters-per-second");
		get_and_init_widget(refGlade, "spin-min-gap-between-subtitles", "timing", "min-gap-between-subtitles");
		get_and_init_widget(refGlade, "spin-min-display", "timing", "min-display");
		get_and_init_widget(refGlade, "spin-max-characters-per-line", "timing", "max-characters-per-line");
		get_and_init_widget(refGlade, "spin-max-line-per-subtitle", "timing", "max-line-per-subtitle");

		create_treeview();

		m_buttonPreferences->signal_clicked().connect(
				sigc::mem_fun(*this, &DialogErrorCheckingPreferences::on_checker_preferences));

		// set default sensitive
		m_buttonAbout->set_sensitive(false);
		m_buttonPreferences->set_sensitive(false);
	}

	/*
	 *
	 */
	static void create(Gtk::Window &parent, std::vector<ErrorChecking*> &list)
	{
		std::auto_ptr<DialogErrorCheckingPreferences> dialog(
				utility::get_widget_derived<DialogErrorCheckingPreferences>(
					(Glib::getenv("SE_DEV") == "") ? SE_PLUGIN_PATH_GLADE : SE_PLUGIN_PATH_DEV,
					"dialog-error-checking-preferences.glade", 
					"dialog-error-checking-preferences"));

		dialog->set_transient_for(parent);
		dialog->init_treeview(list);
		dialog->run();
	}
	/*
	 *
	 */
	void get_and_init_widget( 
			const Glib::RefPtr<Gnome::Glade::Xml>& refGlade, 
			const Glib::ustring &widget_name, 
			const Glib::ustring &config_group,
			const Glib::ustring &config_key)
	{
		Gtk::Widget *widget =NULL;

		refGlade->get_widget(widget_name, widget);

		WidgetToConfig::read_config_and_connect(widget, config_group, config_key);
	}

	/*
	 *
	 */
	void create_treeview()
	{
		// create the model
		m_model = Gtk::ListStore::create(m_column);
		m_treeviewPlugins->set_model(m_model);

		Gtk::TreeViewColumn* column = NULL;
		Gtk::CellRendererToggle* toggle;
		Gtk::CellRendererText* renderer = NULL;

		// enabled column
		column = manage(new Gtk::TreeViewColumn);
		m_treeviewPlugins->append_column(*column);

		// 
		toggle = manage(new Gtk::CellRendererToggle);
		toggle->signal_toggled().connect(
				sigc::mem_fun(*this, &DialogErrorCheckingPreferences::on_enabled_toggled));
		column->pack_start(*toggle, false);
		column->add_attribute(toggle->property_active(), m_column.enabled);

		// label column
		column = manage(new Gtk::TreeViewColumn);
		m_treeviewPlugins->append_column(*column);
		// 
		renderer = manage(new Gtk::CellRendererText);
		column->pack_start(*renderer, true);
		column->add_attribute(renderer->property_markup(), m_column.label);

		// set treeview property
		m_treeviewPlugins->set_rules_hint(true);
		m_treeviewPlugins->get_selection()->signal_changed().connect(
				sigc::mem_fun(*this, &DialogErrorCheckingPreferences::on_checker_selection_changed));
		m_treeviewPlugins->show_all();
	}

	/*
	 *
	 */
	void init_treeview(std::vector<ErrorChecking*> &list)
	{
		std::vector<ErrorChecking*>::iterator checker;

		for(checker = list.begin(); checker != list.end(); ++checker)
		{
			Gtk::TreeIter it = m_model->append();
			(*it)[m_column.enabled] = (*checker)->get_active();
			(*it)[m_column.name] = (*checker)->get_name();
			(*it)[m_column.label] = build_message("<b>%s</b>\n%s", (*checker)->get_label().c_str(), (*checker)->get_description().c_str());
			(*it)[m_column.checker] = (*checker);
		}
	}

	/*
	 *
	 */
	void on_enabled_toggled(const Glib::ustring &path)
	{
		Gtk::TreeIter it = m_model->get_iter(path);
		if(it)
		{
			ErrorChecking* checker = (*it)[m_column.checker];

			(*it)[m_column.enabled] = !bool((*it)[m_column.enabled]);

			// save on config
			checker->set_active((*it)[m_column.enabled]);
		}
	}

	/*
	 *
	 */
	void on_checker_selection_changed()
	{
		Gtk::TreeIter it = m_treeviewPlugins->get_selection()->get_selected();

		if(it)
		{
			ErrorChecking* checker = (*it)[m_column.checker];
			
			if(checker == NULL)
				return;

			bool has_config = checker->has_configuration();

			m_buttonPreferences->set_sensitive(has_config);
			m_buttonAbout->set_sensitive(false);
		}
		else
		{
			m_buttonPreferences->set_sensitive(false);
			m_buttonAbout->set_sensitive(false);
		}
	}

	/*
	 *
	 */
	void on_checker_preferences()
	{
		Gtk::TreeIter it = m_treeviewPlugins->get_selection()->get_selected();

		if(!it)
			return;

		ErrorChecking* checker = (*it)[m_column.checker];
			
		if(checker == NULL)
			return;

		checker->create_configuration();
	}
protected:

	Gtk::TreeView* m_treeviewPlugins;
	Gtk::Button* m_buttonPreferences;
	Gtk::Button* m_buttonAbout;
	Glib::RefPtr<Gtk::ListStore> m_model;
	Column m_column;
};

#endif//_ErrorCheckingPreferences_h

