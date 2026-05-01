/*
* TimeWindowClassRegister.cpp
*
*  Created on: May 17, 2017
*/

#include "TimeWindowClassRegister.h"

namespace FuzzyFW {

	/**
	* Initialize all static variables in SchedulingClassRegister
	*/
	std::map<std::string, TimeWindow*(*)()>	TimeWindowClassRegister::TimeWindowMap;


}
