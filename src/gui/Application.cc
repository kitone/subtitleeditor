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

#include <config.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/accelmap.h>
#include "utility.h"
#include "Application.h"
#include "DocumentSystem.h"
#include "ActionSystem.h"

#include "gui/CheckErrorsUI.h"


void on_current_document_changed(Document *doc)
{
	PluginSystem::get_instance().update_ui();
}

/*
 *
 */
Application::Application(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)//int argc, char *argv[])
:Gtk::Window(cobject)
{
	//simple_test();

	refGlade->get_widget_derived("statusbar", m_statusbar);

	refGlade->get_widget("vbox-main", m_vboxMain);
	refGlade->get_widget("paned-multimedia", m_paned_multimedia);
	refGlade->get_widget_derived("video-player", m_videoPlayer);
	refGlade->get_widget("notebook-documents", m_notebook_documents);

	//m_notebook_documents->set_scrollable(true);

	set_default_size(800,600);
	set_icon_from_file(get_share_dir("subtitleeditor.svg"));

	m_vboxMain->pack_start(m_menubar, false, false);
	m_vboxMain->reorder_child(m_menubar, 0);

	m_menubar.show_all();

	DocumentSystem::getInstance().signal_document_create().connect(
			sigc::mem_fun(*this, &Application::on_document_create));

	DocumentSystem::getInstance().signal_document_delete().connect(
			sigc::mem_fun(*this, &Application::on_document_delete));

	
	DocumentSystem::getInstance().signal_current_document_changed().connect(
			sigc::ptr_fun(on_current_document_changed));


	m_notebook_documents->signal_switch_page().connect(
			sigc::mem_fun(*this, &Application::on_signal_switch_page));

	// on va chercher la configuration clavier
	Glib::ustring path_se_accelmap = get_config_dir("accelmap");
	Gtk::AccelMap::load(path_se_accelmap);

	ActionSystem::getInstance().signal_emit().connect(
			sigc::mem_fun(*this, &Application::on_execute_action));


	//
	Config::getInstance().signal_changed("interface").connect(
			sigc::mem_fun(*this, &Application::on_config_interface_changed));

	show();

	// FIXME: hack
	PluginSystem::get_instance().activate_plugins();

	m_menubar.create(*this, *m_statusbar);

	// FIXME: hack
	PluginSystem::get_instance().post_activate_plugins();

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

	m_autosave_timeout.disconnect();

	Glib::ustring path_se_accelmap = get_config_dir("accelmap");
	Gtk::AccelMap::save(path_se_accelmap);
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

	cfg.get_value_bool("interface", "display-video-player", value);
	set_display_video_player(value);

	// first launch
	if(!cfg.has_group("encodings"))
	{
		cfg.set_value_string("encodings", "encodings", "ISO-8859-15;UTF-8");
		cfg.set_value_bool("encodings", "used-auto-detected", true);
	}

	cfg.get_value_bool("interface", "used-autosave", value);
	if( value )
	{
		int autosave_minutes = 0;
		cfg.get_value_int("interface", "autosave-minutes", autosave_minutes);
		
		SubtitleTime time(0, autosave_minutes, 0, 0);

		m_autosave_timeout = Glib::signal_timeout().connect(
				sigc::mem_fun(*this, &Application::on_autosave_files), time.totalmsecs);
	}
}

/*
 *
 */
