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
 

#include "subtitles.h"
#include "utility.h"
#include "document.h"

/*
 *
 */
class AppendSubtitleCommand : public Command
{
public:
	AppendSubtitleCommand(Document *doc)
	:Command(doc, _("Append subtitle"))
	{
		unsigned int path = utility::string_to_int(document()->subtitles().get_last().get("path"));

		m_path = to_string(path + 1);
	}
	
	void execute()
	{
		get_document_subtitle_model()->append();
	}

	void restore()
	{
		Gtk::TreeIter iter = get_document_subtitle_model()->get_iter(m_path);
		get_document_subtitle_model()->erase(iter);
		get_document_subtitle_model()->rebuild_column_num();
	}
protected:
	Glib::ustring m_path;
};

/*
 *
 */
class RemoveSubtitlesCommand : public Command
{
public:
	RemoveSubtitlesCommand(Document *doc, std::vector<Subtitle> &subtitles)
	:Command(doc, _("Remove Subtitles"))
	{
		m_backup.resize(subtitles.size());

		for(unsigned int i=0; i<subtitles.size(); ++i)
		{
			subtitles[i].get(m_backup[i]);
		}
	}

	void execute()
	{
		std::vector< std::map<Glib::ustring, Glib::ustring> >::reverse_iterator it;

		for(it = m_backup.rbegin(); it!= m_backup.rend(); ++it)
		{
			Gtk::TreeIter iter = get_document_subtitle_model()->get_iter((*it)["path"]);
			
			get_document_subtitle_model()->erase(iter);
		}

		get_document_subtitle_model()->rebuild_column_num();
	}

	void restore()
	{
		std::vector< std::map<Glib::ustring, Glib::ustring> >::iterator it;

		for(it = m_backup.begin(); it!= m_backup.end(); ++it)
		{
			Gtk::TreeIter newiter = get_document_subtitle_model()->append();
			
			Gtk::TreeIter path = get_document_subtitle_model()->get_iter((*it)["path"]);
			if(path)
				get_document_subtitle_model()->move(newiter, path);

			Subtitle sub(document(), newiter);
			sub.set((*it));
		}

		get_document_subtitle_model()->rebuild_column_num();
	}
protected:
	std::vector< std::map<Glib::ustring, Glib::ustring> > m_backup;
};

/*
 *
 */
class InsertSubtitleCommand : public Command
{
public:
	enum TYPE
	{
		BEFORE,
		AFTER
	};

	InsertSubtitleCommand(Document *doc, const Subtitle &sub, TYPE type)
	:Command(doc, _("Insert Subtitle")), m_type(type)
	{
		int path = utility::string_to_int(sub.get("path"));

		if(type == BEFORE)
			m_path = to_string(path);
		else
			m_path = to_string(path + 1);
	}

	void execute()
	{
		Gtk::TreeIter newiter = get_document_subtitle_model()->append();
		Gtk::TreeIter path = get_document_subtitle_model()->get_iter(m_path);

		get_document_subtitle_model()->move(newiter, path);
		get_document_subtitle_model()->rebuild_column_num();
	}

	void restore()
	{
		Gtk::TreeIter iter = get_document_subtitle_model()->get_iter(m_path);

		get_document_subtitle_model()->erase(iter);
		get_document_subtitle_model()->rebuild_column_num();
	}
protected:
	TYPE m_type;
	Glib::ustring m_path;
};



/*
 *
 */
Subtitles::Subtitles(Document &doc)
:m_document(doc)
{
}

/*
 *
 */
Subtitles::~Subtitles()
{
}

/*
 *
 */
unsigned int Subtitles::size()
{
	return m_document.get_subtitle_model()->getSize();
}

/*
 *
 */
Subtitle Subtitles::get(unsigned int num)
{
	Gtk::TreeIter iter = m_document.get_subtitle_model()->get_iter(to_string(num - 1));
	return Subtitle(&m_document, iter);
}

/*
 *
 */
Subtitle Subtitles::get_first()
{
	return Subtitle(&m_document, m_document.get_subtitle_model()->getFirst());
}


/*
 *
 */
Subtitle Subtitles::get_last()
{
	return Subtitle(&m_document, m_document.get_subtitle_model()->getLast());
}

/*
 *
 */
Subtitle Subtitles::get_previous(const Subtitle &sub)
{
	return Subtitle(&m_document, m_document.get_subtitle_model()->find_previous(sub.m_iter));
}

/*
 *
 */
Subtitle Subtitles::get_next(const Subtitle &sub)
{
	return Subtitle(&m_document, m_document.get_subtitle_model()->find_next(sub.m_iter));
}

/*
 *
 */
