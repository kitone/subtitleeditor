#ifndef _DialogUtility_h
#define _DialogUtility_h

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
 

#include <gtkmm.h>
#include <libglademm/xml.h>
#include "documentsystem.h"

/*
 *
 *
 */
class DialogActionMultiDoc : public Gtk::Dialog
{
public:
	DialogActionMultiDoc(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

	/*
	 *	applique l'action à tous les documents
	 */
	bool apply_to_all_documents();

	/*
	 *	retourne la list des documents à modifier
	 *	selon qu'on utilise "current document" ou "All documents"
	 */
	DocumentList get_documents_to_apply();

protected:
	Gtk::RadioButton*	m_radioCurrentDocument;
	Gtk::RadioButton* m_radioAllDocuments;
};


/*
 *
 */
class ErrorDialog : public Gtk::MessageDialog
{
public:

	/*
	 *
	 */
	ErrorDialog(const Glib::ustring &primary, const Glib::ustring &secondary=Glib::ustring());
};

#endif//_DialogUtility_h

