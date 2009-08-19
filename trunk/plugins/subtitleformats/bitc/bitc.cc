/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2009, kitone
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

#include <memory>
#include <extension/subtitleformat.h>
#include <gtkmm_utility.h>
#include <gui/dialogutility.h>
#include <utility.h>
#include <timeutility.h>

/*
 */
class DialogBITC : public Gtk::Dialog
{
	class ComboBoxFramerate : public Gtk::ComboBox
	{
		class Column : public Gtk::TreeModel::ColumnRecord
		{
		public:
			Column()
			{
				add(label);
				add(value);
			}
			Gtk::TreeModelColumn<Glib::ustring> label;
			Gtk::TreeModelColumn<FRAMERATE> value;
		};
	public:
		ComboBoxFramerate(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& xml)
		:ComboBox(cobject)
		{
			liststore = Gtk::ListStore::create(column);
			set_model(liststore);

			Gtk::CellRendererText* renderer = manage(new Gtk::CellRendererText);
			pack_start(*renderer);
			add_attribute(*renderer, "text", 0);

			liststore->set_sort_column(0, Gtk::SORT_ASCENDING);

			append(FRAMERATE_23_976);
			append(FRAMERATE_24);
			append(FRAMERATE_25, " (PAL)");
			append(FRAMERATE_29_97, " (NTSC)");
			append(FRAMERATE_30);

			set_active(0);
		}

		FRAMERATE get_value()
		{
			Gtk::TreeIter it = get_active();
			return (*it)[column.value];
		}

		void append(FRAMERATE framerate, const Glib::ustring &text = Glib::ustring())
		{
			Gtk::TreeIter it = liststore->append();
			(*it)[column.label] = get_framerate_label(framerate) + text;
			(*it)[column.value] = framerate;
		}

	protected:
		Column column;
		Glib::RefPtr<Gtk::ListStore> liststore;
	};

public:
	DialogBITC(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& xml)
	:Gtk::Dialog(cobject)
	{
		utility::set_transient_parent(*this);

		xml->get_widget_derived("combobox-framerate", m_comboFramerate);
	}

	FRAMERATE execute()
	{
		run();
		return m_comboFramerate->get_value();

	}
protected:
	ComboBoxFramerate* m_comboFramerate;
};

/*
 * BITC (Burnt-in timecode)
 */
class BITC : public SubtitleFormatIO
{
public:

	/*
	 */
	BITC()
	:m_framerate(FRAMERATE_23_976)
	{
		m_framerate_value = get_framerate_value(m_framerate);
	}

	/*
	 */
	void open(FileReader &file)
	{
		FRAMERATE framerate = create_bitc_dialog();
		m_framerate_value = get_framerate_value(framerate);

		Glib::RefPtr<Glib::Regex> re_time = Glib::Regex::create(
				"^(\\d+):(\\d+):(\\d+):(\\d+)\\s(\\d+):(\\d+):(\\d+):(\\d+)$");
		
		Subtitles subtitles = document()->subtitles();

		int start[4], end[4];
		Glib::ustring line;
		Glib::ustring text;

		Subtitle sub;

		while(file.getline(line))
		{
			if(re_time->match(line))
			{
				std::vector<Glib::ustring> group = re_time->split(line);

				start[0] = utility::string_to_int(group[1]);
				start[1] = utility::string_to_int(group[2]);
				start[2] = utility::string_to_int(group[3]);
				start[3] = utility::string_to_int(group[4]);

				end[0] = utility::string_to_int(group[5]);
				end[1] = utility::string_to_int(group[6]);
				end[2] = utility::string_to_int(group[7]);
				end[3] = utility::string_to_int(group[8]);

				file.getline(text);
				// Replace '|' by a newline
				utility::replace(text, "|", "\n");
	
				// last 00 are frame, not time!
				start[3] = start[3] * 1000 / m_framerate_value;
				end[3] = end[3] * 1000 / m_framerate_value;

				// Append a subtitle
				sub = subtitles.append();

				sub.set_text(text);
				sub.set_start_and_end(
								SubtitleTime(start[0], start[1], start[2], start[3]),
								SubtitleTime(end[0], end[1], end[2], end[3]));
			}
		}
	}

	/*
	 */
	void save(FileWriter &file)
	{
		FRAMERATE framerate = create_bitc_dialog();
		m_framerate_value = get_framerate_value(framerate);

		for(Subtitle sub = document()->subtitles().get_first(); sub; ++sub)
		{
			Glib::ustring text =sub.get_text();

			utility::replace(text, "\n", "|");

			file.write(
					Glib::ustring::compose(
						"%1 %2\n%3\n\n",
						to_bitc_time(sub.get_start()),
						to_bitc_time(sub.get_end()),
						text));
		}
	}

	/*
	 * Convert time from SE to BITC
	 * 0:00:00.000 -> 00:00:00:00 (last 00 are frames, not time!)
	 */
	Glib::ustring to_bitc_time(const SubtitleTime &t)
	{
		int frame = (int)(t.mseconds() * m_framerate_value * 0.001);

		return build_message("%02i:%02i:%02i:%02i", t.hours(), t.minutes(), t.seconds(), frame);
	}

	/*
	 */
	FRAMERATE create_bitc_dialog()
	{
		// create dialog
		std::auto_ptr<DialogBITC> dialog(
				gtkmm_utility::get_widget_derived<DialogBITC>(
						SE_DEV_VALUE(SE_PLUGIN_PATH_UI, SE_PLUGIN_PATH_DEV), 
						"dialog-bitc.ui", 
						"dialog-bitc"));
		return dialog->execute();
	}
protected:
	FRAMERATE m_framerate;
	double m_framerate_value;
};

/*
 *
 */
class BITCPlugin : public SubtitleFormat
{
public:

	/*
	 */
	SubtitleFormatInfo get_info()
	{
		SubtitleFormatInfo info;

		info.name = "BITC (Burnt-in timecode)";
		info.extension = "txt";
		info.pattern = 
			"\\d+:\\d+:\\d+:\\d+\\s\\d+:\\d+:\\d+:\\d+\\R"
			".*\\R";
		return info;
	}

	/*
	 */
	SubtitleFormatIO* create()
	{
		return new BITC;
	}
};

REGISTER_EXTENSION(BITCPlugin)
