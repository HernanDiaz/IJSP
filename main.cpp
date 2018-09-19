/*
 * MetaFJSP.cpp
 *
 *  Created on: Sep 22, 2017
 *      Author: jjpalacios
 */

#include "EvoLauncher.h"


int main(int argc, const char *argv[]) {
	FuzzyFW::EvoLauncher *launcher;
	FJSP::ProblemFJSP *problem;

	if(argc < 3) {
		std::cout << "ERROR: Not enough input arguments." << std::endl;
		std::cout << "\t Use: MetaFJSP <setup_file> <problem_file> [log_folder]" << std::endl;
		return 0;
	}

	try {
		launcher = new FuzzyFW::EvoLauncher(argv[1]);
		if(argc > 3)
			launcher->setLogFolder(argv[3]);

		problem = new FJSP::ProblemFJSP(argv[2]);
		launcher->optimise(problem);

	} catch(FJSP::FJSPException &ex) {
		std::cout << ex.what() << std::endl << std::endl;
		system("PAUSE");
		return -2;
	}

	delete launcher;

	system("PAUSE");
	return 0;
}

