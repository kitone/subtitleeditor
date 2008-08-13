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
 
#include <libglademm/xml.h>
#include <gtkmm.h>

#include "utility.h"
#include "Default.h"
#include "DocumentSystem.h"
#include "DialogUtility.h"
#include "CheckErrorsUI.h"
#include "actions/CheckErrors.h"

/*
 *	chevauchement
 *	Une erreur est détectée lorsque deux sous-titres se chevauchent.
 */
class CheckOverlapping : public CheckErrorPlugin
{
public:
	CheckOverlapping()
	:CheckErrorPlugin(
			"overlapping",
			_("Overlapping"), 
			_("An error is detected when the subtitle overlap on next subtitle."), 
			"#00ff00")
	{
	}

	/*
	 *
	 */
	Glib::ustring hasError(	Document &doc, 
													const Subtitle &currentSub, 
													const Subtitle &previousSub, 
													const Subtitle &nextSub)
	{
		if(nextSub)
		{
			if(currentSub.get_end() > nextSub.get_start())
			{
				Glib::ustring msg = build_message(
						_("<b>Subtitle overlap on next subtitle:</b> %ims overlap\n%s -> %s\n%s"), 
						(currentSub.get_end() - nextSub.get_start()).totalmsecs,
						currentSub.get_end().str().c_str(), nextSub.get_start().str().c_str(),
						currentSub.get_text().c_str()); 
				
				return msg;
			}
		}

		return Glib::ustring();
	}

	/*
	 *
	 */
	bool fixError(	Document &doc, 
									const Subtitle &currentSub, 
									const Subtitle &previousSub, 
									const Subtitle &nextSub)
	{
		return false;
	}
};



/*
 *	temps trop court
 *	Une erreur est détectée lorsque le nombre de caractères par seconde
 *	est strictement supérieur à une valeur définie par l'utilisateur.
 */
class CheckTooShortDisplayTime : public CheckErrorPlugin
{
public:
	CheckTooShortDisplayTime()
	:CheckErrorPlugin(
			"too-short-display-time",
			_("Too short display time"), 
			_("An error is detected when the number of characters per second is ..."), 
			"#d1ab70")
	{
		Config::getInstance().get_value_int("timing", "max-characters-per-second", m_maxCPS);
	}

	/*
	 *
	 */
	Glib::ustring hasError(	Document &doc, 
													const Subtitle &currentSub, 
													const Subtitle &previousSub, 
													const Subtitle &nextSub)
	{
		int cps = utility::get_characters_per_second(currentSub.get_text(), currentSub.get_duration().totalmsecs);

		if(cps > m_maxCPS)
		{
			return build_message(
					_("<b>Subtitle display time is too short:</b> %i char/s\n%s -> %s\n%s"), 
					(int)cps,
					currentSub.get_start().str().c_str(), currentSub.get_end().str().c_str(),
					currentSub.get_text().c_str()); 
		}

		return Glib::ustring();
	}

	/*
	 *
	 */
	bool fixError(	Document &doc, 
									const Subtitle &currentSub, 
									const Subtitle &previousSub, 
									const Subtitle &nextSub)
	{
		return false;
	}
protected:
	int m_maxCPS;
};


/*
 *	temps trop long
 *	Une erreur est détectée lorsque le nombre de caractères par seconde
 *	est strictement inférieur à une valeur définie par l'utilisateur.
 */
class CheckTooLongDisplayTime : public CheckErrorPlugin
{
public:
	CheckTooLongDisplayTime()
	:CheckErrorPlugin(
			"too-long-display-time",
			_("Too long display time"), 
			_("An error is detected when the number ..."), 
			"#ff3131")
	{
		Config::getInstance().get_value_int("timing", "min-characters-per-second", m_minCPS);
	}

