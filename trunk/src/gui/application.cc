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

#include <config.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/accelmap.h>
#include "utility.h"
#include "application.h"
#include "documentsystem.h"
#include <algorithm>
#include "encodings.h"

#include "extensionmanager.h"
#include "extension.h"
#include "extension/action.h"

/*
 *
 */
Application::Application(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& builder)//int argc, char *argv[])
:Gtk::Window(cobject)
{
	builder->get_widget_derived("statusbar", m_statusbar);

	builder->get_widget("vbox-main", m_vboxMain);
	builder->get_widget("paned-multimedia", m_paned_multimedia);
	builder->get_widget_derived("video-player", m_video_player);
	builder->get_widget_derived("waveform-editor", m_waveform_editor);
	builder->get_widget("notebook-documents", m_notebook_documents);

	init_panel_multimedia();

	set_default_size(800,600);

	Gtk::Window::set_default_icon_name("subtitleeditor");

	m_vboxMain->pack_start(m_menubar, false, false);
	m_vboxMain->reorder_child(m_menubar, 0);

	m_menubar.show_all();

	DocumentSystem::getInstance().signal_document_create().connect(
			sigc::mem_fun(*this, &Application::on_document_create));

	DocumentSystem::getInstance().signal_document_delete().connect(
			sigc::mem_fun(*this, &Application::on_document_delete));

	
	DocumentSystem::getInstance().signal_current_document_changed().connect(
			sigc::mem_fun(*this, &Application::on_current_document_changed));


	m_notebook_documents->signal_switch_page().connect(
			sigc::mem_fun(*this, &Application::on_signal_switch_page));

	// on va chercher la configuration clavier
	Glib::ustring path_se_accelmap = get_config_dir("accelmap");
	Gtk::AccelMap::load(path_se_accelmap);

	//
	Config::getInstance().signal_changed("interface").connect(
			sigc::mem_fun(*this, &Application::on_config_interface_changed));

	show();

	m_menubar.create(*this, *m_statusbar);

	ExtensionManager::instance().create_extensions();
	
	load_config();

	// open subtitle files with drag-and-drop in NoteBook
	{
		std::list<Gtk::TargetEntry> targets;

		targets.push_back(Gtk::TargetEntry("text/uri-list", Gtk::TargetFlags(0), 0));
		
		m_notebook_documents->signal_drag_data_received().connect(
				sigc::mem_fun(*this, &Application::notebook_drag_data_received));
		m_notebook_documents->drag_dest_set(targets, Gtk::DEST_DEFAULT_ALL, Gdk::DragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));
	}
}

/*
 *
 */
Application::~Application()
{
	se_debug(SE_DEBUG_APP);

	Glib::ustring path_se_accelmap = get_config_dir("accelmap");
	Gtk::AccelMap::save(path_se_accelmap);

	ExtensionManager::instance().destroy_extensions();
}

/*
 *
 */
void Application::load_config()
{
	//
	// Config
	//
	Config &cfg = Config::getInstance();

	// dynamic keyboar shorcuts
	bool value;
	cfg.get_value_bool("interface", "use-dynamic-keyboard-shortcuts", value);
	Gtk::Settings::get_default()->property_gtk_can_change_accels() = value;

	// maximize window
	cfg.get_value_bool("interface", "maximize-window", value);
	if(value)
		maximize();

	// first launch
	if(!cfg.has_group("encodings"))
	{
		cfg.set_value_string("encodings", "encodings", "ISO-8859-15;UTF-8");
		cfg.set_value_bool("encodings", "used-auto-detected", true);
	}
}

/*
 *
 */
bool Application::on_delete_event(GdkEventAny *ev)
{
	se_debug(SE_DEBUG_APP);

	bool res = Gtk::Window::on_delete_event(ev);

	Glib::ustring path_se_accelmap = get_config_dir("accelmap");
	Gtk::AccelMap::save(path_se_accelmap);

	return res;
}

/*
 *	il y a la création d'un nouveau document
 *	on l'ajoute dans le notebook
 *	signal emit par DocumentSystem::signal_document_create
 */
