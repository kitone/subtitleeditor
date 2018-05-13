// subtitleeditor -- a tool to create or edit subtitle
//
// https://kitone.github.io/subtitleeditor/
// https://github.com/kitone/subtitleeditor/
//
// Copyright @ 2005-2018, kitone
//
// - "justice for selected subtitles"
// a subtitleeditor plugin by Eltomito <tomaspartl@centrum.cz>
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

#include <debug.h>
#include <extension/action.h>
#include <i18n.h>
#include <utility.h>

class BestFitPlugin : public Action {
 public:
  BestFitPlugin() {
    activate();
    update_ui();
  }

  ~BestFitPlugin() {
    deactivate();
  }

  void activate() {
    se_debug(SE_DEBUG_PLUGINS);

    // actions
    action_group = Gtk::ActionGroup::create("BestFitPlugin");

    action_group->add(Gtk::Action::create(
                          "best-fit", _("_Best Fit Subtitles"),
                          _("Best fit the selected subtitles between the start "
                            "of the first and the end of the last one.")),
                      sigc::mem_fun(*this, &BestFitPlugin::on_best_fit));

    // ui
    Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

    ui_id = ui->new_merge_id();

    ui->insert_action_group(action_group);

    ui->add_ui(ui_id, "/menubar/menu-timings/best-fit", "best-fit", "best-fit");
  }

  void deactivate() {
    se_debug(SE_DEBUG_PLUGINS);

    Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

    ui->remove_ui(ui_id);
    ui->remove_action_group(action_group);
  }

  void update_ui() {
    se_debug(SE_DEBUG_PLUGINS);

    bool visible = (get_current_document() != NULL);

    action_group->get_action("best-fit")->set_sensitive(visible);
  }

 protected:
  void on_best_fit() {
    se_debug(SE_DEBUG_PLUGINS);

    Document *doc = get_current_document();
    g_return_if_fail(doc);

    // Check if the selection is contiguous and complain if it isn't.
    // Can work on multiple contiguous subtitles
    std::list<std::vector<Subtitle> > contiguous_selection;
    if (get_contiguous_selection(contiguous_selection) == false)
      return;

    doc->start_command(_("Best fit"));

    for (auto &subtitles : contiguous_selection) {
      bestfit(subtitles);
    }

    doc->emit_signal("subtitle-time-changed");
    doc->finish_command();
  }

  bool get_contiguous_selection(
      std::list<std::vector<Subtitle> > &contiguous_selection) {
    Document *doc = get_current_document();

    std::vector<Subtitle> selection = doc->subtitles().get_selection();
    if (selection.size() < 2) {
      doc->flash_message(_("Best Fit needs at least 2 subtitles to work on."));
      return false;
    }

    contiguous_selection.push_back(std::vector<Subtitle>());

    guint last_id = 0;

    for (const auto &sub : selection) {
      // Is the next subtitle?
      if (sub.get_num() == last_id + 1) {
        contiguous_selection.back().push_back(sub);
        ++last_id;
      } else {
        // Create new list only if the previous is empty.
        if (!contiguous_selection.back().empty()) {
          contiguous_selection.push_back(std::vector<Subtitle>());
        }
        contiguous_selection.back().push_back(sub);

        last_id = sub.get_num();
      }
    }

    // We check if we have at least one contiguous subtitles.
    for (const auto &subs : contiguous_selection) {
      if (subs.size() >= 2)
        return true;
    }
    doc->flash_message(
        _("Best Fit only works on an uninterrupted selection of subtitles."));
    return false;
  }

  void bestfit(std::vector<Subtitle> &subtitles) {
    if (subtitles.size() < 2)
      return;

    // Get relevant preferences
    Config &cfg = get_config();

    SubtitleTime gap = cfg.get_value_int("timing", "min-gap-between-subtitles");
    double mincps = cfg.get_value_double("timing", "min-characters-per-second");

    // SubtitleTime minlen = cfg.get_value_int("timing", "min-display");
    // long maxcpl = cfg.get_value_int("timing", "max-characters-per-line");
    // long maxcps = cfg.get_value_int("timing", "max-characters-per-second");

    SubtitleTime startime = subtitles.front().get_start();
    SubtitleTime endtime = subtitles.back().get_end();
    SubtitleTime grosstime = endtime - startime;
    SubtitleTime allgaps = gap * (subtitles.size() - 1);
    SubtitleTime nettime = grosstime - allgaps;

    // Get the total of characters counts
    long totalchars = 0;
    for (const auto &sub : subtitles) {
      totalchars += utility::get_text_length_for_timing(sub.get_text());
    }

    // Avoid divide by zero
    // Fix bug #23151 : Using best fit subtitles on zero-length subtitles
    // crashes subtitleeditor
    if (totalchars == 0)
      return;

    // Distribute available time between selected subtitles in proportion to the
    // length of their text
    long subchars = 0;
    long prevchars = 0;
    SubtitleTime intime;
    SubtitleTime prevend;
    SubtitleTime dur;
    SubtitleTime maxdur;

    for (unsigned int i = 0; i < subtitles.size(); ++i) {
      Subtitle &sub = subtitles[i];

      subchars = utility::get_text_length_for_timing(sub.get_text());

      // give this subtitle a fair share of the net time available
      dur = (nettime * subchars) / totalchars;
      // calculate proportionate start time
      intime = startime + ((grosstime * prevchars) / totalchars);

      // make sure we're not under the minimum cps
      maxdur.totalmsecs =
          (long)floor(((double)1000 * (double)subchars) / mincps);
      if (dur > maxdur)
        dur = maxdur;

      // make sure minimum gap is preserved
      // (rounding errors could've shortened it)
      if (i > 0 && (intime - prevend) < gap) {
        intime = prevend + gap;
      }

      sub.set_start_and_end(intime, intime + dur);

      prevend = intime + dur;
    }
    // reset the end time of the last subtitle to make sure rounding errors
    // didn't move it
    subtitles.back().set_end(endtime);
  }

 protected:
  Gtk::UIManager::ui_merge_id ui_id;
  Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(BestFitPlugin)
