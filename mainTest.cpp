/*
* IntervalTest.cpp
*
*  Created on: July 29, 2019
*      Author: Hernan Diaz Rodriguez
*/

#include "IntervalTest.h"

class mainTest{
public:
	static void testMain()
	{
		CppUnit::TextUi::TestRunner run;
		run.addTest(IntervalTest::suite());
		run.run();
	}
};