bool Application::on_delete_event(GdkEventAny *ev)
{
	bool res = Gtk::Window::on_delete_event(ev);

	Glib::ustring path_se_accelmap = get_config_dir("accelmap");
	Gtk::AccelMap::save(path_se_accelmap);

	//return ask_to_save_on_exit();
	
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
	Gtk::Tooltips *tooltips = NULL;
	Gtk::Widget *page = NULL;


	scroll = manage(new Gtk::ScrolledWindow);
	scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scroll->add(*doc->widget());
	scroll->show();
	

	id = m_notebook_documents->append_page(*scroll, *hbox);


	tooltips = manage(new Gtk::Tooltips);
	m_notebook_documents->set_current_page(id);


	page = m_notebook_documents->get_nth_page(id);

	page->set_data("event", eventbox);
	page->set_data("document", doc);
	page->set_data("label", label);
	page->set_data("tooltips", tooltips);

	doc->get_signal("document-changed").connect(
			sigc::bind<Document*>(
				sigc::mem_fun(*this, &Application::on_document_changed), doc));

	doc->get_signal("document-property-changed").connect(
			sigc::bind<Document*>(
				sigc::mem_fun(*this, &Application::update_document_property), doc));


	update_document_property(doc);

	DocumentSystem::getInstance().setCurrentDocument(doc);

	connect_document(doc);

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

	Gtk::Widget *event = (Gtk::Widget*)widget->get_data("event");
	g_return_if_fail(event);

	Gtk::Label *label = (Gtk::Label*)widget->get_data("label");
	g_return_if_fail(label);

	Gtk::Tooltips *tooltips = (Gtk::Tooltips*)widget->get_data("tooltips");
	g_return_if_fail(tooltips);

	// mise à jour du nom du document
	Glib::ustring display_name = (doc->get_document_changed() ? "*" : "") + doc->getName();
	
	label->set_text(display_name);

	// mise à jour du tooltip
	Glib::ustring tip = build_message("%s %s\n\n%s %s\n%s %s\n%s %s",
			_("Name:"), doc->getFilename().c_str(),
			_("Encoding:"), doc->getCharset().c_str(),
			_("Format:"), doc->getFormat().c_str(),
			_("NewLine:"), doc->getNewLine().c_str());

	tooltips->force_window();
	label->set_use_markup(true);
	tooltips->set_tip(*event, tip);
	label->set_use_markup(true);

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
void Application::on_execute_action(const Glib::ustring &name)
{
#define REGISTER(action, callback) if(action == name) { callback(); return; }

	// TOOLS
	REGISTER("check-errors", check_errors);

#undef REGISTER
}


// TOOLS

/*
 *
 */
void Application::check_errors()
{
	createDialogCheckErrors();
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
	else if(key == "display-video-player")
	{
		bool state;
		from_string(value, state);

		set_display_video_player(state);
	}
	else if(key == "used-autosave")
	{
		if(m_autosave_timeout)
			m_autosave_timeout.disconnect();

		bool state;
		from_string(value, state);

		if(state)
		{
			if(m_autosave_timeout)
				m_autosave_timeout.disconnect();

			int autosave_minutes = 0;
			Config::getInstance().get_value_int("interface", "autosave-minutes", autosave_minutes);
			
			SubtitleTime time(0, autosave_minutes, 0, 0);

			m_autosave_timeout = Glib::signal_timeout().connect(
					sigc::mem_fun(*this, &Application::on_autosave_files), time.totalmsecs);
		}
	}
}

/*
 *
 */
void Application::set_display_video_player(bool state)
{
	Gtk::Paned *paned = dynamic_cast<Gtk::Paned*>(m_videoPlayer->get_parent());

	g_return_if_fail(paned);

	if(state)
	{
		m_videoPlayer->show();

		if(!paned->is_visible())
			paned->show();
	}
	else
	{
		m_videoPlayer->hide();

		if(paned->get_child1()->is_visible() == false && paned->get_child2()->is_visible() == false)
			paned->hide();
	}
}

/*
 *
 */
bool Application::on_autosave_files()
{
	DocumentList docs = DocumentSystem::getInstance().getAllDocuments();

	if(docs.size() == 0)
		return true;
	
	m_statusbar->flash_message("Autosave files...");

	DocumentList::iterator it;
	
	for(it = docs.begin(); it != docs.end(); ++it)
	{
		Glib::ustring filename = (*it)->getFilename();
		/*
		Glib::ustring format = (*it)->getFormat();
		Glib::ustring charset = (*it)->getCharset();
		*/
#warning "FIXME: if filename doesn't exist"
		(*it)->save(filename);
	}

	m_statusbar->flash_message("Autosave files...ok");
	return true;
}

/*
 *
 */
void Application::init(OptionGroup &options)
{
	se_debug(SE_DEBUG_APP);

	// files
	for(unsigned int i = 0; i< options.files.size(); ++i)
	{
		Glib::ustring filename = options.files[i];

		if(	Glib::file_test(filename, Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_REGULAR) && 
				Glib::file_test(filename, Glib::FILE_TEST_IS_DIR) == false)
		{
			Document *doc = new Document;
			doc->setFormat("");
			doc->setCharset(options.encoding);

			if(doc->open(filename))
			{
				DocumentSystem::getInstance().append(doc);
				DocumentSystem::getInstance().setCurrentDocument(doc);
			}
			else
			{
				delete doc;
			}
		}
	}

	// ------------------------------------------------
	// video
	Glib::ustring video = options.video;

	// s'il n'y a pas de video et s'il n'y a qu'un seule fichier sous-titre
	// recherche une video par rapport au nom du sous-titre
	bool automatically_open_video;
	Config::getInstance().get_value_bool("general", "automatically-open-video", automatically_open_video);

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

			m_videoPlayer->open(uri);
		}
		catch(const Glib::Error &ex)
		{
			std::cerr << ex.what() << std::endl;
		}
	}

#warning "FIXME: auto open waveform"
	/*
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

			//m_waveform_system->open(uri);
		}
		catch(const Glib::Error &ex)
		{
			std::cerr << ex.what() << std::endl;
		}
	}
	*/
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

		try
		{
			Document *doc = new Document;
			doc->setCharset("");

			if(doc->open(filename))
			{
				DocumentSystem::getInstance().append(doc);
			}
			else
			{
				delete doc;
			}
		}
		catch(...)
		{
		}	
	}
}


/*
 *
 */
Glib::RefPtr<Gtk::UIManager> Application::get_ui_manager()
{
	return m_menubar.m_refUIManager;
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
