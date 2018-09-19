/*
 * FJSPClassRegister.cpp
 *
 *  Created on: May 17, 2017
 *      Author: Juan Jose Palacios
 */

#include "FJSPClassRegister.h"

namespace FJSP {

/**
 * Initialize all static variables in SchedulingClassRegister
 */
	std::map<std::string, SGS_FJSP*(*)()>	FJSPClassRegister::SGSMap;


}
