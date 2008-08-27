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


#include <iostream>
#include <memory>
#include "Document.h"
#include "SubtitleSystem.h"
#include "SubtitleFormat.h"

#include <cppunit/extensions/HelperMacros.h>

/*
 *	Suite de teste sur les differents formats de fichier
 *	sur chaque format on teste la capabilité de déterminé le format
 *	puis la lecture du fichier du même format
 *	et enfin la validité de la lecture (simple teste si le document est vide)
 *	les testes s'effectue sur des fichiers a l'encodage UTF-8
 */
class testSubtitleLoaders : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( testSubtitleLoaders );
	CPPUNIT_TEST( testASS );
	CPPUNIT_TEST( testSSA );
	CPPUNIT_TEST( testMicroDVD );
	CPPUNIT_TEST( testMPL2 );
	CPPUNIT_TEST( testMPsub );
	CPPUNIT_TEST( testSubRip );
	CPPUNIT_TEST( testSubViewer2 );
	CPPUNIT_TEST( testAdobeEncoreNTSC );
	CPPUNIT_TEST( testAdobeEncorePAL );
	CPPUNIT_TEST( testUnrecognizeFormatError );
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp()
	{
	}

	void tearDown()
	{
	}

	void testASS()
	{
		testFormat("Advanced Sub Station Alpha", "src/unittest/ass.format");
	}

	void testSSA()
	{
		testFormat("Sub Station Alpha", "src/unittest/ssa.format");
	}

	void testMicroDVD()
	{
		testFormat("MicroDVD", "src/unittest/microdvd.format");
	}

	void testMPL2()
	{
		testFormat("MPL2", "src/unittest/mpl2.format");
	}

	void testMPsub()
	{
		testFormat("MPsub", "src/unittest/mpsub.format");
	}

	void testSubRip()
	{
		testFormat("SubRip", "src/unittest/subrip.format");
	}

	void testSubViewer2()
	{
		testFormat("SubViewer 2.0", "src/unittest/subviewer2.0.format");
	}

	void testAdobeEncorePAL()
	{
		testFormat("Adobe Encore DVD PAL", "src/unittest/adobe.encore.dvd.pal.format");
	}

	void testAdobeEncoreNTSC()
	{
		testFormat("Adobe Encore DVD NTSC", "src/unittest/adobe.encore.dvd.ntsc.format");
	}

	bool testUnrecognize(const Glib::ustring &file)
	{
		try
		{
			Glib::ustring format = SubtitleSystem::getInstance().find_subtitle_format(file);

			if(format.empty())
				return true;
		}
		catch(const UnrecognizeFormatError &ex)
		{
			return true;
		}

		return false;
	}

	void testUnrecognizeFormatError()
	{
		CPPUNIT_ASSERT(testUnrecognize("src/unittest/transcript"));
	}
	/*
	 *	verifie s'il c'est trouver le format et s'il c'est lire le document
	 */
	void testFormat(const Glib::ustring &format, const Glib::ustring &file)
	{
		CPPUNIT_ASSERT(testFindFormat(format, file));
		CPPUNIT_ASSERT(testOpenSubtitleFormat(format, file));
		testError(format, file);
	}

	/*
	 *	connait il ce format ?
	 */
	bool testFindFormat(const Glib::ustring &format, const Glib::ustring &file)
	{
		try
		{
			Glib::ustring format_find = SubtitleSystem::getInstance().find_subtitle_format(file);

			if(format_find.empty())
				return false;

			if(format_find != format)
				return false;

			return true;
		}
		catch(const UnrecognizeFormatError &ex)
		{
			std::cerr << "testFindFormat::Exception: " << ex.what() << std::endl;
		}

		return false;
	}

	/*
	 *	c'est il lire le fichier ?
	 */
	bool testOpenSubtitleFormat(const Glib::ustring &format, const Glib::ustring &file)
	{
		try
		{
			Document doc;
			SubtitleFormat *subtitle = SubtitleSystem::getInstance().create_subtitle_format(format, &doc);

			if(subtitle == NULL)
			{
				std::cerr << "testOpenSubtitleFormat::Can create subtitle: " << format << std::endl;
				return false;
			}

			subtitle->set_charset("UTF-8");

			if(!subtitle->open(file))
			{
				std::cerr << "testOpenSubtitleFormat::Can open subtitle:" << file << " (" << format << ")" << std::endl;
				return false;
			}

			delete subtitle;

			if(doc.subtitles().size() == 0)
			{
				std::cerr << "testOpenSubtitleFormat::subtitle_model()->getSize() == 0: " 
					<< file << "(" << format << ")" << std::endl;
				return false;
			}
			return true;
			
		}
		catch(const std::exception &ex)
		{
			std::cerr << "testOpenSubtitleFormat::Exception: " << ex.what() << std::endl;
		}

		return false;
	}

	/*
	 *
	 */
	void testError(const Glib::ustring &format, const Glib::ustring &file)
	{
		// file don't exist
		CPPUNIT_ASSERT(testCatchError<IOFileError>(format, file + ".do.not.exist", "UTF-8"));

		// try to test with another encoding EUC-JP
		CPPUNIT_ASSERT(testCatchError<EncodingConvertError>(format, file + ".euc-jp", "UTF-8"));
		CPPUNIT_ASSERT(!testCatchError<EncodingConvertError>(format, file + ".euc-jp", "EUC-JP"));
		//CPPUNIT_ASSERT(!testCatchError<UnrecognizeFormatError>(format, "src/unittest/transcript", "UTF-8"));
	}

	/*
	 *
	 */
	template<class Error>
	bool testCatchError(const Glib::ustring &format, const Glib::ustring &file, const Glib::ustring &charset)
	{
		try
		{
			Document doc;
			std::auto_ptr<SubtitleFormat> subtitle(SubtitleSystem::getInstance().create_subtitle_format(format, &doc));

			subtitle->set_charset(charset);

			if(subtitle->open(file))
				return false;
		}
		catch(const Error &ex)
		{
			// catch the good error
			return true;
		}
		catch(const std::exception &ex)
		{
			std::cerr <<ex.what() << std::endl;
		}

		return false;
	}
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(testSubtitleLoaders);

