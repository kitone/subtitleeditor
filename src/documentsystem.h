#pragma once

// subtitleeditor -- a tool to create or edit subtitle
//
// https://kitone.github.io/subtitleeditor/
// https://github.com/kitone/subtitleeditor/
//
// Copyright @ 2005-2018, kitone
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "document.h"

class DocumentSystem {
 public:
  DocumentSystem();
  ~DocumentSystem();

  static DocumentSystem& getInstance();

  void append(Document* doc);

  // emit signal_document_delete and delete the document
  void remove(Document* doc);

  sigc::signal<void, Document*>& signal_document_create();

  sigc::signal<void, Document*>& signal_document_delete();

  sigc::signal<void, Document*>& signal_current_document_changed();

  sigc::signal<void, Document*, const std::string&>& signals_document();

  void setCurrentDocument(Document* doc);

  Document* getCurrentDocument();

  DocumentList getAllDocuments();

  // filename (getFilename) is used not name (getName)!
  Document* getDocument(const Glib::ustring& filename);

  // Find a unique name (like "Untitled-5") for a new document
  Glib::ustring create_untitled_name(const Glib::ustring& extension = "");

  // Check with other document if this name exist
  // Return true if it is
  bool check_if_document_name_exist(const Glib::ustring& name);

 protected:
  DocumentList m_listDocuments;

  Document* m_currentDocument{nullptr};

  sigc::signal<void, Document*> m_signal_document_create;
  sigc::signal<void, Document*> m_signal_document_delete;
  sigc::signal<void, Document*> m_signal_current_document_changed;
  sigc::signal<void, Document*, const std::string&> m_signal_document;
};
