#ifndef _ComboBoxTextColumns_h
#define _ComboBoxTextColumns_h

#include <gtkmm.h>

class ComboBoxTextColumns : public Gtk::TreeModel::ColumnRecord
{
public:
	ComboBoxTextColumns()
	{
		add(m_col_name);
		add(m_col_id);
	}
	
	Gtk::TreeModelColumn<Glib::ustring> m_col_id;
	Gtk::TreeModelColumn<Glib::ustring> m_col_name;
};

#endif//_ComboBoxTextColumns_h
