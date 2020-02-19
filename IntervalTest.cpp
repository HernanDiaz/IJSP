/*
* IntervalTest.cpp
*
*  Created on: July 29, 2019
*      Author: Hernan Diaz Rodriguez
*/

//#include "IntervalTest.h"



#include "IntervalTest.h"

void IntervalTest::setUp() {}

void  IntervalTest::tearDown() {}

void  IntervalTest::testLei2011FormulaCase1() {
	//Al >= Br
	FuzzyFW::Interval a = FuzzyFW::Interval(5,10);
	FuzzyFW::Interval b = FuzzyFW::Interval(1,5);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, a.Lei2011Formula(b), tolerance);
}

void  IntervalTest::testLei2011FormulaCase2() {
	//Bl <= Al < Br <= Ar
	FuzzyFW::Interval a = FuzzyFW::Interval(5, 10);
	FuzzyFW::Interval b = FuzzyFW::Interval(1, 7);
	FuzzyFW::Interval c = FuzzyFW::Interval(5, 10);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0 / 15.0, a.Lei2011Formula(b), tolerance);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, a.Lei2011Formula(c), tolerance);
}

void IntervalTest::testLei2011FormulaCase3() {
	//Al < Bl < Br <= Ar
	FuzzyFW::Interval a = FuzzyFW::Interval(5, 10);
	FuzzyFW::Interval b = FuzzyFW::Interval(6, 10);
	FuzzyFW::Interval c = FuzzyFW::Interval(6, 8);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0/5.0, a.Lei2011Formula(b), tolerance);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.4, a.Lei2011Formula(c), tolerance);
}

void IntervalTest::testLei2011FormulaCase4() {
	//Al < Bl <= Ar < Br
	FuzzyFW::Interval a = FuzzyFW::Interval(5, 10);
	FuzzyFW::Interval b = FuzzyFW::Interval(6, 12);
	FuzzyFW::Interval c = FuzzyFW::Interval(10, 12);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0/15.0, a.Lei2011Formula(b), tolerance);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, a.Lei2011Formula(c), tolerance);
}


void IntervalTest::testLei2011FormulaCase5() {
	//Ar <= Bl
	FuzzyFW::Interval a = FuzzyFW::Interval(5, 10);
	FuzzyFW::Interval b = FuzzyFW::Interval(4, 12);
	FuzzyFW::Interval c = FuzzyFW::Interval(5, 14);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(9.0/16.0, a.Lei2011Formula(b), tolerance);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0/18.0, a.Lei2011Formula(c), tolerance);
}

void IntervalTest::testLei2011AEqualsA() {
	FuzzyFW::Interval a = FuzzyFW::Interval(5, 10);
	CPPUNIT_ASSERT(a.isEqualTo(a, FuzzyFW::Interval::C_JIANG));
}
	

void IntervalTest::testLei2011AisGreaterEqualToB() {
	FuzzyFW::Interval a = FuzzyFW::Interval(5, 10);
	FuzzyFW::Interval b = FuzzyFW::Interval(1, 4);
	CPPUNIT_ASSERT(a.isGreaterEqualTo(b, FuzzyFW::Interval::C_JIANG));
	a = FuzzyFW::Interval(7, 16);
	b = FuzzyFW::Interval(8, 13);
	CPPUNIT_ASSERT(a.isGreaterEqualTo(b, FuzzyFW::Interval::C_JIANG));
}

void IntervalTest::testLex1AisGreaterThanB() {
	FuzzyFW::Interval a = FuzzyFW::Interval(3, 7);
	FuzzyFW::Interval b = FuzzyFW::Interval(1, 5);
	CPPUNIT_ASSERT(a.isGreaterThan(b, FuzzyFW::Interval::C_LEX1));
	a = FuzzyFW::Interval(1, 5);
	b = FuzzyFW::Interval(1, 3);
	CPPUNIT_ASSERT(a.isGreaterThan(b, FuzzyFW::Interval::C_LEX1));
	a = FuzzyFW::Interval(1, 5);
	b = FuzzyFW::Interval(1, 5);
	CPPUNIT_ASSERT(!a.isGreaterThan(b, FuzzyFW::Interval::C_LEX1));
	a = FuzzyFW::Interval(1, 5);
	b = FuzzyFW::Interval(1, 6);
	CPPUNIT_ASSERT(!a.isGreaterThan(b, FuzzyFW::Interval::C_LEX1));
}

void IntervalTest::testLex2AisGreaterThanB() {
	FuzzyFW::Interval a = FuzzyFW::Interval(3, 7);
	FuzzyFW::Interval b = FuzzyFW::Interval(1, 5);
	CPPUNIT_ASSERT(a.isGreaterThan(b, FuzzyFW::Interval::C_LEX2));
	a = FuzzyFW::Interval(1, 5);
	b = FuzzyFW::Interval(1, 3);
	CPPUNIT_ASSERT(a.isGreaterThan(b, FuzzyFW::Interval::C_LEX2));
  	a = FuzzyFW::Interval(1, 5);
	b = FuzzyFW::Interval(1, 5);
	CPPUNIT_ASSERT(!a.isGreaterThan(b, FuzzyFW::Interval::C_LEX2));
	a = FuzzyFW::Interval(1, 5);
	b = FuzzyFW::Interval(1, 6);
	CPPUNIT_ASSERT(!a.isGreaterThan(b, FuzzyFW::Interval::C_LEX2));
	a = FuzzyFW::Interval(2, 6);
	b = FuzzyFW::Interval(1, 6);
	CPPUNIT_ASSERT(a.isGreaterThan(b, FuzzyFW::Interval::C_LEX2));
}