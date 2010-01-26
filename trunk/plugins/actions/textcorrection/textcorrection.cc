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

#include <extension/action.h>
#include <utility.h>
#include <gtkmm_utility.h>
#include <gtkmm.h>
#include <filereader.h>
#include "patternmanager.h"
#include "taskspage.h"
#include "hearingimpairedpage.h"
#include "commonerrorpage.h"
#include "capitalizationpage.h"
#include "confirmationpage.h"

/*
 *
 */
class AssistantTextCorrection : public Gtk::Assistant
{
public:
	AssistantTextCorrection(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
	:Gtk::Assistant(cobject)
	{
		se_debug(SE_DEBUG_PLUGINS);

		doc = SubtitleEditorWindow::get_instance()->get_current_document();

		builder->get_widget_derived("vbox-tasks", m_tasksPage);
		builder->get_widget_derived("vbox-comfirmation", m_comfirmationPage);

		add_tasks();

		se_debug_message(SE_DEBUG_PLUGINS, "Init tasks pages");
		// Init tasks pages
		for(int i=0; i< get_n_pages(); ++i)
		{
			PatternsPage* page = dynamic_cast<PatternsPage*>(get_nth_page(i));
			if(page)
				m_tasksPage->add_task(page);
		}
	}

	/*
	 */
	~AssistantTextCorrection()
	{
		se_debug(SE_DEBUG_PLUGINS);
	}

	/*
	 */
	void add_tasks()
	{
		se_debug(SE_DEBUG_PLUGINS);

		add_page(manage(new HearingImpairedPage), 1);
		add_page(manage(new CommonErrorPage), 2);
		add_page(manage(new CapitalizationPage), 3);
	}

	/*
	 */
	void add_page(PatternsPage *page, unsigned int pos)
	{
		se_debug_message(SE_DEBUG_PLUGINS, "new task page '%s' to the position '%d'", page->get_page_title().c_str(), pos);

		insert_page(*page, pos);
		set_page_title(*page, page->get_page_title());
	}

	/*
	 * Catch the comfirmation page and initialize with the current document
	 * and patterns available.
	 */
	void on_prepare(Gtk::Widget* page)
	{
		se_debug(SE_DEBUG_PLUGINS);

		AssistantPage* ap = dynamic_cast<AssistantPage*>(page);
		if(ap && ap == m_comfirmationPage)
		{
			bool res = m_comfirmationPage->comfirme(doc, get_patterns());
			set_page_complete(*page, res);
			set_page_title(*page, m_comfirmationPage->get_page_title());
		}
		else
			set_page_complete(*page, true);
	}

	/*
	 * Return all patterns activated.
	 */
	std::list<Pattern*> get_patterns()
	{
		se_debug(SE_DEBUG_PLUGINS);

		std::list<Pattern*> patterns;

		for(int i=0; i< get_n_pages(); ++i)
		{
			PatternsPage* page = dynamic_cast<PatternsPage*>(get_nth_page(i));
			if(page == NULL)
				continue;
			if(page->is_enable() == false)
				continue;

			std::list<Pattern*> p = page->get_patterns();
			patterns.merge(p);
		}
		return patterns;
	}

	/*
	 * Apply the change and destroy the window.
	 */
	void on_apply()
	{
		se_debug(SE_DEBUG_PLUGINS);

		m_comfirmationPage->apply(doc);
		
		se_debug_message(SE_DEBUG_PLUGINS, "save config...");

		save_cfg();
	}

	/*
	 * Destroy the window.
	 */
	void on_cancel()
	{
		se_debug(SE_DEBUG_PLUGINS);
		//destroy_();
		delete this;
	}

	/*
	 * Close signal, destroy the window.
	 */
	void on_close()
	{
		se_debug(SE_DEBUG_PLUGINS);
		//destroy_();
		delete this;
	}

	/*
	 * Save the configuration for each pages.
	 */
	void save_cfg()
	{
		se_debug(SE_DEBUG_PLUGINS);

		for(int i=0; i< get_n_pages(); ++i)
		{
			PatternsPage* page = dynamic_cast<PatternsPage*>(get_nth_page(i));
			if(page != NULL)
				page->save_cfg();
		}
	}

protected:
	TasksPage* m_tasksPage;
	ComfirmationPage* m_comfirmationPage;
	Document* doc;
};

/*
 *
 */
class TextCorrectionPlugin : public Action
{
public:

	/*
	 *
	 */
	TextCorrectionPlugin()
	{
		activate();
		update_ui();
	}

	/*
	 *
	 */
	~TextCorrectionPlugin()
	{
		deactivate();
	}

	/*
	 *
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("TextCorrectionPlugin");
		action_group->add(
				Gtk::Action::create("text-correction", _("Text _Correction")),
					sigc::mem_fun(*this, &TextCorrectionPlugin::on_execute));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();
		ui->insert_action_group(action_group);
		ui->add_ui(ui_id, "/menubar/menu-tools/checking", "text-correction", "text-correction");
	}

	/*
	 *
	 */
	void deactivate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->remove_ui(ui_id);
		ui->remove_action_group(action_group);
	}

	/*
	 *
	 */
	void update_ui()
	{
		se_debug(SE_DEBUG_PLUGINS);

		bool visible = (get_current_document() != NULL);

		action_group->get_action("text-correction")->set_sensitive(visible);
	}

	/*
	 *
	 */
	void on_execute()
	{
		// create dialog
		/*
		std::auto_ptr<AssistantTextCorrection> assistant(
				gtkmm_utility::get_widget_derived<AssistantTextCorrection>(
						SE_DEV_VALUE(SE_PLUGIN_PATH_UI, SE_PLUGIN_PATH_DEV),
						"assistant-text-correction.ui", 
						"assistant"));
		*/
		AssistantTextCorrection *assistant = 
			gtkmm_utility::get_widget_derived<AssistantTextCorrection>(
						SE_DEV_VALUE(SE_PLUGIN_PATH_UI, SE_PLUGIN_PATH_DEV),
						"assistant-text-correction.ui", 
						"assistant");
		//assistant->set_document(document());
		assistant->show();
		//Gtk::Main::run(*assistant);
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(TextCorrectionPlugin)