void Application::on_document_create(Document *doc)
{
	g_return_if_fail(doc);

	Glib::ustring filename = doc->getName();

	//
	Gtk::HBox *hbox = NULL;
	Gtk::Image *image = NULL;
	Gtk::Button *close_button = NULL;
	Gtk::EventBox *eventbox = NULL;
	Gtk::Label* label = NULL;

	// hbox
	hbox = manage(new Gtk::HBox(false, 4));
	hbox->set_border_width(0);

	// event box
	eventbox = manage(new Gtk::EventBox);
	eventbox->set_visible_window(false);
	hbox->pack_start(*eventbox, true, true, 0);

	// label
	label = manage(new Gtk::Label(filename));
	label->set_use_markup(true);
	label->set_alignment(0.0, 0.5);
	label->set_padding(0, 0);
	eventbox->add(*label);

	// close button
	close_button = manage(new Gtk::Button);
	close_button->set_relief(Gtk::RELIEF_NONE);
	close_button->set_focus_on_click(false);
	close_button->set_border_width(0);

	close_button->signal_clicked().connect(
			sigc::bind<Document*>(
				sigc::mem_fun(*this, &Application::on_close_document), doc));

	// rcstyle for the button
	close_button->get_modifier_style()->set_xthickness(0);
	close_button->get_modifier_style()->set_ythickness(0);
	
	// close image 
	image = manage(new Gtk::Image(Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU));
	close_button->add(*image);
	hbox->pack_start(*close_button, false, false, 0);

	// show all	
	hbox->show_all();


	//
	int id = 0;
	Gtk::ScrolledWindow *scroll = NULL;
	Gtk::Widget *page = NULL;


	scroll = manage(new Gtk::ScrolledWindow);
	scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scroll->add(*doc->widget());
	scroll->show();
	

	id = m_notebook_documents->append_page(*scroll, *hbox);
	m_notebook_documents->set_current_page(id);


	page = m_notebook_documents->get_nth_page(id);

	page->set_data("event", eventbox);
	page->set_data("document", doc);
	page->set_data("label", label);

	doc->get_signal("document-changed").connect(
			sigc::bind<Document*>(
				sigc::mem_fun(*this, &Application::on_document_changed), doc));

	doc->get_signal("document-property-changed").connect(
			sigc::bind<Document*>(
				sigc::mem_fun(*this, &Application::update_document_property), doc));


	update_document_property(doc);

	DocumentSystem::getInstance().setCurrentDocument(doc);

	connect_document(doc);

	// Update UI
	while(Gtk::Main::events_pending())
		Gtk::Main::iteration();
}

/*
 *	retourne le widget (notebook) par rapport au document
 */
Gtk::Widget* Application::get_widget(Document *doc)
{
	for( int i = 0; i < m_notebook_documents->get_n_pages(); ++i)
	{
		Gtk::Widget *w = m_notebook_documents->get_nth_page(i);

		Document *document = (Document*)w->get_data("document");

		if(document == doc)
			return w;
	}
	return NULL;
}

/*
 *	mise a jour d'info (filename, tooltip, ...) dans la page
 */
void Application::update_document_property(Document *doc)
{
	Gtk::Widget *widget = get_widget(doc);
	g_return_if_fail(widget);

	Gtk::Label *label = (Gtk::Label*)widget->get_data("label");
	g_return_if_fail(label);

	// Update the document name
	Glib::ustring display_name = (doc->get_document_changed() ? "*" : "") + doc->getName();

	// Update the document property (tooltip)
	Glib::ustring name = doc->getName();
	Glib::ustring dir = Glib::path_get_dirname(doc->getFilename());

	Glib::ustring character_coding = Encodings::get_label_from_charset(doc->getCharset());
	Glib::ustring format = doc->getFormat();
	Glib::ustring newline = doc->getNewLine();
	Glib::ustring timing_mode = (doc->get_timing_mode() == TIME) ? _("Times") : _("Frames");

	Glib::ustring tip = build_message(
			"<b>%s</b> %s\n"
			"<b>%s</b> %s\n\n"
			"<b>%s</b> %s\n"
			"<b>%s</b> %s\n"
			"<b>%s</b> %s\n"
			"<b>%s</b> %s",
			_("Name:"), name.c_str(),
			_("Path:"), dir.c_str(),
			_("Character Coding:"), character_coding.c_str(),
			_("Format:"), format.c_str(),
			_("Newline:"), newline.c_str(),
			_("Timing Mode:"), timing_mode.c_str());

	label->set_text(display_name);
	label->set_tooltip_markup(tip);
}

