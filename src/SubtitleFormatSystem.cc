#include <memory>
#include <map>
#include "SubtitleFormatSystem.h"
#include "SubtitleFormatFactory.h"

#include "formats/AdobeEncoreDVD.h"
#include "formats/AdvancedSubStationAlpha.h"
#include "formats/MicroDVD.h"
#include "formats/MPL2.h"
#include "formats/MPsub.h"
#include "formats/SubRip.h"
#include "formats/SubStationAlpha.h"
#include "formats/SubtitleEditorProject.h"
#include "formats/SubViewer2.h"
#include "formats/TimedTextAuthoringFormat1.h"


/*
 * FIXME
 */
std::map<Glib::ustring, SubtitleFormatFactory*> m_sf_map;

/*
 * Return the instance.
 */
SubtitleFormatSystem& SubtitleFormatSystem::instance()
{
	static SubtitleFormatSystem instance;
	return instance;
}


/*
 * Constructor
 */
SubtitleFormatSystem::SubtitleFormatSystem()
{
#define REGISTER_FORMAT(x)	add_subtitle_format_factory(new SubtitleFormatFactoryRegister<x>());

	REGISTER_FORMAT(AdobeEncoreDVDNTSC);
	REGISTER_FORMAT(AdobeEncoreDVDPAL);
	REGISTER_FORMAT(AdvancedSubStationAlpha);
	REGISTER_FORMAT(MicroDVD);
	REGISTER_FORMAT(MPL2);
	REGISTER_FORMAT(MPsub);
	REGISTER_FORMAT(SubRip);
	REGISTER_FORMAT(SubStationAlpha);
	REGISTER_FORMAT(SubtitleEditorProject);
	REGISTER_FORMAT(SubViewer2);
	REGISTER_FORMAT(TimedTextAuthoringFormat1);

#undef REGISTER_FORMAT
}

/*
 * Destructor
 */
SubtitleFormatSystem::~SubtitleFormatSystem()
{
	std::map<Glib::ustring, SubtitleFormatFactory*>::iterator it;
	for(it = m_sf_map.begin(); it != m_sf_map.end(); ++it)
	{
		delete it->second;
	}
	m_sf_map.clear();
}

/*
 *  Append subtitle format.
 */
void SubtitleFormatSystem::add_subtitle_format_factory(SubtitleFormatFactory *creator)
{
	m_sf_map[creator->get_info().name] = creator;
}

/*
 * Try to determine the format of the file, and return the format name.
 * Exceptions:
 *	UnrecognizeFormatError.
 *	EncodingConvertError.
 */
Glib::ustring SubtitleFormatSystem::get_subtitle_format_from_small_contents(const Glib::ustring &uri, const Glib::ustring &charset)
{
	// Open the file and read only a small contents (max size: 1000)
	FileReader file(uri, charset, 1000);

	const Glib::ustring& contents = file.get_data();

	se_debug_message(SE_DEBUG_APP, "small content:\n%s", contents.c_str());

	Glib::RegexCompileFlags compile_flags = Glib::REGEX_MULTILINE;

	se_debug_message(SE_DEBUG_APP, "Trying to determinate the file format...");

	std::map<Glib::ustring, SubtitleFormatFactory*>::iterator it;
	for(it = m_sf_map.begin(); it != m_sf_map.end(); ++it)
	{
		SubtitleFormatFactory* sff = it->second;

		se_debug_message(SE_DEBUG_APP, "Try with '%s' format", sff->get_info().name.c_str());

		Glib::ustring pattern = sff->get_info().pattern;

		if(Glib::Regex::match_simple(pattern, contents, compile_flags))
		{
			Glib::ustring name = sff->get_info().name;

			se_debug_message(SE_DEBUG_APP, "Determine the format as '%s'", name.c_str());
			return name;
		}
	}

	throw UnrecognizeFormatError(_("Couldn't recognize format of the file."));
}

/*
 * Create a SubtitleFormat from a name.
 * Throw UnrecognizeFormatError if failed.
 */