	/*
	 *
	 */
	Glib::ustring hasError(	Document &doc, 
													const Subtitle &currentSub, 
													const Subtitle &previousSub, 
													const Subtitle &nextSub)
	{
		int cps = utility::get_characters_per_second(currentSub.get_text(), currentSub.get_duration().totalmsecs);

		if(cps < m_minCPS)
		{
			return build_message(
					_("<b>Subtitle display time is too long:</b> %i char/s\n%s -> %s\n%s"), 
					(int)cps,
					currentSub.get_start().str().c_str(), currentSub.get_end().str().c_str(),
					currentSub.get_text().c_str()); 
		}

		return Glib::ustring();
	}

	/*
	 *
	 */
	bool fixError(	Document &doc, 
									const Subtitle &currentSub, 
									const Subtitle &previousSub, 
									const Subtitle &nextSub)
	{
		return false;
	}
protected:
	int m_minCPS;
};


/*
 *	ligne trop longue
 *	Une erreur est détectée lorsque la longueur d'une ligne de sous-titre
 *	est strictement supérieur à une valeur spécifiée par l'utilisateur.
 */
class CheckTooLongLine : public CheckErrorPlugin
{
public:

	enum TYPE
	{
		TEXT,
		TRANSLATION
	};

	CheckTooLongLine(TYPE type)
	:CheckErrorPlugin(
			"too-long-line",
			_("Too Long line"), 
			_("An error is detected when ..."), 
			"#f8bcff"), m_type(type)
	{
		Config::getInstance().get_value_int("timing", "max-characters-per-line", m_maxCPL);
	}

	/*
	 *
	 */
	Glib::ustring hasError(	Document &doc, 
													const Subtitle &currentSub, 
													const Subtitle &previousSub, 
													const Subtitle &nextSub)
	{
		std::istringstream iss((m_type == TEXT) ? currentSub.get_characters_per_line_text() : currentSub.get_characters_per_line_translation() );	
		std::string line;
	
		int val=0;

		while(std::getline(iss, line))
		{
			from_string(line, val);

			if(val > m_maxCPL)
			{
				return build_message(
									_("<b>Subtitle has a too long line (%s):</b> %i characters\n%s -> %s\n%s"),
									(m_type == TEXT) ? _("text") : _("translation"),
									val,
									currentSub.get_start().str().c_str(), currentSub.get_end().str().c_str(),
									(m_type == TEXT) ? currentSub.get_text().c_str() : currentSub.get_translation().c_str()); 
			}
		}
		return Glib::ustring();
	}

	/*
	 *
	 */
	bool fixError(	Document &doc, 
									const Subtitle &currentSub, 
									const Subtitle &previousSub, 
									const Subtitle &nextSub)
	{
		return false;
	}
protected:
	TYPE m_type;
	int m_maxCPL;
};




/*
 *	trop de ligne dans le sous-titre
 */
class CheckMaxLinePerSubtitle : public CheckErrorPlugin
{
public:
	enum TYPE
	{
		TEXT,
		TRANSLATION
	};

	CheckMaxLinePerSubtitle(TYPE type)
	:CheckErrorPlugin(
			"max-line-per-subtitle",
			_("Max lines per subtitle"), 
			_("An error is detected when ..."), 
			"#94b6f0"), m_type(type)
	{
		Config::getInstance().get_value_int("timing", "max-line-per-subtitle", m_maxLinePerSubtitle);
	}

	/*
	 *
	 */
	Glib::ustring hasError(	Document &doc, 
													const Subtitle &currentSub, 
													const Subtitle &previousSub, 
													const Subtitle &nextSub)
	{
		// >= m_maxCPL
		std::istringstream iss((m_type == TEXT) ? currentSub.get_characters_per_line_text() : currentSub.get_characters_per_line_translation());
	
		std::string line;
	
		int count=0;

		while(std::getline(iss, line))
		{
			++count;
		}

		if(count > m_maxLinePerSubtitle)
		{
			return build_message(
								_("<b>Subtitle has too many lines (%s):</b> %i\n%s -> %s\n%s"),
								(m_type) ? _("text") : _("translation"),
								count,
								currentSub.get_start().str().c_str(), 
								currentSub.get_end().str().c_str(),
								(m_type == TEXT) ? currentSub.get_text().c_str() : currentSub.get_translation().c_str()); 
		}

		return Glib::ustring();
	}

