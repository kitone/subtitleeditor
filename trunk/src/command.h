#ifndef _Command_h
#define _Command_h

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
 

#include <glibmm.h>

class Document;
class SubtitleModel;
class SubtitleView;

class Command
{
public:
	Command(Document *doc, const Glib::ustring &description);
	virtual ~Command();

	/*
	 *
	 */
	virtual void restore() = 0;

	/*
	 *
	 */
	virtual void execute() = 0;

	/*
	 *
	 */
	Document* document();

	/*
	 *
	 */
	Glib::RefPtr<SubtitleModel> get_document_subtitle_model();

	/*
	 *
	 */
	SubtitleView* get_document_subtitle_view();

	/*
	 *
	 */
	Glib::ustring description() const;
protected:
	Document* m_document;
	Glib::ustring m_description;
};


#endif//_Command_h

