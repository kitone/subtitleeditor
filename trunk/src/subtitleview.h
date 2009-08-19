#ifndef _SubtitleView_h
#define _SubtitleView_h

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
 

#include <gtkmm/treeview.h>
#include <gtkmm/builder.h>
#include "stylemodel.h"
#include "cfg.h"

class Document;
class SubtitleModel;

/*
 *
 */
class SubtitleView : public Gtk::TreeView
{
public:
	SubtitleView(Document &doc);
	~SubtitleView();

  /*
   *	return first iter select
   */
	Gtk::TreeIter   getSelected();

	/*
	 *	select and set the cursor on iter
	 */
	void select_and_set_cursor(const Gtk::TreeIter &iter, bool start_editing=false);

	/*
	 * This is a static function. 
	 * Return the humain label by the internal name of the column.
	 */
	static Glib::ustring get_column_label_by_name(const Glib::ustring &name);

	/*
	 * Return the name of the current column focus. 
	 * (start, end, duration, text, translation ...)
	 */
	Glib::ustring get_current_column_name();

protected:	
	void loadCfg();

  /*
   *  create columns with the config order
	 *  ["subtitle-view"] columns="number;layer;..."
   */
	void createColumns();

	void createColumnNum();
	void createColumnLayer();
	void createColumnStart();
	void createColumnEnd();
	void createColumnDuration();
	void createColumnStyle();
	void createColumnName();
	void createColumnMarginR();
	void createColumnMarginL();
	void createColumnMarginV();
	void createColumnEffect();
	void createColumnText();
	void createColumnCPS();
	void createColumnTranslation();
	void createColumnNote();
 
	/*
	 * Get the columns displayed from the configuration and updates.
	 */
	void update_columns_displayed_from_config();


	/*
	 *
	 */
	void on_selection_changed();

	/*
	 *
	 */
	void on_edited_layer(const Glib::ustring &path, const Glib::ustring &newtext);

	/*
	 * callback utiliser pour modifier le temps directement depuis la list (treeview)
	 */
	void on_edited_start(const Glib::ustring &path, const Glib::ustring &newtext);

	/*
	 * callback utiliser pour modifier le temps directement depuis la list (treeview)
	 */
	void on_edited_end(const Glib::ustring &path, const Glib::ustring &newtext);

	/*
	 *  callback utiliser pour modifier le temps directement depuis la list (treeview)
	 */
	void on_edited_duration(const Glib::ustring &path, const Glib::ustring &newtext);

	/*
	 * callback utiliser pour modifier le texte directement depuis la list (treeview)
	 */
	void on_edited_text(const Glib::ustring &path, const Glib::ustring &newtext);

	void on_edited_translation(const Glib::ustring &path, const Glib::ustring &newtext);
	void on_edited_note(const Glib::ustring &path, const Glib::ustring &newtext);

	void on_edited_effect(const Glib::ustring &path, const Glib::ustring &newtext);

	/*
	 * callback utiliser pour modifier le style (a partir d'un menu, cell renderer combo)
	 */
	void on_edited_style(const Glib::ustring &path, const Glib::ustring &newstyle);

	/*
	 * callback utiliser pour modifier le nom (a partir d'un menu, cell renderer combo)
	 */
	void on_edited_name(const Glib::ustring &path, const Glib::ustring &newstyle);

	/*
	 * 
	 */
	void on_edited_margin_l(const Glib::ustring &path, const Glib::ustring &value);
	void on_edited_margin_r(const Glib::ustring &path, const Glib::ustring &value);
	void on_edited_margin_v(const Glib::ustring &path, const Glib::ustring &value);

	/*
	 *
	 */
	void on_set_style_to_selection(const Glib::ustring &name);


	/*
	 * COLUMN
	 */

	/*
	 * Return a new column (already manage) with Gtk::Label in title.
	 */
	Gtk::TreeViewColumn* create_treeview_column(const Glib::ustring &name);

	/*
	 *
	 */
	void create_column_time( 
					const Glib::ustring &name, 
					const Gtk::TreeModelColumnBase& column_attribute,
					const sigc::slot<void, const Glib::ustring&, const Glib::ustring&> &slot, 
					const Glib::ustring &tooltips=Glib::ustring());

	/*
	 * Retourne le nom utiliser en interne de la column
	 */
	Glib::ustring get_name_of_column(Gtk::TreeViewColumn *column);
	
	/*
	 * Retourne la colonne par rapport a son nom (interne)
	 */
	Gtk::TreeViewColumn* get_column_by_name(const Glib::ustring &name);
	
	/*
	 *
	 */
	void set_column_visible(const Glib::ustring& name, bool state);

	/*
	 *
	 */
	bool get_column_visible(const Glib::ustring &name);

	/*
	 * The position of the cursor (focused cell) has changed.
	 * Update the column title (bold).
	 */
	void on_cursor_changed();
	
	
	/*
	 *
	 */
	bool on_key_press_event(GdkEventKey *event);

	/*
	 *
	 */
	bool on_button_press_event(GdkEventButton *ev);
	
	
	/*
	 *	il y a eu des changements dans la configuration du groupe "subtitle-view"
	 */
	void on_config_subtitle_view_changed(const Glib::ustring &key, const Glib::ustring &value);

	/*
	 *
	 */
	void set_tooltips(Gtk::TreeViewColumn *column, const Glib::ustring &text);

protected:
	Document* m_refDocument;
	
	SubtitleColumnRecorder m_column;
  
	Glib::RefPtr<SubtitleModel> m_subtitleModel;
	Glib::RefPtr<StyleModel> m_styleModel;

	// pointe sur les columns par rapport a leur nom
	Gtk::TreeViewColumn* m_currentColumn;
	std::map<Glib::ustring, Gtk::TreeViewColumn*>	m_columns;

	Gtk::Menu m_menu_popup;
};


#endif//_SubtitleView_h