/*
 *	when the document has changed, update the name (*)name
 *	signal "document-changed"
 */
void Application::on_document_changed(Document *doc)
{
	update_document_property(doc);
}


/*
 *	on efface le document du notebook
 *	signal emit par DocumentSystem::signal_document_delete
 */
void Application::on_document_delete(Document *doc)
{
	if(doc == NULL)
		return;

	Gtk::Widget *widget = get_widget(doc);

	if(widget != NULL)
		m_notebook_documents->remove_page(*widget);

}

/*
 * The current document has changed.
 * Needs to update the ui.
 */
void Application::on_current_document_changed(Document* doc)
{
	// Update page
	// First check if it's not already the good page
	int cur_id = m_notebook_documents->get_current_page();
	Gtk::Widget *cur_w = m_notebook_documents->get_nth_page(cur_id);

	Document* cur_doc = static_cast<Document*>(cur_w->get_data("document"));
	if(cur_doc != doc)
	{
		// This is not the good page, active it
		for( int i = 0; i < m_notebook_documents->get_n_pages(); ++i)
		{
			Gtk::Widget *w = m_notebook_documents->get_nth_page(i);
			Document *document = static_cast<Document*>(w->get_data("document"));
			if(document == doc)
			{
				m_notebook_documents->set_current_page(i);
				break;
			}
		}
	}
	// Update actions
	std::list<ExtensionInfo*> actions = ExtensionManager::instance().get_info_list_from_categorie("action");

	for(std::list<ExtensionInfo*>::iterator it = actions.begin(); it != actions.end(); ++it)
	{
		if((*it)->get_active() == false)
			continue;

		Action* action = dynamic_cast<Action*>((*it)->get_extension());
		if(action)
			action->update_ui();
	}
}

/*
 *
 */
void Application::on_close_document(Document *doc)
{
	se_debug(SE_DEBUG_APP);

	g_return_if_fail(doc);

	disconnect_document(doc);

	DocumentSystem::getInstance().remove(doc);
}

/*
 *	Changement dans le notebook de la page editer
 *	On recupere la page pusi on init DocumentManager avec le document
 */
void Application::on_signal_switch_page(GtkNotebookPage* page, guint page_num)
{
	se_debug(SE_DEBUG_APP);

	Gtk::Widget *w = m_notebook_documents->get_nth_page(page_num);
	
	if(w)
	{
		Document *doc = (Document*)w->get_data("document");

		if(doc)
		{
			DocumentSystem::getInstance().setCurrentDocument(doc);
			connect_document(doc);
		}
		else
		{
			DocumentSystem::getInstance().setCurrentDocument(NULL);
			disconnect_document(NULL);
		}
	}
}

/*
 *
 */
void Application::connect_document(Document *doc)
{
	se_debug(SE_DEBUG_APP);

	disconnect_document(doc);

	if(doc)
	{
		se_debug_message(SE_DEBUG_APP, "connect_document: %s", doc->getName().c_str());
		
		// connect document message
		m_document_connections.push_back(
				doc->get_signal_message().connect(
					sigc::mem_fun(m_statusbar, &Statusbar::push_text)));
	
		m_document_connections.push_back(
				doc->get_signal_flash_message().connect(
					sigc::mem_fun(m_statusbar, &Statusbar::flash_message)));

		update_title(doc);
	}
}

/*
 *
 */
