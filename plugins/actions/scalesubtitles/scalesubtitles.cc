/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2010, kitone
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

#include <extension/action.h>
#include <utility.h>
#include <gtkmm_utility.h>
#include <gui/spinbuttontime.h>
#include <memory>

/*
 *
 */
class DialogScaleSubtitles : public Gtk::Dialog
{
public:
	DialogScaleSubtitles(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
	:Gtk::Dialog(cobject)
	{
		utility::set_transient_parent(*this);

		m_document = NULL;

		builder->get_widget("spin-first-number", m_spinFirstNumber);
		builder->get_widget("label-first-start-value", m_labelFirstStartValue);
		builder->get_widget_derived("spin-first-start-value", m_spinFirstStartValue);
		builder->get_widget_derived("spin-first-new-start", m_spinFirstNewStart);
		builder->get_widget("label-first-text", m_labelFirstText);

		builder->get_widget("spin-last-number", m_spinLastNumber);
		builder->get_widget("label-last-start-value", m_labelLastStartValue);
		builder->get_widget_derived("spin-last-start-value", m_spinLastStartValue);
		builder->get_widget_derived("spin-last-new-start", m_spinLastNewStart);
		builder->get_widget("label-last-text", m_labelLastText);

		builder->get_widget("radio-selected-range", m_radioSelectedRange);
		builder->get_widget("radio-all-subtitles", m_radioAllSubtitles);

		// connect signals
		m_spinFirstNumber->signal_value_changed().connect(
				sigc::mem_fun(*this, &DialogScaleSubtitles::on_spin_first_number_changed));
		m_spinLastNumber->signal_value_changed().connect(
				sigc::mem_fun(*this, &DialogScaleSubtitles::on_spin_last_number_changed));

	}

	void execute(Document *doc)
	{
		if(init_with_document(doc) == false)
			return;

		show();

		if(run() == Gtk::RESPONSE_OK)
		{
			Subtitles subtitles = doc->subtitles();

			unsigned int firstNumber = (unsigned int)m_spinFirstNumber->get_value();
			unsigned int lastNumber = (unsigned int)m_spinLastNumber->get_value();

			if(firstNumber > lastNumber)
			{
				dialog_warning(
						_("You can't use <i>scale</i> with this values."), 
						_("The first point is superior to the last point."));
			}
			else if(firstNumber == lastNumber)
			{
				dialog_warning(
						_("You can't use <i>scale</i> with this values."), 
						_("The first point is equal to the last point."));
			}
			else
			{
				Subtitle firstSubtitle = subtitles.get(firstNumber);
				Subtitle lastSubtitle = subtitles.get(lastNumber);
			
				SubtitleTime dest1( (long int)m_spinFirstNewStart->get_value() );
				SubtitleTime dest2( (long int)m_spinLastNewStart->get_value() );

				// apply change
				doc->start_command(_("Scale subtitles"));

				// Apply to all subs or selected subs
				Subtitle subbegin, subend;
				
				if(apply_to_all_subtitles())
				{
					Subtitles subs = doc->subtitles();
					subbegin = subs.get_first();
					subend = subs.get_last();
				}
				else
				{
					subbegin = firstSubtitle;
					subend = lastSubtitle;
				}

				// Apply the scale
				scale(subbegin, subend, firstSubtitle, dest1, lastSubtitle, dest2);

				doc->emit_signal("subtitle-time-changed");
				doc->finish_command();
				doc->flash_message(_("The scale was applied"));
			}
		}
		hide();
	}

protected:

	bool init_with_document(Document *doc)
	{
		g_return_val_if_fail(doc, false);

		m_document = doc;

		Subtitles subtitles = doc->subtitles();

		unsigned int subtitle_size = subtitles.size();

		if(subtitle_size == 0)
		{
			dialog_warning(
					_("You can't use <i>scale</i> with this document."), 
					build_message("The document <b>%s</b> has not subtitle, it's empty.", doc->getName().c_str()));
			return false;
		}

		// init subtitles size
		m_spinFirstNumber->set_range(1, subtitle_size);
		m_spinLastNumber->set_range(1, subtitle_size);

		m_edit_timing_mode = doc->get_edit_timing_mode();

		// init label
		m_labelFirstStartValue->set_label((m_edit_timing_mode == TIME) ? _("_Start Time:") : _("_Start Frame:"));
		m_labelLastStartValue->set_label((m_edit_timing_mode == TIME) ? _("_Start Time:") : _("_Start Frame:"));

		// init spin timing mode
		m_spinFirstStartValue->set_timing_mode(m_edit_timing_mode);
		m_spinFirstNewStart->set_timing_mode(m_edit_timing_mode);

		m_spinLastStartValue->set_timing_mode(m_edit_timing_mode);
		m_spinLastNewStart->set_timing_mode(m_edit_timing_mode);

		// set defaut value to selected subtitles
		std::vector<Subtitle> selection = subtitles.get_selection();
		if(selection.size()>1)
		{
			unsigned int first = selection.front().get_num();
			unsigned int last = selection.back().get_num();
			
			m_spinFirstNumber->set_value(first);
			m_spinLastNumber->set_value(last);
		}
		else
		{
			m_spinFirstNumber->set_value(1);
			m_spinLastNumber->set_value(subtitle_size);
		}

		// first spin init
		on_spin_first_number_changed();
		on_spin_last_number_changed();

		return true;
	}

	/*
	 */
	void on_spin_first_number_changed()
	{
		unsigned int i = (unsigned int)m_spinFirstNumber->get_value();

		Subtitle sub = m_document->subtitles().get(i);
	
		if(sub)
			init_spin(sub, m_spinFirstStartValue, m_spinFirstNewStart, m_labelFirstText);
	}

	/*
	 */
	void on_spin_last_number_changed()
	{
		unsigned int i = (unsigned int)m_spinLastNumber->get_value();

		Subtitle sub = m_document->subtitles().get(i);
			
		if(sub)
			init_spin(sub, m_spinLastStartValue, m_spinLastNewStart, m_labelLastText);
	}
	
	/*
	 */
	void init_spin(const Subtitle &subtitle, SpinButtonTime *current, SpinButtonTime *newtime, Gtk::Label *label)
	{
		// time
		long int time = (m_edit_timing_mode == TIME) ? subtitle.get_start().totalmsecs : subtitle.get_start_frame();

		current->set_value(time);
		current->set_range(time, time);
		newtime->set_value(time);

		// text
		Glib::ustring text = subtitle.get_text();

		Gtk::Tooltips tooltips;
		tooltips.set_tip(*label, text);

		label->set_text(text);
	}

	/*
	 */
	void scale(
			Subtitle &first, Subtitle &last,
			const Subtitle &sub1, const SubtitleTime &dest1,
			const Subtitle &sub2, const SubtitleTime &dest2)
	{
		SubtitleTime source1 = sub1.get_start();
		//SubtitleTime source2 = sub2.get_start();

		double scale = calcul_scale(
				sub1.get_start().totalmsecs, dest1.totalmsecs,
				sub2.get_start().totalmsecs, dest2.totalmsecs);

		++last; // stop to the next sub
		for(Subtitle subtitle=first; subtitle != last; ++subtitle)
		{
			SubtitleTime start = calcul(subtitle.get_start(), scale, source1, dest1);
			SubtitleTime end = calcul(subtitle.get_end(), scale, source1, dest1);

			subtitle.set_start_and_end(start, end);
		}
	}


	/*
	 */
	SubtitleTime calcul(
			const SubtitleTime &source, double scale,
			const SubtitleTime &sourcedisp, const SubtitleTime &destdisp)
	{
		return (source + (((source - sourcedisp) * scale) + (destdisp - sourcedisp)));
	}

	/*
	 */
	double calcul_scale(long source1, long dest1, long source2, long dest2)
	{
		return (double)(((dest2 - source2) - (dest1 - source1)) / (double)(source2 - source1));
	}

	/*
	 * Do we apply this action to all the subtitles?
	 */
	bool apply_to_all_subtitles()
	{
		if(m_radioAllSubtitles->get_active())
			return true;
		return false;
	}

protected:
	Document*					m_document;
	TIMING_MODE				m_edit_timing_mode;
	Gtk::SpinButton*	m_spinFirstNumber;
	SpinButtonTime*		m_spinFirstStartValue;
	Gtk::Label*				m_labelFirstStartValue;
	SpinButtonTime*		m_spinFirstNewStart;
	Gtk::Label*				m_labelFirstText;

	Gtk::SpinButton*	m_spinLastNumber;
	SpinButtonTime*		m_spinLastStartValue;
	Gtk::Label*				m_labelLastStartValue;
	SpinButtonTime*		m_spinLastNewStart;
	Gtk::Label*				m_labelLastText;

	Gtk::RadioButton* m_radioSelectedRange;
	Gtk::RadioButton* m_radioAllSubtitles;
};

/*
 *
 */
class ScaleSubtitlesPlugin : public Action
{
public:

