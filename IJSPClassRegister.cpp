/*
 * IJSPClassRegister.cpp
 *
 *  Created on: Jul 07, 2019
 *      Author: Hernan Diaz Rodriguez
 */

#include "IJSPClassRegister.h"

namespace IJSP {

/**
 * Initialize all static variables in SchedulingClassRegister
 */
	std::map<std::string, SGS_IJSP*(*)()>	IJSPClassRegister::SGSMap;


}