SubtitleFormat* SubtitleFormatSystem::create_subtitle_format(const Glib::ustring &name)
{
	se_debug_message(SE_DEBUG_APP, "Trying to create the subtitle format '%s'", name.c_str());

	std::map<Glib::ustring, SubtitleFormatFactory*>::iterator it;

	it = m_sf_map.find(name);

	if(it != m_sf_map.end())
	{
		return it->second->create();
	}

	throw UnrecognizeFormatError(build_message(_("Couldn't create the subtitle format '%s'."), name.c_str()));
}

/*
 * Try to open a subtitle file from the uri.
 * If charset is empty, the automatically detection is used.
 *
 * Exceptions: UnrecognizeFormatError, EncodingConvertError, IOFileError, Glib::Error... 
 */
void SubtitleFormatSystem::open(Document *document, const Glib::ustring &uri, const Glib::ustring &charset)
{
	se_debug_message(SE_DEBUG_APP, "Trying to open the file %s with charset '%s'", uri.c_str(), charset.c_str());

	// First try to find the subtitle file type from the contents
	Glib::ustring format = get_subtitle_format_from_small_contents(uri, charset);
	
	FileReader file(uri, charset);

	std::auto_ptr<SubtitleFormat> sf( create_subtitle_format(format) );

	se_debug_message(SE_DEBUG_APP, "Trying to read the file ...");

	// init the reader
	sf->set_document(document);
	//sf->set_charset(file.get_charset());
	
	sf->open(file);

	se_debug_message(SE_DEBUG_APP, "Sets the document property ...");

	document->setFilename(Glib::filename_from_uri(uri));
	document->setCharset(file.get_charset());
	document->setNewLine(file.get_newline());
	document->setFormat(format);
	
	document->emit_signal("document-changed");
	document->emit_signal("document-property-changed");
	
	se_debug_message(SE_DEBUG_APP, "The file %s has been read with success.", uri.c_str());
}

/*
 * Save the document in a file.
 *
 * Exceptions: UnrecognizeFormatError, EncodingConvertError, IOFileError, Glib::Error... 
 */
void SubtitleFormatSystem::save(	Document *document, 
																	const Glib::ustring &uri, 
																	const Glib::ustring &format, 
																	const Glib::ustring &charset,
																	const Glib::ustring &newline)
{
	se_debug_message(SE_DEBUG_APP, 
			"Trying to save to the file '%s' as format '%s' with charset '%s' and newline '%s'", 
			uri.c_str(),
			format.c_str(),
			charset.c_str(),
			newline.c_str());

	std::auto_ptr<SubtitleFormat> sf(create_subtitle_format(format));
	// init the reader
	sf->set_document(document);

	FileWriter writer(uri, charset, newline);

	se_debug_message(SE_DEBUG_APP, "Save in the Writer...");

	sf->save(writer);

	se_debug_message(SE_DEBUG_APP, "Save to the file...");

	writer.to_file();

	se_debug_message(SE_DEBUG_APP, "Update the document property...");

	document->setCharset(charset);
	document->setFilename(Glib::filename_from_uri(uri));
	document->setFormat(format);
	document->make_document_unchanged();
	document->emit_signal("document-property-changed");

	se_debug_message(SE_DEBUG_APP, "The file %s has been save with success.", uri.c_str());
}

/*
 * Returns all information about supported subtitles.
 */
std::list<SubtitleFormatInfo> SubtitleFormatSystem::get_infos()
{
	std::list<SubtitleFormatInfo> infos;

	std::map<Glib::ustring, SubtitleFormatFactory*>::iterator it;
	for(it = m_sf_map.begin(); it != m_sf_map.end(); ++it)
		infos.push_back(it->second->get_info());

	return infos;
}

/*
 * Check if the subtitle format is supported.
 */
bool SubtitleFormatSystem::is_supported(const Glib::ustring &format)
{
	std::map<Glib::ustring, SubtitleFormatFactory*>::iterator it;
	for(it = m_sf_map.begin(); it != m_sf_map.end(); ++it)
	{
		if(it->second->get_info().name == format)
			return true;
	}
	return false;
}

