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


#include "RegEx.h"
#include <cppunit/extensions/HelperMacros.h>

class testRegEx : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( testRegEx );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testOther );
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp()
	{
	}

	void tearDown()
	{
	}


	/*
	 *
	 */
	void testConstructor()
	{
	}

	/*
	 *
	 */
	void testOther()
	{
		/*
		CPPUNIT_ASSERT( RegEx(
					"([0-9]{2}|[0-9]):[0-9]{2}:[0-9]{2},[0-9]{3}"
					"--> "
					"([0-9]{2}|[0-9]):[0-9]{2}:[0-9]{2},[0-9]{3}"
					).exec("1:10:20,556 --> 2:15:00,132"));
		
		CPPUNIT_ASSERT( RegEx("^--").exec("--Bonjours!") );
		CPPUNIT_ASSERT( RegEx("^--").exec("blabla --") == false);
		CPPUNIT_ASSERT( RegEx("(^--|$!)").exec("--Bonjours!") );
		*/
	}
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(testRegEx);

