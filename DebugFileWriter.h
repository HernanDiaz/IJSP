/*
* SturdinessFileWriter.h
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "heading.h"
#include "FuzzyFWException.h"

namespace IJSP {
	// Name for sets of solutions
#define COL_SEPARATOR "\n"
	   
class DebugFileWriter {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:
	std::ofstream outputFile;

	DebugFileWriter();

	void open(std::string outputName);

	void write(double value);

	void write(std::string value);

	void endline();

	void close();
};
}
