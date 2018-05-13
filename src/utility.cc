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

#include <glibmm.h>
#include <gtkmm.h>
#include <iostream>
#include <sstream>
#include <string>
#include "cfg.h"
#include "subtitleeditorwindow.h"
#include "subtitletime.h"
#include "utility.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// profile name use by config dir
Glib::ustring static_profile_name = "default";

Glib::ustring build_message(const char *format, ...) {
  Glib::ustring res;

  va_list args;
  char *formatted = NULL;

  va_start(args, format);
  formatted = g_strdup_vprintf(format, args);
  va_end(args);

  res = formatted;

  g_free(formatted);

  return res;
}

// the profile name for the config dir
// ~/config/subtitleeditor/{profile}
void set_profile_name(const Glib::ustring &profile) {
  se_debug_message(SE_DEBUG_UTILITY, "profile=%s", profile.c_str());

  if (!profile.empty())
    static_profile_name = profile;
}

// ~/.config/subtitleeditor/{profile}/
// XDG Base Directory Specification
Glib::ustring get_config_dir(const Glib::ustring &file) {
  const gchar *configdir = g_get_user_config_dir();

  Glib::ustring path = Glib::build_filename(configdir, "subtitleeditor");

  // create config path if need
  if (Glib::file_test(path, Glib::FILE_TEST_IS_DIR) == false) {
    // g_mkdir(path.c_str(), 0700);
    Glib::spawn_command_line_sync("mkdir " + path);
  }

  // create profile path if need
  path = Glib::build_filename(path, static_profile_name);

  if (Glib::file_test(path, Glib::FILE_TEST_IS_DIR) == false) {
    Glib::spawn_command_line_sync("mkdir " + path);
  }

  return Glib::build_filename(path, file);
}

void dialog_warning(const Glib::ustring &primary_text,
                    const Glib::ustring &secondary_text) {
  Glib::ustring msg;

  msg += "<span weight=\"bold\" size=\"larger\">";
  msg += primary_text;
  msg += "</span>\n\n";
  msg += secondary_text;

  Gtk::MessageDialog dialog(msg, true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK,
                            true);
  dialog.run();
  // MessageDialog *dialog = new MessageDialog(msg, Gtk::MESSAGE_WARNING);
}

void dialog_error(const Glib::ustring &primary_text,
                  const Glib::ustring &secondary_text) {
  Glib::ustring msg;

  msg += "<span weight=\"bold\" size=\"larger\">";
  msg += primary_text;
  msg += "</span>\n\n";
  msg += secondary_text;

  Gtk::MessageDialog dialog(msg, true, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK,
                            true);
  dialog.run();
  // MessageDialog *dialog = new MessageDialog(msg, Gtk::MESSAGE_ERROR);
}

