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
 
#include "Color.h"

#include <cppunit/extensions/HelperMacros.h>

class testColor : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(testColor);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testOther);
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
		unsigned int r=127, g=0, b=58, a=100;
		
		Color color(r,g,b,a);
		CPPUNIT_ASSERT_EQUAL(color.getR(), r);
		CPPUNIT_ASSERT_EQUAL(color.getG(), g);
		CPPUNIT_ASSERT_EQUAL(color.getB(), b);
		CPPUNIT_ASSERT_EQUAL(color.getA(), a);


		float rgba[4];
		color.get_value(rgba, 1);

		CPPUNIT_ASSERT_EQUAL(rgba[0], ((float)r/255));
		CPPUNIT_ASSERT_EQUAL(rgba[1], ((float)g/255));
		CPPUNIT_ASSERT_EQUAL(rgba[2], ((float)b/255));
		CPPUNIT_ASSERT_EQUAL(rgba[3], ((float)a/255));

		Color color2;
		color2.set_value(rgba, 1);

		CPPUNIT_ASSERT_EQUAL(color.to_string(), color2.to_string());

		CPPUNIT_ASSERT_EQUAL(color.getR(), color2.getR());
		CPPUNIT_ASSERT_EQUAL(color.getG(), color2.getG());
		CPPUNIT_ASSERT_EQUAL(color.getB(), color2.getB());
		CPPUNIT_ASSERT_EQUAL(color.getA(), color2.getA());

	}

	/*
	 *
	 */
	void testOther()
	{
	}
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(testColor);

