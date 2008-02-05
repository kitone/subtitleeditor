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
#include "DocumentSystem.h"

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
 *
 */

class FileChooserExtra;

enum FILE_CHOOSER_EXTRA
{
	EXTRA_ENCODING_WITH_AUTO_DETECTED	= 1 << 0,
	EXTRA_ENCODING	= 1 << 1,
	EXTRA_FORMAT		= 1 << 2,
	EXTRA_NEWLINE		= 1 << 3
};

class DialogFileChooser : public Gtk::FileChooserDialog
{
public:


	/*
	 *
	 */
	DialogFileChooser(const Glib::ustring &title, const Glib::ustring &dialog_name,
			Gtk::FileChooserAction action=Gtk::FILE_CHOOSER_ACTION_OPEN, int ext=EXTRA_ENCODING);

	~DialogFileChooser();

	/*
	 *	utiliser pour lire la config (dernier rep ouvert)
	 */
	void setDialogName(const Glib::ustring &name);

	/*
	 *
	 */
	Glib::ustring getEncoding();

	/*
	 *
	 */
	Glib::ustring getFormat();

	/*
	 *	"WINDOWS"
	 *	"UNIX"
	 */
	Glib::ustring getNewLine();

	/*
	 *
	 */
	void show_filter();


protected:
	
	/*
	 * exemple :
	 * name = "All Format Supported"
	 * patterns = "*.ssa;*.ass;*.sub;*.txt"
	 */
	void addFilter(const Glib::ustring &name, const Glib::ustring &patterns);

	/*
	 *
	 */
	void loadConfig(const Glib::ustring &name);
	
	/*
	 *
	 */
	void saveConfig(const Glib::ustring &name);

protected:
	Glib::ustring m_name;
	FileChooserExtra* m_fileChooserExtra;
};

#endif//_DialogUtility_h

