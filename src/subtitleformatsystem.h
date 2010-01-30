#ifndef _SubtitleFormatSystem_h
#define _SubtitleFormatSystem_h

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
#include "subtitleformatio.h"

class SubtitleFormat;

typedef std::list<SubtitleFormat*> SubtitleFormatList;

/*
 *
 */
class SubtitleFormatSystem
{
public:

	/*
	 * Return the instance.
	 */
	static SubtitleFormatSystem& instance();

	/*
	 * Try to open a subtitle file from the uri.
	 * If charset is empty, the automatically detection is used.
	 *
	 * Exceptions: UnrecognizeFormatError, EncodingConvertError, IOFileError, Glib::Error... 
	 */
	void open(Document *document, const Glib::ustring &uri, const Glib::ustring &charset);

	/*
	 * Save the document in a file.
	 *
	 * Exceptions: UnrecognizeFormatError, EncodingConvertError, IOFileError, Glib::Error... 
	 */
	void save(	Document *document, 
							const Glib::ustring &uri, 
							const Glib::ustring &charset, 
							const Glib::ustring &format,
							const Glib::ustring &newline);

	/*
	 * Returns all information about supported subtitles.
	 */
	std::list<SubtitleFormatInfo> get_infos();

	/*
	 * Return information about the subtitle format.
	 */
	bool get_info(const Glib::ustring &subtitle_format, SubtitleFormatInfo &info);

	/*
	 * Check if the subtitle format is supported.
	 */
	bool is_supported(const Glib::ustring &format);

protected:

	/*
	 * Constructor
	 */
	SubtitleFormatSystem();

	/*
	 * Destructor
	 */
	~SubtitleFormatSystem();

	/*
	 * Try to determine the format of the file, and return the format name.
	 * Exceptions:
	 *	UnrecognizeFormatError.
	 *	EncodingConvertError.
	 */
	Glib::ustring get_subtitle_format_from_small_contents(const Glib::ustring &uri, const Glib::ustring &charset);

	/*
	 * Create a SubtitleFormat from a name.
	 * Throw UnrecognizeFormatError if failed.
	 */
	SubtitleFormatIO* create_subtitle_format_io(const Glib::ustring &name);

	/*
	 * Return a list of SubtitleFormat from ExtensionManager.
	 */
	SubtitleFormatList get_subtitle_format_list();
};

#endif//_SubtitleFormatSystem_h
