/*
* SturdinessFileWriter.h
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "heading.h"
#include "FuzzyFWException.h"

namespace PostExecution {
	// Name for sets of solutions
#define TARDINESS_OUTPUT_SOLS "Makespan_MR"
#define TARDINESS_COL_SEPARATOR ";"
	   
class MakespanMRFileWriter {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:
	std::ofstream outputFile;

	MakespanMRFileWriter();

	void open(std::string outputName);

	void writeHeader(int numIter);
	
	void write(double value);

	void write(std::string value);

	void endline();

	void close();
};
}
