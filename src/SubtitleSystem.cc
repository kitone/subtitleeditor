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
 
#include "SubtitleSystem.h"
#include <regex.h>
#include <fstream>
#include <iostream>
#include "utility.h"

#include "formats/SubtitleSubRip.h"
#include "formats/SubtitleMicroDVD.h"
#include "formats/SubtitleTTXT.h"
#include "formats/SubtitleMPL2.h"
#include "formats/SubtitleASS.h"
#include "formats/SubtitleSSA.h"
#include "formats/SubtitleSubViewer2.h"
#include "formats/SubtitleMPsub.h"
#include "formats/SubtitleEncorePAL.h"
#include "formats/SubtitleEncoreNTSC.h"
#include "formats/SubtitleEditorProject.h"
#include "formats/SubtitleTimedText.h"

/*
 *
 */
class IFactory
{
public:
	virtual ~IFactory() { }
	virtual SubtitleFormat* create(Document *doc) = 0;
	virtual Glib::ustring get_name() = 0;
	virtual Glib::ustring get_extension() = 0;
	virtual bool check(const std::string &line) = 0;
};

/*
 *
 */
template<class Sub>
class Factory : public IFactory
{
public:
	Factory()
	{
	}
	
	virtual SubtitleFormat* create(Document *doc)
	{
		return new Sub(doc);
	}

	virtual Glib::ustring get_name()
	{
		return Sub::get_name();
	}
	
	virtual bool check(const std::string &line)
	{
		return Sub::check(line);
	}

	virtual Glib::ustring get_extension()
	{
		return Sub::get_extension();
	}
};

/*
 *
 */
#define REGISTER_SUBTITLE(class) \
	{ \
		se_debug_message((SE_DEBUG_APP | SE_DEBUG_LOADER | SE_DEBUG_SAVER), "Register subtitle class <%s>", #class); \
		static Factory<class> factory; m_static_subtitle_formats.push_back(&factory); }

/*
 *
 */
static std::list<IFactory*> m_static_subtitle_formats;


/*
 *
 */
SubtitleSystem& SubtitleSystem::getInstance()
{
	if(m_static_subtitle_formats.empty())
	{
		se_debug_message( (SE_DEBUG_APP|SE_DEBUG_LOADER|SE_DEBUG_SAVER),
				"Init Register Subtitles");

		REGISTER_SUBTITLE(SubtitleASS);
		REGISTER_SUBTITLE(SubtitleSSA);
		REGISTER_SUBTITLE(SubtitleSubRip);
		REGISTER_SUBTITLE(SubtitleMicroDVD);
		REGISTER_SUBTITLE(SubtitleMPL2);
		REGISTER_SUBTITLE(SubtitleSubViewer2);
		REGISTER_SUBTITLE(SubtitleMPsub);
		REGISTER_SUBTITLE(SubtitleEncorePAL);
		REGISTER_SUBTITLE(SubtitleEncoreNTSC);
		REGISTER_SUBTITLE(SubtitleEditorProject);
		REGISTER_SUBTITLE(SubtitleTimedText);
#ifdef ENABLE_TTXT
		REGISTER_SUBTITLE(SubtitleTTXT);
#endif

	}
	static SubtitleSystem system;
	return system;
}


/*
 *	retourne la list des formats supporter
 */
std::list<Glib::ustring> SubtitleSystem::get_formats()
{
	static std::list<Glib::ustring> list;
	if(list.empty())
	{
		for(std::list<IFactory*>::const_iterator it=m_static_subtitle_formats.begin();
				it!=m_static_subtitle_formats.end(); ++it)
		{
			list.push_back((*it)->get_name());
		}
	}
	return list;
}


/*
 *	determine quel est le format du sous-titre
 */
Glib::ustring SubtitleSystem::find_subtitle_format(const Glib::ustring &filename)
{
	se_debug_message( (SE_DEBUG_APP|SE_DEBUG_LOADER|SE_DEBUG_SAVER),
			"Open file <%s>", filename.c_str());

	std::ifstream file(filename.c_str());

	if(!file)
	{
		se_debug_message( (SE_DEBUG_APP|SE_DEBUG_LOADER|SE_DEBUG_SAVER),
			"Open file failed <%s>.", filename.c_str());

		throw IOFileError(_("Failed to open the file for reading."));
	}

	std::string line;

	unsigned int count = 0;	

	while(!file.eof() && std::getline(file, line))
	{
		Glib::ustring fmt = find_format(line.c_str());
		
		if(!fmt.empty())
		{
			file.close();
			
			se_debug_message(
					(SE_DEBUG_APP|SE_DEBUG_LOADER|SE_DEBUG_SAVER),
					"Format detected <%s>", fmt.c_str());
			
			return fmt;		
		}

		if(++count > 100)
			break;
	}

	file.close();

	se_debug_message((SE_DEBUG_APP|SE_DEBUG_LOADER|SE_DEBUG_SAVER),
			"Can't determinate this format!");
	
	throw UnrecognizeFormatError(_("Couldn't recognize format of the file."));
}

/*
 *	crée la class du format
 */
SubtitleFormat* SubtitleSystem::create_subtitle_format(const Glib::ustring &name, Document *doc)
{
	g_return_val_if_fail(doc, NULL);

	for(std::list<IFactory*>::const_iterator it=m_static_subtitle_formats.begin();
				it!=m_static_subtitle_formats.end(); ++it)
	{
		if((*it)->get_name() == name)
		{
			se_debug_message( (SE_DEBUG_APP|SE_DEBUG_LOADER|SE_DEBUG_SAVER),
					"create subtitle format <%s>", name.c_str());

			return (*it)->create(doc);
		}
	}

	se_debug_message( (SE_DEBUG_APP|SE_DEBUG_LOADER|SE_DEBUG_SAVER),
		"create subtitle format failed <%s>", name.c_str());

	return NULL;
}

/*
 *	cherche le format de la line str
 */
Glib::ustring	SubtitleSystem::find_format(const char *str)
{
	for(std::list<IFactory*>::const_iterator it=m_static_subtitle_formats.begin();
			it!=m_static_subtitle_formats.end(); ++it)
	{
		if((*it)->check(str))
		{
			se_debug_message((SE_DEBUG_APP|SE_DEBUG_LOADER|SE_DEBUG_SAVER),
					"find format <%s> = %s", str, (*it)->get_name().c_str());
		
			return (*it)->get_name();
		}
	}

	se_debug_message((SE_DEBUG_APP|SE_DEBUG_LOADER|SE_DEBUG_SAVER),
			"find format <%s> failed.", str);
			
	return "";
}


/*
 *	retourne l'extension utiliser par le format
 *	ex: "ass", "ssa", "srt", ...
 */
Glib::ustring SubtitleSystem::get_extension(const Glib::ustring &format)
{
	for(std::list<IFactory*>::const_iterator it=m_static_subtitle_formats.begin();
				it!=m_static_subtitle_formats.end(); ++it)
	{
		if((*it)->get_name() == format)
			return (*it)->get_extension();
	}
	return "";
}

