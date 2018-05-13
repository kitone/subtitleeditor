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

#include <gtkmm.h>
#include "cfg.h"
#include "debug.h"
#include "widget_config_utility.h"

namespace widget_config {

void on_check_button(Gtk::CheckButton *widget, const Glib::ustring &group,
                     const Glib::ustring &key) {
  Config::getInstance().set_value_bool(group, key, widget->get_active());
}

void on_font_button(Gtk::FontButton *widget, const Glib::ustring &group,
                    const Glib::ustring &key) {
  Config::getInstance().set_value_string(group, key, widget->get_font_name());
}

void on_color_button(Gtk::ColorButton *widget, const Glib::ustring &group,
                     const Glib::ustring &key) {
  Color color;
  color.getFromColorButton(*widget);

  Config::getInstance().set_value_color(group, key, color);
}

void on_range(Gtk::Range *range, const Glib::ustring &group,
              const Glib::ustring &key) {
  Config::getInstance().set_value_double(group, key, range->get_value());
}

void on_entry(Gtk::Entry *spin, const Glib::ustring &group,
              const Glib::ustring &key) {
  Config::getInstance().set_value_string(group, key, spin->get_text());
}

void on_spin_button(Gtk::SpinButton *spin, const Glib::ustring &group,
                    const Glib::ustring &key) {
  Config::getInstance().set_value_double(group, key, spin->get_value());
}

void on_combobox_text(Gtk::ComboBoxText *combo, const Glib::ustring &group,
                      const Glib::ustring &key) {
  Config::getInstance().set_value_string(group, key, combo->get_active_text());
}

void connect(Gtk::Widget *widget, const Glib::ustring &group,
             const Glib::ustring &key) {
  if (Gtk::CheckButton *check = dynamic_cast<Gtk::CheckButton *>(widget)) {
    check->signal_toggled().connect(
        sigc::bind<Gtk::CheckButton *, Glib::ustring, Glib::ustring>(
            sigc::ptr_fun(&on_check_button), check, group, key));
  } else if (Gtk::Range *range = dynamic_cast<Gtk::Range *>(widget)) {
    range->signal_value_changed().connect(
        sigc::bind<Gtk::Range *, Glib::ustring, Glib::ustring>(
            sigc::ptr_fun(&on_range), range, group, key));
  } else if (Gtk::SpinButton *spin = dynamic_cast<Gtk::SpinButton *>(widget)) {
    spin->signal_value_changed().connect(
        sigc::bind<Gtk::SpinButton *, Glib::ustring, Glib::ustring>(
            sigc::ptr_fun(&on_spin_button), spin, group, key));
  } else if (Gtk::Entry *entry = dynamic_cast<Gtk::Entry *>(widget)) {
    // entry->signal_activate().connect(
    entry->signal_changed().connect(
        sigc::bind<Gtk::Entry *, Glib::ustring, Glib::ustring>(
            sigc::ptr_fun(&on_entry), entry, group, key));
  } else if (Gtk::FontButton *font = dynamic_cast<Gtk::FontButton *>(widget)) {
    font->signal_font_set().connect(
        sigc::bind<Gtk::FontButton *, Glib::ustring, Glib::ustring>(
            sigc::ptr_fun(&on_font_button), font, group, key));
  } else if (Gtk::ColorButton *color =
                 dynamic_cast<Gtk::ColorButton *>(widget)) {
    color->signal_color_set().connect(
        sigc::bind<Gtk::ColorButton *, Glib::ustring, Glib::ustring>(
            sigc::ptr_fun(&on_color_button), color, group, key));
  } else if (Gtk::ComboBoxText *combobox =
                 dynamic_cast<Gtk::ComboBoxText *>(widget)) {
    combobox->signal_changed().connect(
        sigc::bind<Gtk::ComboBoxText *, Glib::ustring, Glib::ustring>(
            sigc::ptr_fun(&on_combobox_text), combobox, group, key));
  }
}

void read_config(Gtk::Widget *widget, const Glib::ustring &group,
                 const Glib::ustring &key) {
  Config &cfg = Config::getInstance();

  if (Gtk::CheckButton *check = dynamic_cast<Gtk::CheckButton *>(widget)) {
    bool value = false;
    if (cfg.get_value_bool(group, key, value)) {
      check->set_active(value);
    }
  } else if (Gtk::Range *range = dynamic_cast<Gtk::Range *>(widget)) {
    double value = 0;
    if (cfg.get_value_double(group, key, value)) {
      range->set_value(value);
    }
  } else if (Gtk::SpinButton *spin = dynamic_cast<Gtk::SpinButton *>(widget)) {
    double value;
    if (cfg.get_value_double(group, key, value)) {
      spin->set_value(value);
    }
  } else if (Gtk::Entry *entry = dynamic_cast<Gtk::Entry *>(widget)) {
    Glib::ustring value;
    if (cfg.get_value_string(group, key, value)) {
      entry->set_text(value);
    }
  } else if (Gtk::FontButton *font = dynamic_cast<Gtk::FontButton *>(widget)) {
    Glib::ustring value;
    if (cfg.get_value_string(group, key, value)) {
      font->set_font_name(value);
    }
  } else if (Gtk::ColorButton *colorbutton =
                 dynamic_cast<Gtk::ColorButton *>(widget)) {
    Color color;
    cfg.get_value_color(group, key, color);

    color.initColorButton(*colorbutton);
  } else if (Gtk::ComboBoxText *combobox =
                 dynamic_cast<Gtk::ComboBoxText *>(widget)) {
    Glib::ustring value;
    if (cfg.get_value_string(group, key, value)) {
      combobox->set_active_text(value);
    }
  }
}

void read_config_and_connect(Gtk::Widget *widget, const Glib::ustring &group,
                             const Glib::ustring &key) {
  g_return_if_fail(widget);

  read_config(widget, group, key);
  connect(widget, group, key);
}

}  // namespace widget_config