	ScaleSubtitlesPlugin()
	{
		activate();
		update_ui();
	}

	~ScaleSubtitlesPlugin()
	{
		deactivate();
	}

	/*
	 *
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("ScaleSubtitlesPlugin");

		action_group->add(
				Gtk::Action::create("scale-subtitles", Gtk::Stock::CONVERT, _("_Scale"), _("Scale by two points")),
					sigc::mem_fun(*this, &ScaleSubtitlesPlugin::on_scale_subtitles));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-timings/scale-subtitles", "scale-subtitles", "scale-subtitles");
	}

	/*
	 *
	 */
	void deactivate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->remove_ui(ui_id);
		ui->remove_action_group(action_group);
	}

	/*
	 *
	 */
	void update_ui()
	{
		se_debug(SE_DEBUG_PLUGINS);

		bool visible = (get_current_document() != NULL);

		action_group->get_action("scale-subtitles")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_scale_subtitles()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();
		g_return_if_fail(doc);

		// create dialog
		std::auto_ptr<DialogScaleSubtitles> dialog(
				gtkmm_utility::get_widget_derived<DialogScaleSubtitles>(
						SE_DEV_VALUE(SE_PLUGIN_PATH_UI, SE_PLUGIN_PATH_DEV),
						"dialog-scale-subtitles.ui", 
						"dialog-scale-subtitles"));

		dialog->execute(doc);
	}
	
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(ScaleSubtitlesPlugin)
