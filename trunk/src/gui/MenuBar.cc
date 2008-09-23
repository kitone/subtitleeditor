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
 

#include "MenuBar.h"
#include "utility.h"
#include "DocumentSystem.h"
#include "Config.h"
#include "gui/PreferencesUI.h"

/*
 *
 */
class ActionGroup
{
public:
	ActionGroup(const Glib::ustring &name, Glib::RefPtr<Gtk::UIManager> ui)
	{
		ag = Gtk::ActionGroup::create(name);
		ui->insert_action_group(ag);
	}


	void menu_item(const Glib::ustring &name, const Glib::ustring &label = Glib::ustring())
	{
		ag->add(Gtk::Action::create(name, label));
	}


	void menu_item(const Glib::ustring &name, const Gtk::StockID &stock_id, const Glib::ustring &label = Glib::ustring())
	{
		ag->add(Gtk::Action::create(name, stock_id, label));
	}

	void item(	const Glib::ustring &name, 
							const Gtk::StockID &stock_id, 
							const Glib::ustring &label = Glib::ustring(), 
							const Glib::ustring &tooltip = Glib::ustring(), 
							const Glib::ustring &accel = Glib::ustring())
	{
		if(accel.empty())
			ag->add(Gtk::Action::create(name, stock_id, label, tooltip));
		else
			ag->add(Gtk::Action::create(name, stock_id, label, tooltip), Gtk::AccelKey(accel));
	}

	void item(	const Glib::ustring &name, 
							const Glib::ustring &label = Glib::ustring(), 
							const Glib::ustring &tooltip = Glib::ustring(), 
							const Glib::ustring &accel = Glib::ustring())
	{
		if(accel.empty())
			ag->add(Gtk::Action::create(name, label, tooltip));
		else
			ag->add(Gtk::Action::create(name, label, tooltip), Gtk::AccelKey(accel));
	}

protected:
	Glib::RefPtr<Gtk::ActionGroup> ag;
};


/*
 *
 */
MenuBar::MenuBar()
:Gtk::VBox(false, 0), m_statusbar(NULL)
{
	m_refUIManager = Gtk::UIManager::create();
}

/*
 *
 */
void MenuBar::connect_proxy(const Glib::RefPtr<Gtk::Action> &action, Gtk::Widget *widget)
{
	if(Gtk::MenuItem *item = dynamic_cast<Gtk::MenuItem*>(widget))
	{
		Glib::ustring tooltip = action->property_tooltip();

		item->signal_select().connect(
				sigc::bind(
					sigc::mem_fun(m_statusbar, &Statusbar::push_text), tooltip));
		item->signal_deselect().connect(
				sigc::mem_fun(m_statusbar, &Statusbar::pop_text));
	}
}

/*
 *
 */
void MenuBar::create(Gtk::Window &window, Statusbar &statusbar)
{
	m_statusbar = &statusbar;

	m_refActionGroup = Gtk::ActionGroup::create("default");

	// menu-file
	{
		ActionGroup ag("file", m_refUIManager);

		ag.item("menu-file", _("_File"));

		ag.item("properties", _("_Properties"),	"");
	}
	
	// menu-edit
	{
		ActionGroup ag("edit", m_refUIManager);

		ag.item("menu-edit", _("_Edit"));
	
		//ag.item("cut", Gtk::Stock::CUT, "", _(""));
		//ag.item("copy", Gtk::Stock::COPY, "", _(""));
		//ag.item("paste", Gtk::Stock::PASTE, "", _(""));
	}

	// timings
	{
			ActionGroup ag("timings", m_refUIManager);

			ag.item("menu-timings", _("_Timings"));
	}

	// menu-tools
	{
		ActionGroup ag("tools", m_refUIManager);

		ag.item("menu-tools", _("T_ools"));
	}


	// menu-view
	{
		ActionGroup ag("view", m_refUIManager);

		ag.item("menu-view", _("V_iew"));
	}
	
	// menu-option
	{
		ActionGroup ag("options", m_refUIManager);

		ag.item("menu-options", _("_Options"));
		ag.item("preferences", Gtk::Stock::PREFERENCES, "",
				_("Configure Subtitle Editor"));
	}

	// menu-help
	{
		ActionGroup ag("help", m_refUIManager);

		ag.item("menu-help", _("_Help"));
	}

	show_all();

	// UIManager

	// on connect tous les groups au callback
	std::vector<Glib::RefPtr<Gtk::ActionGroup> > ags = m_refUIManager->get_action_groups();
	for(unsigned int i=0; i < ags.size(); ++i)
	{
		ags[i]->signal_pre_activate().connect(
				sigc::mem_fun(*this, &MenuBar::action_activate));
	}


	m_refUIManager->signal_connect_proxy().connect(
			sigc::mem_fun(*this, &MenuBar::connect_proxy));

	m_refUIManager->insert_action_group(m_refActionGroup);

	window.add_accel_group(m_refUIManager->get_accel_group());

	create_ui_from_file();

#warning "FIXME: properties"
	set_sensitive("properties", false);
}

/*
 *
 */
void MenuBar::create_ui_from_file()
{
	m_refUIManager->add_ui_from_file(get_share_dir("menubar.xml"));

	pack_start(*m_refUIManager->get_widget("/menubar"), false, false);

	show_all();
}

/*
 *
 */
MenuBar::~MenuBar()
{
}

/*
 *
 */
void MenuBar::action_activate(const Glib::RefPtr<Gtk::Action> action)
{
	execute(action->get_name());
}

/*
 *
 */
void MenuBar::execute(const Glib::ustring &name)
{
	if(name == "preferences")
		on_preferences();
}

/*
 *
 */
void MenuBar::on_preferences()
{
	PreferencesUI*	dialog = utility::get_widget_derived<PreferencesUI>("dialog-preferences.glade", "dialog-preferences");

	dialog->run();

	delete dialog;
}

/*
 *
 */
void MenuBar::set_sensitive(const Glib::ustring &name, bool state)
{
	std::vector<Glib::RefPtr<Gtk::ActionGroup> > ags = m_refUIManager->get_action_groups();
	for(unsigned int i=0; i < ags.size(); ++i)
	{
		Glib::RefPtr<Gtk::Action> action = ags[i]->get_action(name);
		if(action)
		{
			action->set_sensitive(state);
			return;
		}
	}
}

