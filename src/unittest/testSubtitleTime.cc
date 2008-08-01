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

#include "SubtitleTime.h"
#include <cppunit/extensions/HelperMacros.h>

class testSubtitleTime : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( testSubtitleTime );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testOperator );
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
		int hours = 1;
		int mins = 26;
		int secs = 59;
		int msecs = 326;
		long int totalmsecs = 3600000 * hours + 60000 * mins + 1000 * secs + msecs;

#define check(time) \
		CPPUNIT_ASSERT_EQUAL(hours, time.hours()); \
		CPPUNIT_ASSERT_EQUAL(mins, time.minutes()); \
		CPPUNIT_ASSERT_EQUAL(secs, time.seconds()); \
		CPPUNIT_ASSERT_EQUAL(msecs, time.mseconds()); \
		CPPUNIT_ASSERT_EQUAL(totalmsecs, time.totalmsecs);

		SubtitleTime a(hours, mins, secs, msecs);
		check(a);

		CPPUNIT_ASSERT(SubtitleTime(totalmsecs) == SubtitleTime("1:26:59.326"));
		CPPUNIT_ASSERT(a == SubtitleTime("1:26:59.326"));
		CPPUNIT_ASSERT(a == SubtitleTime(std::string("1:26:59.326")));
		CPPUNIT_ASSERT(a.str() == "1:26:59.326");

		SubtitleTime b("1:26:59.326");
		check(b);
#undef check

	}

	/*
	 *
	 */
	void testOperator()
	{
		SubtitleTime a(1,10,50,600);
		SubtitleTime b(2,55,10,655);
		SubtitleTime res(4,6,1,255);

		// ==
		CPPUNIT_ASSERT(SubtitleTime("1:50:30.654") == SubtitleTime(1,50,30,654));
		// !=
		CPPUNIT_ASSERT(SubtitleTime("1:50:30.650") != SubtitleTime(1,50,30,654));
		// +
		CPPUNIT_ASSERT(SubtitleTime(4,6,1,255) == SubtitleTime(1,10,50,600) + SubtitleTime(2,55,10,655));
		CPPUNIT_ASSERT(SubtitleTime(0,6,1,0) != SubtitleTime(1,10,50,600) + SubtitleTime(2,55,10,655));
		// -
		CPPUNIT_ASSERT(SubtitleTime(0,50,0,0) == SubtitleTime(1,0,0,600) - SubtitleTime(0,10,0,600));
		// *
		
		// /
		
		// >
		CPPUNIT_ASSERT(SubtitleTime(1,2,3,400) > SubtitleTime(1,0,50,600));
		// <
		CPPUNIT_ASSERT(SubtitleTime(0,2,3,400) < SubtitleTime(1,0,50,600));
		// >=
		CPPUNIT_ASSERT(SubtitleTime(1,2,3,400) >= SubtitleTime(1,2,3,400));
		CPPUNIT_ASSERT(SubtitleTime(1,2,3,400) >= SubtitleTime(1,2,3,300));
		// <=
		CPPUNIT_ASSERT(SubtitleTime(1,2,3,400) <= SubtitleTime(1,2,6,400));
		CPPUNIT_ASSERT(SubtitleTime(1,2,3,300) <= SubtitleTime(1,2,3,400));
	}

	/*
	 *
	 */
	void testOther()
	{
		CPPUNIT_ASSERT(SubtitleTime(0,0,0,0) == SubtitleTime::null());
		CPPUNIT_ASSERT(SubtitleTime("0:00:00.000") == SubtitleTime::null());
		CPPUNIT_ASSERT(SubtitleTime::validate("1:00:50.500"));
		CPPUNIT_ASSERT(SubtitleTime::validate("1:00:50,500") == false);
		CPPUNIT_ASSERT(SubtitleTime::validate("1:00:50") == false);
		CPPUNIT_ASSERT(SubtitleTime::validate("") == false);
		CPPUNIT_ASSERT(SubtitleTime::validate("hello") == false);
	}
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(testSubtitleTime);

