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

#include "documentsystem.h"
#include "utility.h"

DocumentSystem::DocumentSystem() : m_currentDocument(nullptr) {
  se_dbg(SE_DBG_APP);
}

DocumentSystem::~DocumentSystem() {
  se_dbg(SE_DBG_APP);

  m_currentDocument = nullptr;

  for (const auto& doc : m_listDocuments) {
    delete doc;
  }
  m_listDocuments.clear();
}

DocumentSystem& DocumentSystem::getInstance() {
  static DocumentSystem instance;
  return instance;
}

void DocumentSystem::append(Document* doc) {
  se_dbg(SE_DBG_APP);

  g_return_if_fail(doc);

  m_listDocuments.push_back(doc);

  m_signal_document_create(doc);
}

void DocumentSystem::remove(Document* doc) {
  se_dbg(SE_DBG_APP);

  g_return_if_fail(doc);

  m_listDocuments.remove(doc);

  if (m_currentDocument == doc) {
    setCurrentDocument(nullptr);
  }
  m_signal_document_delete(doc);

  delete doc;
  doc = nullptr;
}

sigc::signal<void, Document*>& DocumentSystem::signal_document_create() {
  se_dbg(SE_DBG_APP);
  return m_signal_document_create;
}

sigc::signal<void, Document*>& DocumentSystem::signal_document_delete() {
  se_dbg(SE_DBG_APP);
  return m_signal_document_delete;
}

sigc::signal<void, Document*>&
DocumentSystem::signal_current_document_changed() {
  se_dbg(SE_DBG_APP);
  return m_signal_current_document_changed;
}

sigc::signal<void, Document*, const std::string&>&
DocumentSystem::signals_document() {
  se_dbg(SE_DBG_APP);
  return m_signal_document;
}

void DocumentSystem::setCurrentDocument(Document* doc) {
  se_dbg_msg(SE_DBG_APP, "%s",
             ((doc == NULL) ? "NULL" : doc->getFilename().c_str()));

  // g_return_if_fail(doc);
  if (doc) {
    m_currentDocument = doc;
    m_signal_current_document_changed(doc);
  } else {
    m_currentDocument = nullptr;
    m_signal_current_document_changed(NULL);
  }
}

Document* DocumentSystem::getCurrentDocument() {
  if (m_currentDocument == nullptr) {
    return nullptr;
  }
  return m_currentDocument;
}

DocumentList DocumentSystem::getAllDocuments() {
  return m_listDocuments;
}

Document* DocumentSystem::getDocument(const Glib::ustring& filename) {
  se_dbg_msg(SE_DBG_APP, "filename = %s", filename.c_str());

  for (const auto& doc : m_listDocuments) {
    if (doc->getFilename() == filename)
      return doc;
  }
  se_dbg_msg(SE_DBG_APP, "return NULL: FAILED");

  return nullptr;
}

// find a unique name (like "Untitled-5") for a new document
Glib::ustring DocumentSystem::create_untitled_name(
    const Glib::ustring& extension) {
  se_dbg(SE_DBG_PLUGINS);

  Glib::ustring ext = extension.empty() ? "" : "." + extension;

  const gchar* untitled = _("Untitled %d");

  unsigned int i = 1;

  while (check_if_document_name_exist(build_message(untitled, i) + ext)) {
    ++i;
  }
  return build_message(untitled, i) + ext;
}

// check with other document if this name exist
// return true if it is
bool DocumentSystem::check_if_document_name_exist(const Glib::ustring& name) {
  se_dbg(SE_DBG_PLUGINS);

  for (const auto& doc : m_listDocuments) {
    if (name == doc->getName())
      return true;
  }
  return false;
}
