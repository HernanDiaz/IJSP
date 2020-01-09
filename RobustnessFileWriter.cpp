/*
* SturdinessFileWriter.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "RobustnessFileWriter.h"




namespace Robustness {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	
	RobustnessFileWriter::RobustnessFileWriter() {
		
	}

	void RobustnessFileWriter::open(std::string outputName)
	{
		// Open output file for solutions
		std::string finalOutputName = outputName + "_" + STURDINESS_OUTPUT_SOLS + ".csv";
		outputFile.open(finalOutputName.c_str());
		if (!outputFile.is_open()) {
			std::string err = "It was impossible to generate the output ";
			err += "files. They may be opened or the logFolder does ";
			err += "not exist";
			throw new FuzzyFW::FuzzyFWException("Environment", err);
		}
	}
	
	void RobustnessFileWriter::close()
	{
		outputFile.close();
	}

	void RobustnessFileWriter::writeHeader(int numIter) {
		outputFile << "Run\Simulation;";
		for (int i = 0; i < numIter; i++) {
			outputFile << "S";
			outputFile <<(i+1);
			outputFile << STURDINESS_COL_SEPARATOR; 
		}
		outputFile << "Avg";
		outputFile << STURDINESS_COL_SEPARATOR;
		this->endline();
	}

	void RobustnessFileWriter::write(double value) {
		outputFile << value;
		outputFile << STURDINESS_COL_SEPARATOR;
	}

	void RobustnessFileWriter::write(std::string value) {
		outputFile << value;
		outputFile << STURDINESS_COL_SEPARATOR;
	}

	void RobustnessFileWriter::endline() {
		outputFile << std::endl;
	}
	
}