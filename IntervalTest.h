/*
* IntervalTest.h
*
*  Created on: Sept 29, 2019
*      Author: Hernan Diaz
*/
#pragma once

#include "heading.h"
#include <iostream>
#include "Interval.h"
#include <cppunit\TestFixture.h>
#include <cppunit\extensions\HelperMacros.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/ui/text/TestRunner.h>

class IntervalTest : public CppUnit::TestFixture {
	double const tolerance = 1e-10;

	CPPUNIT_TEST_SUITE(IntervalTest);
	CPPUNIT_TEST(testLei2011FormulaCase1);
	CPPUNIT_TEST(testLei2011FormulaCase2);
	CPPUNIT_TEST(testLei2011FormulaCase3);
	CPPUNIT_TEST(testLei2011FormulaCase4);
	CPPUNIT_TEST(testLei2011FormulaCase5);
	CPPUNIT_TEST(testLei2011AEqualsA);
	CPPUNIT_TEST(testLei2011AisGreaterEqualToB);
	CPPUNIT_TEST(testLex1AisGreaterThanB);
	CPPUNIT_TEST(testLex2AisGreaterThanB);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();

	void tearDown();

	void testLei2011FormulaCase1();

	void testLei2011FormulaCase2();

	void testLei2011FormulaCase3();

	void testLei2011FormulaCase4();

	void testLei2011FormulaCase5();

	void IntervalTest::testLei2011AEqualsA();

	void IntervalTest::testLei2011AisGreaterEqualToB();

	void IntervalTest::testLex1AisGreaterThanB();

	void IntervalTest::testLex2AisGreaterThanB();
};