void Application::disconnect_document(Document *doc)
{
	se_debug(SE_DEBUG_APP);

	update_title(NULL);

	if(doc)
	{
		se_debug_message(SE_DEBUG_APP, "disconnect_document: %s", doc->getName().c_str());
	}

	// clear old connection
	std::list<sigc::connection>::iterator it;
	for(it = m_document_connections.begin(); it!= m_document_connections.end(); ++it)
		(*it).disconnect();
	m_document_connections.clear();
}

/*
 *
 */
void Application::update_title(Document *doc)
{
	if(doc != NULL)
	{
		Glib::ustring name = doc->getName();
	
		Glib::ustring dirname = Glib::path_get_dirname(doc->getFilename());

		// replace home dir by ~
		{
			Glib::ustring home = Glib::get_home_dir();
			if(dirname.compare(0, home.length(), home) == 0)
				dirname.replace(0, home.length(), "~");
		}
		
		
		if(dirname.empty() || dirname == ".")
		{
			set_title(
					build_message("%s - %s", 
						name.c_str(), 
						PACKAGE));
		}
		else
		{
			set_title(
					build_message("%s (%s) - %s", 
						name.c_str(), 
						dirname.c_str(),
						PACKAGE));
		}
	}
	else
	{
		set_title(PACKAGE);
	}
}

/*
 *
 */
void Application::on_config_interface_changed(const Glib::ustring &key, const Glib::ustring &value)
{
	if(key == "use-dynamic-keyboard-shortcuts")
	{
		bool state;
		from_string(value, state);

	  Gtk::Settings::get_default()->property_gtk_can_change_accels() = state;
	}
	else if(key == "maximize-window")
	{
		bool state;
		from_string(value, state);

		if(state)
			maximize();
		else 
			unmaximize();
	}
}

/*
 *
 */
void Application::init(OptionGroup &options)
{
	se_debug(SE_DEBUG_APP);

	std::vector<Glib::ustring> files(options.files.size() + options.files_list.size());

	std::merge(
			options.files.begin(), options.files.end(),
			options.files_list.begin(), options.files_list.end(),
			files.begin());

	// files
	for(unsigned int i = 0; i< files.size(); ++i)
	{
		Glib::ustring filename = files[i];

		if(	Glib::file_test(filename, Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_REGULAR) && 
				Glib::file_test(filename, Glib::FILE_TEST_IS_DIR) == false)
		{
			Glib::ustring uri = Glib::filename_to_uri(utility::create_full_path(filename));
			
			Document *doc = Document::create_from_file(uri, options.encoding);
			if(doc)
			{
				DocumentSystem::getInstance().append(doc);
			}
		}
	}

	// ------------------------------------------------
	// video
	Glib::ustring video = options.video;

	// s'il n'y a pas de video et s'il n'y a qu'un seule fichier sous-titre
	// recherche une video par rapport au nom du sous-titre
	bool automatically_open_video;
	Config::getInstance().get_value_bool("video-player", "automatically-open-video", automatically_open_video);

	if(video.empty() && (options.files.size() == 1) && automatically_open_video)
	{
		Glib::ustring tmp = options.files[0];
		
		Glib::ustring::size_type dot = tmp.rfind('.');
		
		if(dot != Glib::ustring::npos)
		{
			tmp = tmp.substr(0, dot);

			if(Glib::file_test(tmp + ".mpg", Glib::FILE_TEST_EXISTS))
				video = tmp + ".mpg";
			else if(Glib::file_test(tmp + ".mpeg", Glib::FILE_TEST_EXISTS))
				video = tmp + ".mpeg";
			else if(Glib::file_test(tmp + ".avi", Glib::FILE_TEST_EXISTS))
				video = tmp + ".avi";
			else if(Glib::file_test(tmp + ".ogm", Glib::FILE_TEST_EXISTS))
				video = tmp + ".ogm";
			else if(Glib::file_test(tmp + ".mkv", Glib::FILE_TEST_EXISTS))
				video = tmp + ".mkv";
		}
	}
	
	// une vidéo ?
	// on connect le lecteur interne
	if(!video.empty())
	{
		try
		{
			Glib::ustring uri = Glib::filename_to_uri(utility::create_full_path(video));

			get_player()->open(uri);
		}
		catch(const Glib::Error &ex)
		{
			std::cerr << ex.what() << std::endl;
		}
	}

	// -----------------------------------------------------
	// waveform
	Glib::ustring waveform = options.waveform;
	if(waveform.empty() && (options.files.size() == 1))
	{
		Glib::ustring tmp = options.files[0];
		Glib::ustring::size_type dot = tmp.rfind('.');
		if(dot != Glib::ustring::npos)
		{
			tmp = tmp.substr(0, dot);

			if(Glib::file_test(tmp + ".wf", Glib::FILE_TEST_EXISTS))
				waveform = tmp + ".wf";
		}
	}

	if(!waveform.empty())
	{
		try
		{
			Glib::ustring uri = Glib::filename_to_uri(utility::create_full_path(waveform));

			get_waveform_manager()->open_waveform(uri);
		}
		catch(const Glib::Error &ex)
		{
			std::cerr << ex.what() << std::endl;
		}
	}
}

