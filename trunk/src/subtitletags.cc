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

#include "subtitletags.h"

/*
 *
 */
SubtitleTags::SubtitleTags(const Glib::ustring &format)
:m_format(format)
{
}

/*
 *
 */
SubtitleTags::~SubtitleTags()
{
}

/*
 * Subtitle format to subtitle editor
 */
bool SubtitleTags::decode(Glib::ustring &text)
{
	return false;
}

/*
 * Subtitle editor to subtitle format
 */
bool SubtitleTags::encode(Glib::ustring &text)
{
	return false;
}

/*
 * Remove all tags
 */
bool SubtitleTags::clean(Glib::ustring &text)
{
	return false;
}

