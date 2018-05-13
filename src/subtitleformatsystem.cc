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

#include <map>
#include <memory>
#include "error.h"
#include "extension/subtitleformat.h"
#include "extensionmanager.h"
#include "filereader.h"
#include "filewriter.h"
#include "subtitleformatsystem.h"
#include "utility.h"

// Return the instance.
SubtitleFormatSystem &SubtitleFormatSystem::instance() {
  static SubtitleFormatSystem instance;
  return instance;
}

// Constructor
SubtitleFormatSystem::SubtitleFormatSystem() {
}

// Destructor
SubtitleFormatSystem::~SubtitleFormatSystem() {
}

// Try to determine the format of the subtitles in the submitted FileReader
// Exceptions:
// UnrecognizeFormatError.
Glib::ustring SubtitleFormatSystem::get_subtitle_format_from_small_contents(
    Reader *reader) {
  const Glib::ustring &contents = reader->get_data();

  se_debug_message(SE_DEBUG_APP, "small content:\n%s", contents.c_str());

  Glib::RegexCompileFlags compile_flags = Glib::REGEX_MULTILINE;

  se_debug_message(SE_DEBUG_APP, "Trying to determinate the file format...");

  SubtitleFormatList sfe_list = get_subtitle_format_list();
  for (SubtitleFormatList::const_iterator it = sfe_list.begin();
       it != sfe_list.end(); ++it) {
    SubtitleFormatInfo sfi = (*it)->get_info();

    se_debug_message(SE_DEBUG_APP, "Try with '%s' format", sfi.name.c_str());

    Glib::ustring pattern = sfi.pattern;

    if (Glib::Regex::match_simple(pattern, contents, compile_flags)) {
      Glib::ustring name = sfi.name;

      se_debug_message(SE_DEBUG_APP, "Determine the format as '%s'",
                       name.c_str());
      return name;
    }
  }

  throw UnrecognizeFormatError(_("Couldn't recognize format of the file."));
}

// Try to determine the format of the file, and return the format name.
// Exceptions:
// UnrecognizeFormatError.
// EncodingConvertError.
Glib::ustring SubtitleFormatSystem::get_subtitle_format_from_small_contents(
    const Glib::ustring &uri, const Glib::ustring &charset) {
  // Open the file and read only a small contents (max size: 1000)
  FileReader file(uri, charset, 1000);

  return get_subtitle_format_from_small_contents(&file);
}

// Try to determine the format of the subtitles in the submitted ustring
// Exceptions:
// UnrecognizeFormatError.
Glib::ustring SubtitleFormatSystem::get_subtitle_format_from_small_contents(
    const Glib::ustring &data) {
  // Open the file and read only a small contents (max size: 1000)
  Reader file(data);
  return get_subtitle_format_from_small_contents(&file);
}

// Create a SubtitleFormat from a name.
// Throw UnrecognizeFormatError if failed.
SubtitleFormatIO *SubtitleFormatSystem::create_subtitle_format_io(
    const Glib::ustring &name) {
  se_debug_message(SE_DEBUG_APP, "Trying to create the subtitle format '%s'",
                   name.c_str());

  SubtitleFormatList sfe_list = get_subtitle_format_list();
  for (SubtitleFormatList::const_iterator it = sfe_list.begin();
       it != sfe_list.end(); ++it) {
    SubtitleFormat *sfe = *it;

    se_debug_message(SE_DEBUG_APP, "considering subtitle format'%s'...",
                     sfe->get_info().name.c_str());

    if (sfe->get_info().name == name)
      return sfe->create();
  }
  throw UnrecognizeFormatError(build_message(
      _("Couldn't create the subtitle format '%s'."), name.c_str()));
}

