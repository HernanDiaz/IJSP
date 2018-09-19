/*
 * Statistics.h
 *
 *  Created on: Sep 20, 2017
 *      Author: jjpalacios
 */

#ifndef SRC_ECOBJECTS_STATISTICS_H_
#define SRC_ECOBJECTS_STATISTICS_H_


#include "heading.h"
#include "Population.h"


namespace FJSP {

#define FULL_POP "FullPopulation"

//=============================================================================
//
//	Class Statistics
//
//=============================================================================
/**
 * This class allows to print additional data on files. It it mainly used
 * for debugging purposes
 *
 * @author jjpalacios
 *
 */
struct Statistics {
protected:
	static int numCalls;
public:
	static void printPopulation(int number, std::string signature,
			std::string logFolder, Population *population);

};


}
#endif /* SRC_ECOBJECTS_STATISTICS_H_ */
