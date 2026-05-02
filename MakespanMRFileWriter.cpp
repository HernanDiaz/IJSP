/*
* SturdinessFileWriter.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "MakespanMRFileWriter.h"




namespace PostExecution {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	
	MakespanMRFileWriter::MakespanMRFileWriter() {
		
	}

	void MakespanMRFileWriter::open(std::string outputName)
	{
		// Open output file for solutions
		std::string finalOutputName = outputName + "_" + TARDINESS_OUTPUT_SOLS + ".csv";
		outputFile.open(finalOutputName.c_str());

		if (!outputFile.is_open()) {
			std::string err = "It was impossible to generate the output ";
			err += "files. They may be opened or the logFolder does ";
			err += "not exist";
			throw FuzzyFW::FuzzyFWException("Environment", err);
		}
	}
	
	void MakespanMRFileWriter::close()
	{
		outputFile.close();
	}

	void MakespanMRFileWriter::writeHeader(int numIter) {
		outputFile << "Run\Simulation;";
		for (int i = 0; i < numIter; i++) {
			outputFile << "S";
			outputFile <<(i+1);
			outputFile << TARDINESS_COL_SEPARATOR;
		}
		outputFile << "Avg";
		outputFile << TARDINESS_COL_SEPARATOR;
		this->endline();
	}

	void MakespanMRFileWriter::write(double value) {
		outputFile << value;
		outputFile << TARDINESS_COL_SEPARATOR;
	}

	void MakespanMRFileWriter::write(std::string value) {
		outputFile << value;
		outputFile << TARDINESS_COL_SEPARATOR;
	}

	void MakespanMRFileWriter::endline() {
		outputFile << std::endl;
	}
	
}