namespace utility {

bool string_to_bool(const std::string &str) {
  std::istringstream s(str);
  bool val = false;
  s >> val;
  return val;
}

int string_to_int(const std::string &str) {
  std::istringstream s(str);
  int val = 0;
  s >> val;
  return val;
}

int string_to_long(const std::string &str) {
  std::istringstream s(str);
  long val = 0;
  s >> val;
  return val;
}

double string_to_double(const std::string &str) {
  std::istringstream s(str);
  double val = 0;
  s >> val;
  return val;
}

void split(const std::string &str, const char &c,
           std::vector<std::string> &array, int max) {
  array.clear();

  std::istringstream iss(str);
  std::string word;

  if (max > 0) {
    int count = 1;
    while (std::getline(iss, word, (count < max) ? c : '\n')) {
      // std::cout << "word:" << word << std::endl;
      array.push_back(word);

      ++count;
    }
  } else {
    while (std::getline(iss, word, c)) {
      // std::cout << "word:" << word << std::endl;
      array.push_back(word);
    }
  }
}

void usplit(const Glib::ustring &str,
            const Glib::ustring::value_type &delimiter,
            std::vector<Glib::ustring> &container) {
  Glib::ustring::const_iterator it = str.begin(), end = str.end(), first;

  for (first = it; it != end; ++it) {
    if (delimiter == *it) {
      if (first != it) {  // || keep_blank)
        // extract the current field from the string
        container.push_back(Glib::ustring(first, it));
        // skip the next delimiter
        first = it;
        ++first;
      } else {
        ++first;
      }
    }
  }

  if (first != it) {  // || keep_blank
    // extract the last field from the string
    container.push_back(Glib::ustring(first, it));
  }
}

// Search and replace function.
void replace(Glib::ustring &text, const Glib::ustring &pattern,
             const Glib::ustring &replace_by) {
  Glib::ustring::size_type pos = 0;

  while ((pos = text.find(pattern, pos)) != Glib::ustring::npos) {
    text.replace(pos, pattern.size(), replace_by);
    pos = pos + replace_by.size();
  }
}

// Search and replace function.
void replace(std::string &text, const std::string &pattern,
             const std::string &replace_by) {
  std::string::size_type pos = 0;

  while ((pos = text.find(pattern, pos)) != std::string::npos) {
    text.replace(pos, pattern.size(), replace_by);
    pos = pos + replace_by.size();
  }
}

// transforme test/file.srt en /home/toto/test/file.srt
Glib::ustring create_full_path(const Glib::ustring &_path) {
  if (_path.empty())
    return Glib::ustring();

  if (Glib::path_is_absolute(_path))
    return _path;

  Glib::ustring path = _path;

  // remove ./
  {
    Glib::ustring str("./");
    if (path.compare(0, str.length(), str) == 0)
      path.replace(0, str.length(), "");
  }

  Glib::ustring curdir = Glib::get_current_dir();

  Glib::ustring newpath = Glib::build_filename(curdir, path);

  return newpath;
}

// Get the number of characters per second.
// msec = SubtitleTime::totalmsecs
double get_characters_per_second(const Glib::ustring &text, const long msecs) {
  if (msecs == 0)
    return 0;

  unsigned int len = get_text_length_for_timing(text);

  if (len == 0)
    return 0;

  double cps =
      static_cast<double>(((double)len * (double)1000) / (double)msecs);

  return cps;
}

// Count characters in a subtitle the way they need to be counted
// for subtitle timing.
unsigned int get_text_length_for_timing(const Glib::ustring &text) {
  std::vector<int> num_characters = utility::get_characters_per_line(text);

  if (num_characters.size() == 0)
    return 0;

  unsigned int len = 0;

  for (const auto &number : num_characters) {
    len += number;
  }

  len += 2 * (num_characters.size() - 1);  // a newline counts as 2 characters
  return len;
}

// Calculate the minimum acceptable duration for a string of this length.
unsigned long get_min_duration_msecs(unsigned long textlen, double maxcps) {
  if (maxcps > 0)
    return ((long)ceil((1000 * (double)textlen) / maxcps));
  else
    return 0;
}

// Calculate the minimum acceptable duration for a string of this length.
unsigned long get_min_duration_msecs(const Glib::ustring &text, double maxcps) {
  return utility::get_min_duration_msecs(
      (unsigned long)get_text_length_for_timing(text), maxcps);
}

// get number of characters for each line in the text
std::vector<int> get_characters_per_line(const Glib::ustring &text) {
  std::vector<int> num_characters;
  std::istringstream iss(utility::get_stripped_text(text));
  std::string line;

  while (std::getline(iss, line)) {
    Glib::ustring::size_type len =
        reinterpret_cast<Glib::ustring &>(line).size();
    num_characters.push_back(len);
  }

  return num_characters;
}

// get a text stripped from tags
Glib::ustring get_stripped_text(const Glib::ustring &text) {
  // pattern for tags like <i>, </i>, {\comment}, etc.
  // or space
  static bool ignore_space =
      Config::getInstance().get_value_bool("timing", "ignore-space");
  static Glib::RefPtr<Glib::Regex> tag_pattern =
      ignore_space ? Glib::Regex::create("<.*?>|{.*?}| ")
                   : Glib::Regex::create("<.*?>|{.*?}");

  return tag_pattern->replace(text, 0, "",
                              static_cast<Glib::RegexMatchFlags>(0));
}

void set_transient_parent(Gtk::Window &window) {
  Gtk::Window *root =
      dynamic_cast<Gtk::Window *>(SubtitleEditorWindow::get_instance());
  if (root)
    window.set_transient_for(*root);
}

Glib::ustring add_or_replace_extension(const Glib::ustring &filename,
                                       const Glib::ustring &extension) {
  Glib::ustring renamed;
  Glib::RefPtr<Glib::Regex> re = Glib::Regex::create("^(.*)(\\.)(.*)$");
  if (re->match(filename)) {
    renamed =
        re->replace(filename, 0, "\\1." + extension, Glib::RegexMatchFlags(0));
  } else {
    renamed = filename + "." + extension;
  }
  return renamed;
}

}  // namespace utility
