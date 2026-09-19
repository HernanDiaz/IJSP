/*
 * JSPClassRegister.cpp
 *
 *  Created on: Jul 07, 2019
 *      Author: Hernan Diaz Rodriguez
 */

#include "JSPClassRegister.h"

namespace JSP {

/**
 * Initialize all static variables in SchedulingClassRegister
 */
	std::map<std::string, SGS_JSP*(*)()>	JSPClassRegister::SGSMap;
	

}
