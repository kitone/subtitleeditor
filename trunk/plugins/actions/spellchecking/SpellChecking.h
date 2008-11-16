#ifndef _SpellChecking_h
#define _SpellChecking_h

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
 

#include "config.h"

#include <libglademm/xml.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/textview.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/entry.h>
#include <gtkmm/comboboxtext.h>
#include <i18n.h>
#include <gtkmm_utility.h>
#include <Document.h>

/*
 *
 */
class DialogSpellChecking : public Gtk::Dialog
{
protected:

	class ComboBoxText : public Gtk::ComboBoxText
	{
	public:
		ComboBoxText(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
		:Gtk::ComboBoxText(cobject)
		{
		}
	};

	/*
	 *
	 */
	class ListSuggestions : public Gtk::ListStore
	{
	public:
		/*
		 *
		 */
		class Column : public Gtk::TreeModel::ColumnRecord
		{
		public:
			Column()
			{
				add(string);
			}
			Gtk::TreeModelColumn<Glib::ustring> string;
		};

		Column m_column;
	public:

		/*
		 *
		 */
		ListSuggestions()
		{
			set_column_types(m_column);
		}

		/*
		 *
		 */
		void add(const Glib::ustring &str)
		{
			Gtk::TreeIter iter = append();
			(*iter)[m_column.string] = str;
		}
	};

public:
	DialogSpellChecking(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~DialogSpellChecking();

	/*
	 *
	 */
	void execute(Document *doc);

protected:

	/*
	 *	creation de la treeview suggestion + model
	 */
	void create_treeview_suggestions();

	/*
	 *	callback 
	 */

	/*
	 *	remplace le mot par celui de la list des suggestions (m_entryReplaceWith)
	 */
	void on_replace();

	/*
	 *	passe simplement au mot suivant
	 */
	void on_ignore();

	/*
	 *	ajoute le mot dans le dico de la session
	 */
	void on_ignore_all();

	/*
	 *	ajoute le mot dans le dico perso
	 */
	void on_add_word();

	/*
	 *	la selection a changer dans la list des suggestions
	 *	init l'interface selon le choix
	 */
	void on_treeview_suggestions_selection_changed();

	/*
	 *
	 */
	void on_combobox_dicts_changed();

	/*
	 *	quand on utilise le bouton "Check Word" (m_buttonCheckWord)
	 *	verifie le mot dans "Replace With:" (m_entryReplaceWith)
	 */
	void on_check_word();

protected:

	/*
	 *
	 */
	bool set_dict(const Glib::ustring &name);

	/*
	 *	passe a la ligne suivante grace a l'iter (m_current_subtitle)
	 *	s'il n'y a pas de ligne suivante alors on init l'interface 
	 *	set_sensitive(false) et affiche le message "Completed spell checking."
	 */
	bool check_next_line();
	
	/*
	 *	verifie les mots suivant
	 */
	bool check_next_word();

	/*
	 *	verifie le mot "word"
	 *	return false en cas d'erreur dans le mot
	 */
	bool check_word(const Glib::ustring &word);

	/*
	 *	recupere le texte de l'iter
	 *	init les m_current_xxx
	 *	et appel check_text pour verifie le text
	 */
	bool check_line(Subtitle subtitle);


	
	/*
	 * verifie chaque mot du text (m_current_text)
	 * retourne true en cas d'erreur dans un mot
	 * retourne false s'il n'y a aucune erreur
	 */
	bool check_text();

	/*
	 *	verifie la lettre pour savoir si c'est un nouveau mot
	 *	, . " * 123456789, ...
	 */
	bool is_end_char(gchar c);

	/*
	 *	init l'interface avec ce mot
	 *	textview select
	 *	list suggestion
	 *	...
	 */
	void init_with_word(const Glib::ustring &text, const Glib::ustring &word);

	/*
	 *	init la list des suggestions par rapport au mot "word"
	 */
	void init_suggestions(const Glib::ustring &word);

protected:
	
	Gtk::TextView*	m_textview;
	
	Gtk::Entry*			m_entryReplaceWith;
	Gtk::Button*		m_buttonCheckWord;

	Gtk::TreeView*	m_treeviewSuggestions;
	Glib::RefPtr<ListSuggestions> m_listSuggestions;

	Gtk::Button*		m_buttonReplace;
	Gtk::Button*		m_buttonIgnore;
	Gtk::Button*		m_buttonIgnoreAll;
	Gtk::Button*		m_buttonAddWord;


	Gtk::Label*			m_labelCompletedSpellChecking;

	ComboBoxText*		m_comboboxDicts;

	// word
	Subtitle				m_current_subtitle;
	Glib::ustring		m_current_text;
	unsigned int		m_current_word_start;
	unsigned int		m_current_next_word;
	Glib::ustring		m_current_word;
};

#endif//_SpellChecking_h
