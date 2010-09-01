#ifndef _ErrorChecking_h
#define _ErrorChecking_h

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

#include "document.h"
#include <gtkmm/dialog.h>

/*
 *
 */
class ErrorChecking
{
public:

	/*
	 *
	 */
	class Info
	{
	public:
		Document *document;

		Subtitle currentSub;
		Subtitle nextSub;
		Subtitle previousSub;

		bool tryToFix;

		Glib::ustring error;
		Glib::ustring solution;
	};


	/*
	 *
	 */
	ErrorChecking(
			const Glib::ustring &name,
			const Glib::ustring &label, 
			const Glib::ustring &description)
	:m_name(name), m_label(label), m_description(description), m_has_configuration(false)
	{
	}

	/*
	 *
	 */
	virtual ~ErrorChecking()
	{
	}

	/*
	 *
	 */
	Glib::ustring get_name() const
	{
		return m_name;
	}

	/*
	 *
	 */
	Glib::ustring get_label() const
	{
		return m_label;
	}

	/*
	 *
	 */
	Glib::ustring get_description() const
	{
		return m_description;
	}

	/*
	 *
	 */
	void set_active(bool state)
	{
		Config::getInstance().set_value_bool(get_name(), "enabled", state);
	}
	/*
	 *
	 */
	bool get_active()
	{
		if(Config::getInstance().has_key(get_name(), "enabled") == false)
		{
			set_active(true);
		}
		return Config::getInstance().get_value_bool(get_name(), "enabled");
	}

	/*
	 *
	 */
	bool has_configuration() const
	{
		return m_has_configuration;
	}

	/*
	 *
	 */
	virtual void create_configuration()
	{
		// nothing
	}

	/*
	 *
	 */
	virtual void init()
	{
		// init from your preferences values
	}

	/*
	 *
	 */
	virtual bool execute(Info &info) // = 0;
	{
		return false;
	}

protected:
	Glib::ustring m_name;
	Glib::ustring m_label;
	Glib::ustring m_description;
	bool m_has_configuration;
};

#endif//_ErrorChecking_h