	/*
	 *
	 */
	bool fixError(	Document &doc, 
									const Subtitle &currentSub, 
									const Subtitle &previousSub, 
									const Subtitle &nextSub)
	{
		return false;
	}
protected:
	TYPE m_type;
	int m_maxLinePerSubtitle;
};



/*
 *	transition
 *	Respect du temps entre chaque s-t
 */
class CheckMinGapBetweenSubtitles : public CheckErrorPlugin
{
public:
	CheckMinGapBetweenSubtitles()
	:CheckErrorPlugin(
			"min-gap-between-subtitles",
			_("Minimum gap between subtitles"), 
			_("An error is detected when the minimum gap between subtitles is too short."), 
			"#be7400")
	{
		Config::getInstance().get_value_int("timing", "min-gap-between-subtitles", m_minGBS);
	}

	/*
	 *
	 */
	Glib::ustring hasError(	Document &doc, 
													const Subtitle &currentSub, 
													const Subtitle &previousSub, 
													const Subtitle &nextSub)
	{
		if(nextSub)
		{
			long diff = (nextSub.get_start() - currentSub.get_end()).totalmsecs;

			if(diff < m_minGBS)
			{
				return build_message(
						_("<b>Too short gap between subtitle:</b> %ims"), diff);
			}
		}
		
		return Glib::ustring();
	}

	/*
	 *
	 */
	bool fixError(	Document &doc, 
									const Subtitle &currentSub, 
									const Subtitle &previousSub, 
									const Subtitle &nextSub)
	{
		return false;
	}
protected:
	int m_minGBS;
};

/*
 *
 */
class CellRendererErrorColor : public Gtk::CellRendererText
{
public:
	CellRendererErrorColor()
	:Glib::ObjectBase(typeid(CellRendererErrorColor)),
	Gtk::CellRendererText()
	{
		radius = 15;
	}

	/*
	 *
	 */
	void render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window, 
			Gtk::Widget& widget, 
			const Gdk::Rectangle& background_area,
			const Gdk::Rectangle& cell_area,
			const Gdk::Rectangle& expose_area,
			Gtk::CellRendererState flags)
	{
		int x = cell_area.get_x() + property_xpad();
		int y = cell_area.get_y() + property_ypad();

		Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
		
		Glib::RefPtr<Gdk::Colormap> colormap = widget.get_default_colormap();

		Color rgba(property_text());

		Gdk::Color color;
		color.set_rgb(rgba.getR() * 257, rgba.getG() * 257, rgba.getB() * 257);

		colormap->alloc_color(color);
		
		gc->set_foreground(color);
		
		window->draw_arc(gc, true, x, y, radius, radius, 0 , 23040);//360 * 64);

		colormap->free_color(color);
	}

	/*
	 *
	 */
	void get_size_vfunc(
						Gtk::Widget& widget, 
						const Gdk::Rectangle* cell_area, 
						int* x_offset, 
						int* y_offset, 
						int* width, 
						int* height) const
	{
		*width = property_xpad() * 2 + radius;
		*height = property_ypad() * 2 + radius;
	}
protected:
	int radius;
};

/*
 *
 */
class ErrorsView : public Gtk::TreeView
{
public:


	class Column : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Column()
		{
			add(color);
			add(line);
			add(text);
		}
		
		Gtk::TreeModelColumn<Glib::ustring> color;
		Gtk::TreeModelColumn<Glib::ustring>	line;
		Gtk::TreeModelColumn<Glib::ustring>	text;
	};


