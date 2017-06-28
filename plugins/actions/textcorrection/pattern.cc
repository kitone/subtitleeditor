/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	https://kitone.github.io/subtitleeditor/
 *	https://github.com/kitone/subtitleeditor/
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

#include "pattern.h"

/*
 * Constructor
 */
Pattern::Pattern()
{
	m_enabled = true;
}

/*
 * Destructor
 * Delete rules.
 */
Pattern::~Pattern()
{
	for(std::list<Rule*>::iterator it = m_rules.begin(); it != m_rules.end(); ++it)
	{
		delete *it;
	}
	m_rules.clear();
}

/*
 * Return the name of the pattern.
 */
Glib::ustring Pattern::get_name() const
{
	return m_name;
}

/*
 * Return the name of the pattern.
 */
Glib::ustring Pattern::get_label() const
{
	return m_label;
}

/*
 * Return the name of the pattern.
 */
Glib::ustring Pattern::get_description() const
{
	return m_description;
}

/*
 * Return the active state of the pattern. (Enable by default)
 */
bool Pattern::is_enable() const
{
	return m_enabled;
}

/*
 * Apply the pattern if it is enabled.
 * With the repeat support.
 */
void Pattern::execute(Glib::ustring &text, const Glib::ustring &previous)
{
	if(!m_enabled)
		return;

	Glib::RegexMatchFlags flag = (Glib::RegexMatchFlags)0;
	for(std::list<Rule*>::iterator it = m_rules.begin(); it != m_rules.end(); ++it)
	{
		bool previous_match = true;
		if((*it)->m_previous_match)
			previous_match = (*it)->m_previous_match->match(previous);

		if((*it)->m_repeat)
		{
			while((*it)->m_regex->match(text) && previous_match)
			{
				text = (*it)->m_regex->replace(text, 0, (*it)->m_replacement, flag);
			}
		}
		else if(previous_match)
			text = (*it)->m_regex->replace(text, 0, (*it)->m_replacement, flag);
	}
}