// Try to open a subtitle file from the uri.
// If charset is empty, the automatically detection is used.
// Exceptions: UnrecognizeFormatError, EncodingConvertError, IOFileError,
// Glib::Error...
void SubtitleFormatSystem::open_from_reader(Document *document, Reader *reader,
                                            const Glib::ustring &format) {
  se_debug_message(SE_DEBUG_APP, "Trying to read from reader ...");

  // init the reader
  std::unique_ptr<SubtitleFormatIO> sfio(create_subtitle_format_io(format));
  sfio->set_document(document);
  sfio->open(*reader);

  se_debug_message(SE_DEBUG_APP, "Sets the document property ...");

  // We only have an uri and a charset when it's read from a file (FileReader)
  FileReader *filereader = dynamic_cast<FileReader *>(reader);
  if (filereader != NULL) {
    document->setFilename(Glib::filename_from_uri(filereader->get_uri()));
    document->setCharset(filereader->get_charset());
  }
  document->setNewLine(reader->get_newline());
  document->setFormat(format);

  document->emit_signal("document-changed");
  document->emit_signal("document-property-changed");

  se_debug_message(SE_DEBUG_APP, "The reader has been read with success.");
}

// Try to open a subtitle file from the uri.
// If charset is empty, the automatically detection is used.
// Exceptions: UnrecognizeFormatError, EncodingConvertError, IOFileError,
// Glib::Error...
void SubtitleFormatSystem::open_from_uri(Document *document,
                                         const Glib::ustring &uri,
                                         const Glib::ustring &charset,
                                         const Glib::ustring &myformat) {
  se_debug_message(
      SE_DEBUG_APP,
      "Trying to open the file %s with charset '%s' and format '%s",
      uri.c_str(), charset.c_str(), myformat.c_str());

  // First try to find the subtitle file type from the contents
  Glib::ustring format =
      myformat.empty() ? get_subtitle_format_from_small_contents(uri, charset)
                       : myformat;

  FileReader reader(uri, charset);
  open_from_reader(document, &reader, format);

  se_debug_message(SE_DEBUG_APP, "The file %s has been read with success.",
                   uri.c_str());
}

// Try to open a ustring as a subtitle file
// Charset is assumed to be UTF-8.
// Exceptions: UnrecognizeFormatError, Glib::Error...
void SubtitleFormatSystem::open_from_data(Document *document,
                                          const Glib::ustring &data,
                                          const Glib::ustring &myformat) {
  se_debug_message(SE_DEBUG_APP, "Trying to load ustring as subtitles.");

  // First try to find the subtitle file type from the contents
  Glib::ustring format = myformat.empty()
                             ? get_subtitle_format_from_small_contents(data)
                             : myformat;

  Reader reader(data);
  open_from_reader(document, &reader, format);
  se_debug_message(SE_DEBUG_APP,
                   "The ustring was successfully read in as a subtitle file.");
}

// Save the document in a file.
// Exceptions: UnrecognizeFormatError, EncodingConvertError, IOFileError,
// Glib::Error...
void SubtitleFormatSystem::save_to_uri(Document *document,
                                       const Glib::ustring &uri,
                                       const Glib::ustring &format,
                                       const Glib::ustring &charset,
                                       const Glib::ustring &newline) {
  se_debug_message(SE_DEBUG_APP,
                   "Trying to save to the file '%s' as format '%s' with "
                   "charset '%s' and newline '%s'",
                   uri.c_str(), format.c_str(), charset.c_str(),
                   newline.c_str());

  std::unique_ptr<SubtitleFormatIO> sfio(create_subtitle_format_io(format));
  // init the reader
  sfio->set_document(document);

  FileWriter writer(uri, charset, newline);

  se_debug_message(SE_DEBUG_APP, "Save in the Writer...");

  sfio->save(writer);

  se_debug_message(SE_DEBUG_APP, "Save to the file...");

  writer.to_file();

  se_debug_message(SE_DEBUG_APP, "Update the document property...");

  document->setCharset(charset);
  document->setFilename(Glib::filename_from_uri(uri));
  document->setFormat(format);
  document->make_document_unchanged();
  document->emit_signal("document-property-changed");

  se_debug_message(SE_DEBUG_APP, "The file %s has been save with success.",
                   uri.c_str());
}

