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

#include "documents.h"
#include "documentsystem.h"

namespace se {
namespace documents {

namespace internal {

struct manager {
  ~manager() {
    active = nullptr;
  }
  Document *active{nullptr};
};

}  // namespace internal

void append(Document *doc) {
  DocumentSystem::getInstance().append(doc);
}

void remove(Document *doc) {
  DocumentSystem::getInstance().remove(doc);
}

vector<Document *> all() {
  auto tmp = DocumentSystem::getInstance().getAllDocuments();
  return std::vector<Document *>(tmp.begin(), tmp.end());
}

void active(Document *doc) {
  DocumentSystem::getInstance().setCurrentDocument(doc);
}

Document *active() {
  return DocumentSystem::getInstance().getCurrentDocument();
}

signal_document &signal_created() {
  return DocumentSystem::getInstance().signal_document_create();
}

signal_document &signal_deleted() {
  return DocumentSystem::getInstance().signal_document_delete();
}

signal_document &signal_active_changed() {
  return DocumentSystem::getInstance().signal_current_document_changed();
}

signal_document_modified &signal_modified() {
  return DocumentSystem::getInstance().signals_document();
}

Document *find_by_name(const ustring &name) {
  return DocumentSystem::getInstance().getDocument(name);
}

ustring generate_untitled_name(const ustring &ext) {
  return DocumentSystem::getInstance().create_untitled_name(ext);
}

}  // namespace documents
}  // namespace se
