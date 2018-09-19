/*
 * Statistics.cpp
 *
 *  Created on: Sep 20, 2017
 *      Author: jjpalacios
 */

#include "Statistics.h"

namespace FJSP {

int Statistics::numCalls = 0;

void Statistics::printPopulation(int number, std::string signature,
		std::string logFolder, Population *population) {
	std::ofstream outputFile;

	std::string fileName;
	fileName = logFolder + "/" + FULL_POP + "_" + signature + ".csv";

	if(numCalls == 0)
		outputFile.open(fileName.c_str());
	else
		outputFile.open(fileName.c_str(), std::ofstream::app);
	outputFile << "----------------------------------------------------------";
	outputFile << std::endl << "Iteration " << number << std::endl;
	outputFile << "----------------------------------------------------------";
	outputFile << std::endl;
	population->printCSV(outputFile);
	outputFile << std::endl;
	outputFile.close();
	Statistics::numCalls++;
}

}
