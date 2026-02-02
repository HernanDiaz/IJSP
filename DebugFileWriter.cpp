/*
* SturdinessFileWriter.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "DebugFileWriter.h"




namespace IJSP {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	
	DebugFileWriter::DebugFileWriter() {
		
	}

	void DebugFileWriter::open(std::string outputName)
	{
		// Open output file for solutions
		std::string finalOutputName = outputName + "_.csv";
		outputFile.open(finalOutputName.c_str());

		if (!outputFile.is_open()) {
			std::string err = "It was impossible to generate the output ";
			err += "files. They may be opened or the logFolder does ";
			err += "not exist";
			throw new FuzzyFW::FuzzyFWException("Environment", err);
		}
	}

		void DebugFileWriter::close()
	{
		outputFile.close();
	}

	void DebugFileWriter::write(double value) {
		outputFile << value;
		outputFile << COL_SEPARATOR;
	}

	void DebugFileWriter::write(std::string value) {
		outputFile << value;
		outputFile << COL_SEPARATOR;
	}

	void DebugFileWriter::endline() {
		outputFile << std::endl;
	}
	
}