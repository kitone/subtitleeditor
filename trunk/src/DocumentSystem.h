#ifndef _DocumentSystem_h
#define _DocumentSystem_h

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
 

#include "Document.h"


class DocumentSystem
{
public:
	DocumentSystem();
	~DocumentSystem();

	static DocumentSystem& getInstance();

	/*
	 *
	 */
	void append(Document *doc);

	/*
	 *	emit signal_document_delete and	delete the document
	 */
	void remove(Document *doc);

	/*
	 *
	 */
	sigc::signal<void, Document*>& signal_document_create();

	/*
	 *
	 */
	sigc::signal<void, Document*>& signal_document_delete();

	/*
	 *
	 */
	sigc::signal<void, Document*>& signal_current_document_changed();

	/*
	 *
	 */
	void setCurrentDocument(Document *doc);

	/*
	 *
	 */
	Document* getCurrentDocument();

	/*
	 *
	 */
	DocumentList getAllDocuments();

	/*
	 *	filename (getFilename) is used not name (getName)!
	 */
	Document* getDocument(const Glib::ustring &filename);

protected:
	DocumentList	m_listDocuments;
	
	Document*	m_currentDocument;

	sigc::signal<void, Document*>	m_signal_document_create;
	sigc::signal<void, Document*>	m_signal_document_delete;
	sigc::signal<void, Document*> m_signal_current_document_changed;
};

#endif//_DocumentSystem_h

