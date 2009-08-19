#ifndef _Application_h
#define _Application_h

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
 

#include <gtkmm/window.h>
#include <gtkmm/notebook.h>
#include <gtkmm/box.h>

#include "document.h"
#include "menubar.h"
#include "statusbar.h"
#include "options.h"

#include "subtitleeditorwindow.h"
#include "vp/videoplayer.h"
#include "we/waveformeditor.h"

class Application : public Gtk::Window, public SubtitleEditorWindow
{
public:
	Application(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& builder);//int argc, char *argv[]);
	~Application();

	void init(OptionGroup &options);

	/*
	 *
	 */
	Glib::RefPtr<Gtk::UIManager> get_ui_manager();

	/*
	 *
	 */
	Document* get_current_document();

	/*
	 *
	 */
	DocumentList get_documents();

	/*
	 *
	 */
	Player* get_player();

	/*
	 *
	 */
	WaveformManager* get_waveform_manager();

protected:

	/*
	 *
	 */
	void on_config_interface_changed(const Glib::ustring &key, const Glib::ustring &value);
	
	/*
	 *
	 */
	void load_config();

protected:

	/*
	 *
	 */
	void 	notebook_drag_data_received (const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);

	/*
	 *
	 */
	virtual bool on_delete_event(GdkEventAny *ev);
	bool on_key_press_event(GdkEventKey *ev);

	/*
	 *	il y a la création d'un nouveau document
	 *	on l'ajoute dans le notebook
	 *	signal emit par DocumentSystem::signal_document_create
	 */
	void on_document_create(Document *doc);

	/*
	 *	on efface le document du notebook
	 *	signal emit par DocumentSystem::signal_document_delete
	 */
	void on_document_delete(Document *doc);

	/*
	 * The current document has changed.
	 * Needs to update the ui.
	 */
	void on_current_document_changed(Document* doc);

	/*
	 *	Changement dans le notebook de la page editer
	 *	On recupere la page pusi on init DocumentSystem avec le document
	 */
	void on_signal_switch_page(GtkNotebookPage* page, guint page_num);

	/*
	 *
	 */
	void on_close_document(Document *doc);

	/*
	 *	mise a jour d'info (filename, tooltip, ...) dans la page
	 */
	void update_document_property(Document *doc);

	/*
	 *	when the document has changed, update the name (*)name
	 *	signal "document-changed"
	 */
	void on_document_changed(Document *doc);

	/*
	 *	retourne le widget (notebook) par rapport au document
	 */
	Gtk::Widget* get_widget(Document *doc);

	/*
	 * Need to connect the visibility signal of the widgets children 
	 * (video player and waveform editor) for updating the visibility of 
	 * the paned multimedia widget.
	 */
	void init_panel_multimedia();

	/*
	 * Check the state visibility of the children. 
	 * When one child is show the panel is also show.
	 * When both chidren are hide, the panel is hide.
	 * This callback are connected to signals 
	 * 'signal_show' and 'signal_hide' of the children.
	 */
	void on_paned_multimedia_visibility_child_changed();

	void connect_document(Document *doc);
	void disconnect_document(Document *doc);

	void update_title(Document *doc);

protected:
	Gtk::VBox*			m_vboxMain;
	MenuBar					m_menubar;
	Gtk::HPaned*		m_paned_multimedia;
	VideoPlayer*		m_video_player;
	WaveformEditor*	m_waveform_editor;
	Gtk::Notebook*	m_notebook_documents;
	Statusbar*			m_statusbar;

	std::list<sigc::connection> m_document_connections;

	// uri for external video player
	Glib::ustring		m_uri_movie_external_video_player;
};

#endif//_Application_h

