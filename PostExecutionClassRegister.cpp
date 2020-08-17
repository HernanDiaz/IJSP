/*
 * PostExecutionClassRegister.cpp
 *
 *  Created on: Jul 07, 2019
 *      Author: Hernan Diaz Rodriguez
 */

#include "PostExecutionClassRegister.h"

namespace PostExecution {

/**
 * Initialize all static variables
 */
	std::map<std::string, PostExecutionAnalyzer*(*)()>	PostExecutionClassRegister::SGSMap;
}
