/*
 * Statistics.h
 *
 *  Created on: Sep 20, 2017
 *      Author: jjpalacios
 */
#ifndef SRC_DATA_PRINTER_H_
#define SRC_DATA_PRINTER_H_


#include "Population.h"


namespace FJSP {

#define FULL_POP "FullPopulation"

//=============================================================================
//
//	Class DataPrinter
//
//=============================================================================
/**
 * This class allows to print additional data on files. It it mainly used
 * for debugging purposes
 *
 * @author jjpalacios
 *
 */
struct DataPrinter {
protected:
	static int numCalls;
public:
	static void printPopulation(int number, std::string signature,
			std::string logFolder, Population *population);

};


}
#endif /* SRC_DATA_PRINTER_H_ */