/*
 *
 */
bool Application::on_key_press_event(GdkEventKey *ev)
{
	return Gtk::Window::on_key_press_event(ev);
}


/*
 *
 */
void Application::notebook_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time)
{
	std::vector<Glib::ustring> uris = selection_data.get_uris();
	for(unsigned int i=0; i<uris.size(); ++i)
	{
		Glib::ustring filename = Glib::filename_from_uri(uris[i]);

		// verifie qu'il n'est pas déjà ouvert
		if(DocumentSystem::getInstance().getDocument(filename) != NULL)
			continue;

		Document *doc = Document::create_from_file(uris[i]);
		if(doc)
			DocumentSystem::getInstance().append(doc);
	}
}


/*
 *
 */
Glib::RefPtr<Gtk::UIManager> Application::get_ui_manager()
{
	return m_menubar.get_ui_manager();
}

/*
 *
 */
Document* Application::get_current_document()
{
	return DocumentSystem::getInstance().getCurrentDocument();
}

/*
 *
 */
DocumentList Application::get_documents()
{
	return DocumentSystem::getInstance().getAllDocuments();
}

/*
 *
 */
Player* Application::get_player()
{
	return m_video_player->player();
}

/*
 *
 */
WaveformManager* Application::get_waveform_manager()
{
	return m_waveform_editor;
}

/*
 * Need to connect the visibility signal of the widgets children 
 * (video player and waveform editor) for updating the visibility of 
 * the paned multimedia widget.
 */
void Application::init_panel_multimedia()
{
	Gtk::Widget *child1 = m_paned_multimedia->get_child1();
	Gtk::Widget *child2 = m_paned_multimedia->get_child2();

	if(child1)
	{
		child1->signal_show().connect(
			sigc::mem_fun(*this, &Application::on_paned_multimedia_visibility_child_changed));
		child1->signal_hide().connect(
			sigc::mem_fun(*this, &Application::on_paned_multimedia_visibility_child_changed));
	}

	if(child2)
	{
		child2->signal_show().connect(
			sigc::mem_fun(*this, &Application::on_paned_multimedia_visibility_child_changed));
		child2->signal_hide().connect(
			sigc::mem_fun(*this, &Application::on_paned_multimedia_visibility_child_changed));
	}

	// first check
	on_paned_multimedia_visibility_child_changed();
}

/*
 * Check the state visibility of the children. 
 * When one child is show the panel is also show.
 * When both chidren are hide, the panel is hide.
 * This callback are connected to signals 
 * 'signal_show' and 'signal_hide' of the children.
 */
void Application::on_paned_multimedia_visibility_child_changed()
{
	Gtk::Widget *child1 = m_paned_multimedia->get_child1();
	Gtk::Widget *child2 = m_paned_multimedia->get_child2();

	bool state1 = false;
	bool state2 = false;

	if(child1 != NULL)
		state1 = child1->is_visible();
	if(child2 != NULL)
		state2 = child2->is_visible();

	if(state1 || state2)
		m_paned_multimedia->show();
	else
		m_paned_multimedia->hide();
}

