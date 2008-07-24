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
 
#include "DialogUtility.h"
#include "utility.h"
#include "SubtitleSystem.h"
#include "Config.h"
#include "Encodings.h"

/*
 *
 */
DialogActionMultiDoc::DialogActionMultiDoc(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:Gtk::Dialog(cobject)
{
	refGlade->get_widget("radio-current-document", m_radioCurrentDocument);
	refGlade->get_widget("radio-all-documents", m_radioAllDocuments);
}

/*
 *
 */
bool DialogActionMultiDoc::apply_to_all_documents()
{
	return m_radioAllDocuments->get_active();
}

/*
 *	retourne la list des documents à modifier
 *	selon qu'on utilise "current document" ou "All documents"
 */
DocumentList DialogActionMultiDoc::get_documents_to_apply()
{
	DocumentList docs;

	if(apply_to_all_documents())
		docs = DocumentSystem::getInstance().getAllDocuments();
	else
		docs.push_back( DocumentSystem::getInstance().getCurrentDocument() );
	
	return docs;
}