public:

	/*
	 *
	 */
	ErrorsView(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::TreeView(cobject)
	{
		m_model = Gtk::ListStore::create(m_column);

		set_model(m_model);

		// color
		{
			Gtk::TreeViewColumn* column = NULL;
			CellRendererErrorColor* renderer = NULL;

			column = manage(new Gtk::TreeViewColumn);
			renderer = manage(new CellRendererErrorColor);
			renderer->property_yalign() = 0;

			column->pack_start(*renderer, false);
			column->add_attribute(renderer->property_text(), m_column.color);

			append_column(*column);
		}
		// line
		{
			Gtk::TreeViewColumn* column = NULL;
			Gtk::CellRendererText* renderer = NULL;

			column = manage(new Gtk::TreeViewColumn(_("Line")));
			renderer = manage(new Gtk::CellRendererText);
			renderer->property_yalign() = 0;

			column->pack_start(*renderer, false);
			column->add_attribute(renderer->property_text(), m_column.line);

			append_column(*column);
		}
		// error
		{
			Gtk::TreeViewColumn* column = NULL;
			Gtk::CellRendererText* renderer = NULL;

			column = manage(new Gtk::TreeViewColumn(_("Errors")));
			renderer = manage(new Gtk::CellRendererText);

			column->pack_start(*renderer, false);
			column->add_attribute(renderer->property_markup(), m_column.text);

			append_column(*column);
		}

		set_rules_hint(true);

		get_selection()->signal_changed().connect(
				sigc::mem_fun(*this, &ErrorsView::on_selection_changed));
		show_all();
	}

	/*
	 *
	 */
	void add_message(unsigned int line, const Glib::ustring &text, const Glib::ustring &color)
	{
		Gtk::TreeIter iter = m_model->append();

		(*iter)[m_column.color] = color;
		(*iter)[m_column.line] = to_string(line);
		(*iter)[m_column.text] = text;
	}

	/*
	 *
	 */
	void clear()
	{
		m_model->clear();
	}

	/*
	 *
	 */
	void on_selection_changed()
	{
		Gtk::TreeIter iter = get_selection()->get_selected();

		if(!iter)
			return;

		unsigned int num = utility::string_to_int(Glib::ustring((*iter)[m_column.line]));

		Document *doc = DocumentSystem::getInstance().getCurrentDocument();
		if(doc)
		{
			Subtitle sub = doc->subtitles().get(num);
			if(sub)
				doc->subtitles().select(sub);
		}
	}
protected:
	Column m_column;
	Glib::RefPtr<Gtk::ListStore> m_model;
};

/*
 *
 */