// Save the document to a ustring. Charset is UTF-8, newline is Unix.
// Exceptions: UnrecognizeFormatError, Glib::Error...
void SubtitleFormatSystem::save_to_data(Document *document, Glib::ustring &dst,
                                        const Glib::ustring &format) {
  se_debug_message(SE_DEBUG_APP,
                   "Trying to save to ustring as subtitles in the '%s' format.",
                   format.c_str());

  std::unique_ptr<SubtitleFormatIO> sfio(create_subtitle_format_io(format));
  // init the reader
  sfio->set_document(document);

  Writer writer;

  se_debug_message(SE_DEBUG_APP, "Save in the Writer...");

  sfio->save(writer);

  se_debug_message(SE_DEBUG_APP, "Save to the file...");

  dst = writer.get_data();

  se_debug_message(SE_DEBUG_APP, "Update the document property...");

  document->setCharset("UTF-8");
  document->setFilename("");
  document->setFormat(format);
  document->make_document_unchanged();
  document->emit_signal("document-property-changed");

  se_debug_message(SE_DEBUG_APP, "Succesfully saved to ustring.");
}

// Returns all information about supported subtitles.
std::list<SubtitleFormatInfo> SubtitleFormatSystem::get_infos() {
  std::list<SubtitleFormatInfo> infos;

  SubtitleFormatList sfe_list = get_subtitle_format_list();

  SubtitleFormatList::const_iterator it;
  for (it = sfe_list.begin(); it != sfe_list.end(); ++it)
    infos.push_back((*it)->get_info());

  return infos;
}

// Return information about the subtitle format.
bool SubtitleFormatSystem::get_info(const Glib::ustring &subtitle_format,
                                    SubtitleFormatInfo &info) {
  std::list<SubtitleFormatInfo> infos = get_infos();
  for (std::list<SubtitleFormatInfo>::const_iterator it = infos.begin();
       it != infos.end(); ++it) {
    if ((*it).name == subtitle_format) {
      info = *it;
      return true;
    }
  }
  return false;
}

// Check if the subtitle format is supported.
bool SubtitleFormatSystem::is_supported(const Glib::ustring &format) {
  SubtitleFormatList sfe_list = get_subtitle_format_list();

  SubtitleFormatList::const_iterator it;
  for (it = sfe_list.begin(); it != sfe_list.end(); ++it) {
    if ((*it)->get_info().name == format)
      return true;
  }

  return false;
}

// Sort by name (SubtitleInfo.name)
bool on_sort_sf(SubtitleFormat *a, SubtitleFormat *b) {
  return a->get_info().name < b->get_info().name;
}

// Return a list of SubtitleFormat from ExtensionManager.
SubtitleFormatList SubtitleFormatSystem::get_subtitle_format_list() {
  std::list<SubtitleFormat *> list;
  // Get from ExtensionManager
  std::list<ExtensionInfo *> sf_list =
      ExtensionManager::instance().get_info_list_from_categorie(
          "subtitleformat");
  for (std::list<ExtensionInfo *>::iterator it = sf_list.begin();
       it != sf_list.end(); ++it) {
    if ((*it)->get_active() == false)
      continue;

    SubtitleFormat *sf = dynamic_cast<SubtitleFormat *>((*it)->get_extension());
    if (sf)
      list.push_back(sf);
  }
  list.sort(on_sort_sf);
  return list;
}

// Return quickly the extension used by the format or an empty string
Glib::ustring SubtitleFormatSystem::get_extension_of_format(
    const Glib::ustring &format) {
  SubtitleFormatInfo info;
  if (SubtitleFormatSystem::instance().get_info(format, info))
    return info.extension;
  return Glib::ustring();
}
