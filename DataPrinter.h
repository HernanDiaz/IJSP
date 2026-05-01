/*
 * Statistics.h
 *
 *  Created on: Sep 20, 2017
 */
#pragma once


#include "Population.h"


namespace FuzzyFW {

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