class DialogCheckErrorsPreferences : public Gtk::Dialog
{
public:
	DialogCheckErrorsPreferences(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::Dialog(cobject)
	{
		refGlade->get_widget("check-overlapping", m_checkOverlapping);
		refGlade->get_widget("check-too-short-display-time", m_checkTooShortDisplayTime);
		refGlade->get_widget("check-too-long-display-time", m_checkTooLongDisplayTime);
		refGlade->get_widget("check-too-long-line", m_checkTooLongLine);
		refGlade->get_widget("check-gap-between-subtitles", m_checkGabBetweenSubtitles);
		refGlade->get_widget("check-max-line-per-subtitle", m_checkMaxLinePerSubtitle);

		refGlade->get_widget("spin-min-characters-per-second", m_spinMinCharactersPerSecond);
		refGlade->get_widget("spin-max-characters-per-second", m_spinMaxCharactersPerSecond);
		refGlade->get_widget("spin-min-gap-between-subtitles", m_spinMinGapBetweenSubtitles);
		refGlade->get_widget("spin-min-display", m_spinMinDisplay);
		refGlade->get_widget("spin-max-characters-per-line", m_spinMaxCharactersPerLine);
		refGlade->get_widget("spin-max-line-per-subtitle", m_spinMaxLinePerSubtitle);

		refGlade->get_widget("color-overlapping", m_colorOverlapping);
		refGlade->get_widget("color-too-short-display-time", m_colorTooShortDisplayTime);
		refGlade->get_widget("color-too-long-display-time", m_colorTooLongDisplayTime);
		refGlade->get_widget("color-too-long-line", m_colorTooLongLine);
		refGlade->get_widget("color-gap-between-subtitles", m_colorGabBetweenSubtitles);
		refGlade->get_widget("color-max-line-per-subtitle", m_colorMaxLinePerSubtitle);

		WidgetToConfig::read_config_and_connect(m_checkOverlapping, "dialog-check-errors", "check-overlapping");
		WidgetToConfig::read_config_and_connect(m_checkTooShortDisplayTime, "dialog-check-errors", "check-too-short-display-time");
		WidgetToConfig::read_config_and_connect(m_checkTooLongDisplayTime, "dialog-check-errors", "check-too-long-display-time");
		WidgetToConfig::read_config_and_connect(m_checkTooLongLine, "dialog-check-errors", "check-too-long-line");
		WidgetToConfig::read_config_and_connect(m_checkGabBetweenSubtitles, "dialog-check-errors", "check-gap-between-subtitles");
		WidgetToConfig::read_config_and_connect(m_checkMaxLinePerSubtitle, "dialog-check-errors", "check-max-line-per-subtitle");

		WidgetToConfig::read_config_and_connect(m_spinMinCharactersPerSecond, "timing", "min-characters-per-second");
		WidgetToConfig::read_config_and_connect(m_spinMaxCharactersPerSecond, "timing", "max-characters-per-second");
		WidgetToConfig::read_config_and_connect(m_spinMinGapBetweenSubtitles, "timing", "min-gap-between-subtitles");
		WidgetToConfig::read_config_and_connect(m_spinMinDisplay, "timing", "min-display");
		WidgetToConfig::read_config_and_connect(m_spinMaxCharactersPerLine, "timing", "max-characters-per-line");
		WidgetToConfig::read_config_and_connect(m_spinMaxLinePerSubtitle, "timing", "max-line-per-subtitle");

		WidgetToConfig::read_config_and_connect(m_colorOverlapping, "check-error-plugins", "overlapping-color");
		WidgetToConfig::read_config_and_connect(m_colorTooShortDisplayTime, "check-error-plugins", "too-short-display-time-color");
		WidgetToConfig::read_config_and_connect(m_colorTooLongDisplayTime, "check-error-plugins", "too-long-display-time-color");
		WidgetToConfig::read_config_and_connect(m_colorTooLongLine, "check-error-plugins", "too-long-line-color");
		WidgetToConfig::read_config_and_connect(m_colorGabBetweenSubtitles, "check-error-plugins", "min-gap-between-subtitles-color");
		WidgetToConfig::read_config_and_connect(m_colorMaxLinePerSubtitle, "check-error-plugins", "max-line-per-subtitle-color");

	}
protected:
	Gtk::CheckButton* m_checkOverlapping;
	Gtk::CheckButton* m_checkTooShortDisplayTime;
	Gtk::CheckButton* m_checkTooLongDisplayTime;
	Gtk::CheckButton* m_checkTooLongLine;
	Gtk::CheckButton* m_checkGabBetweenSubtitles;
	Gtk::CheckButton* m_checkMaxLinePerSubtitle;

	Gtk::SpinButton* m_spinMinCharactersPerSecond;
	Gtk::SpinButton* m_spinMaxCharactersPerSecond;
	Gtk::SpinButton* m_spinMinGapBetweenSubtitles;
	Gtk::SpinButton* m_spinMinDisplay;
	Gtk::SpinButton* m_spinMaxCharactersPerLine;
	Gtk::SpinButton* m_spinMaxLinePerSubtitle;

	Gtk::ColorButton* m_colorOverlapping;
	Gtk::ColorButton* m_colorTooShortDisplayTime;
	Gtk::ColorButton* m_colorTooLongDisplayTime;
	Gtk::ColorButton* m_colorTooLongLine;
	Gtk::ColorButton* m_colorGabBetweenSubtitles;
	Gtk::ColorButton* m_colorMaxLinePerSubtitle;
};

/*
 *
 */
class DialogCheckErrors : public Gtk::Window
{
public:
	DialogCheckErrors(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::Window(cobject)
	{
		refGlade->get_widget_derived("treeview-errors", m_treeviewErrors);

		refGlade->get_widget("button-refresh", m_buttonRefresh);
		refGlade->get_widget("button-preferences", m_buttonPreferences);
		refGlade->get_widget("statusbar", m_statusbar);

		m_buttonRefresh->signal_clicked().connect(
				sigc::mem_fun(*this, &DialogCheckErrors::check));

		m_buttonPreferences->signal_clicked().connect(
				sigc::mem_fun(*this, &DialogCheckErrors::on_button_preferences));

		check();

		show();
	}

	void on_button_preferences()
	{
		DialogCheckErrorsPreferences *dialog = 
			utility::get_widget_derived<DialogCheckErrorsPreferences>("dialog-check-errors-preferences.glade", "dialog-check-errors-preferences");

		dialog->run();

		delete dialog;

		check();
	}

	void check()
	{
		bool use_overlapping;
		bool use_too_short_display_time;
		bool use_too_long_display_time;
		bool use_too_long_line;
		bool use_gap_between_subtitles;
		bool use_max_line_per_subtitle;
	
		//
		Config &cfg = Config::getInstance();

		cfg.get_value_bool("dialog-check-errors", "check-overlapping", use_overlapping);
		cfg.get_value_bool("dialog-check-errors", "check-too-short-display-time", use_too_short_display_time);
		cfg.get_value_bool("dialog-check-errors", "check-too-long-display-time", use_too_long_display_time);
		cfg.get_value_bool("dialog-check-errors", "check-too-long-line", use_too_long_line);
		cfg.get_value_bool("dialog-check-errors", "check-gap-between-subtitles", use_gap_between_subtitles);
		cfg.get_value_bool("dialog-check-errors", "check-max-line-per-subtitle", use_max_line_per_subtitle);
		//

		m_treeviewErrors->clear();

		Document *doc = DocumentSystem::getInstance().getCurrentDocument();

		if(doc == NULL)
		{
			m_statusbar->push(_("Document not found."));
			return;
		}

		std::vector<CheckErrorPlugin*> plugins;
		
		if(use_overlapping)
			plugins.push_back(new CheckOverlapping);

		if(use_gap_between_subtitles)
			plugins.push_back(new CheckMinGapBetweenSubtitles);
		
		if(use_too_short_display_time)
			plugins.push_back(new CheckTooShortDisplayTime);
		
		if(use_too_long_display_time)
			plugins.push_back(new CheckTooLongDisplayTime);
		
		if(use_too_long_line)
		{
			plugins.push_back(new CheckTooLongLine(CheckTooLongLine::TEXT));
			plugins.push_back(new CheckTooLongLine(CheckTooLongLine::TRANSLATION));
		}
		
		if(use_max_line_per_subtitle)
		{
			plugins.push_back(new CheckMaxLinePerSubtitle(CheckMaxLinePerSubtitle::TEXT));
			plugins.push_back(new CheckMaxLinePerSubtitle(CheckMaxLinePerSubtitle::TRANSLATION));
		}

		unsigned int count = 0;

		Subtitles subtitles = doc->subtitles();

		Subtitle current, previous, next;

		for(current = subtitles.get_first(); current; ++current)
		{
			next = current; ++next;

			for(unsigned int i=0; i<plugins.size(); ++i)
			{
				Glib::ustring error = plugins[i]->hasError(*doc, current, previous, next);
			
				if(!error.empty())
				{
					m_treeviewErrors->add_message(current.get_num(), error, plugins[i]->get_color());
					++count;
				}
			}

			previous = current;
		}

		
		for(unsigned int i=0; i<plugins.size(); ++i)
			delete plugins[i];
		plugins.clear();

		m_statusbar->push(build_message("%i error(s) found.", count));
	}
protected:
	Gtk::Button* m_buttonRefresh;
	Gtk::Button* m_buttonPreferences;
	ErrorsView* m_treeviewErrors;
	Gtk::Statusbar* m_statusbar;
};


void createDialogCheckErrors()
{
	DialogCheckErrors *dialog = utility::get_widget_derived<DialogCheckErrors>("dialog-check-errors.glade", "dialog-check-errors");
	dialog->show();
}

