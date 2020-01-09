/*
* SturdinessFileWriter.h
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "heading.h"
#include "FuzzyFWException.h"

namespace Robustness {
	// Name for sets of solutions
#define STURDINESS_OUTPUT_SOLS "Robustness"
#define STURDINESS_COL_SEPARATOR ";"
	   
class RobustnessFileWriter {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:
	std::ofstream outputFile;

	RobustnessFileWriter();

	void open(std::string outputName);

	void writeHeader(int numIter);
	
	void write(double value);

	void write(std::string value);

	void endline();

	void close();
};
}