Subtitle Subtitles::append()
{
	if(m_document.is_recording())
		m_document.add_command(new AppendSubtitleCommand(&m_document));

	Gtk::TreeIter iter = m_document.get_subtitle_model()->append();
	return Subtitle(&m_document, iter);
}

/*
 *
 */
Subtitle Subtitles::insert_before(const Subtitle &sub)
{
	if(m_document.is_recording())
		m_document.add_command(new InsertSubtitleCommand(&m_document, sub, InsertSubtitleCommand::BEFORE));

	Gtk::TreeIter iter = sub.m_iter;
	return Subtitle(&m_document, m_document.get_subtitle_model()->insertBefore(iter));
}

/*
 *
 */
Subtitle Subtitles::insert_after(const Subtitle &sub)
{
	if(m_document.is_recording())
		m_document.add_command(new InsertSubtitleCommand(&m_document, sub, InsertSubtitleCommand::AFTER));

	Gtk::TreeIter iter = sub.m_iter;
	return Subtitle(&m_document, m_document.get_subtitle_model()->insertAfter(iter));
}

/*
 *
 */
void Subtitles::remove(std::vector<Subtitle> &subs)
{
	if(m_document.is_recording())
		m_document.add_command(new RemoveSubtitlesCommand(&m_document, subs));

	std::vector<Subtitle>::reverse_iterator it;
	for(it = subs.rbegin(); it != subs.rend(); ++it)
	{
		m_document.get_subtitle_model()->erase((*it).m_iter);
	}

	m_document.get_subtitle_model()->rebuild_column_num();
}

/*
 *
 */
void Subtitles::remove(unsigned int start, unsigned int end)
{
	std::vector<Subtitle> subs;

	Subtitle s = get(start);
	Subtitle e = get(end);

	g_return_if_fail(s);
	g_return_if_fail(e);

	for(; s != e; ++s)
		subs.push_back(s);
	subs.push_back(e);

	//std::cout << "start: " << start << " end: " << end << " diff: " << end - start << " subs: " << subs.size() << std::endl;

	remove(subs);
}

/*
 *
 */
Subtitle Subtitles::find(const SubtitleTime &time)
{
	return Subtitle(&m_document, m_document.get_subtitle_model()->find(time));
}

/*
 *
 */
Subtitle Subtitles::find_in_or_after(const SubtitleTime &time)
{
	return Subtitle(&m_document, m_document.get_subtitle_model()->find_in_or_after(time));
}

/*
 *	Selection
 */

/*
 *
 */
std::vector<Subtitle> Subtitles::get_selection()
{
	std::vector<Subtitle> array;

	std::vector<Gtk::TreeModel::Path> rows = m_document.get_subtitle_view()->get_selection()->get_selected_rows();

	if(!rows.empty())
	{
		array.resize(rows.size());

		for(unsigned int i=0; i<rows.size(); ++i)
		{
			array[i] = Subtitle(&m_document, rows[i].to_string());
		}
	}
	return array;
}

/*
 *
 */
Subtitle Subtitles::get_first_selected()
{
	std::vector<Subtitle> selection = get_selection();
	
	if(selection.empty())
		return Subtitle();

	return selection[0];
}

/*
 *
 */
void Subtitles::select(std::vector<Subtitle> &sub)
{
	for(unsigned int i=0; i< sub.size(); ++i)
	{
		m_document.get_subtitle_view()->get_selection()->select(sub[i].m_iter);
	}
}

/*
 *
 */
void Subtitles::select(const Subtitle &sub, bool start_editing)
{
	m_document.get_subtitle_view()->select_and_set_cursor(sub.m_iter, start_editing);
}

/*
 *
 */
void Subtitles::unselect(const Subtitle &sub)
{
	m_document.get_subtitle_view()->get_selection()->unselect(sub.m_iter);
}


/*
 *
 */
bool Subtitles::is_selected(const Subtitle &sub)
{
	return m_document.get_subtitle_view()->get_selection()->is_selected(sub.m_iter);
}

/*
 *
 */
void Subtitles::select_all()
{
	m_document.get_subtitle_view()->get_selection()->select_all();
}

/*
 *
 */
void Subtitles::invert_selection()
{
	Glib::RefPtr<Gtk::TreeSelection> selection = m_document.get_subtitle_view()->get_selection();

	for(Subtitle sub = get_first(); sub; ++sub)
	{
		if(selection->is_selected(sub.m_iter))
			selection->unselect(sub.m_iter);
		else
			selection->select(sub.m_iter);
			
	}
}

/*
 *
 */
void Subtitles::unselect_all()
{
	m_document.get_subtitle_view()->get_selection()->unselect_all();
}
