/*
 * MetaFJSP.cpp
 *
 *  Created on: Sep 22, 2017
 *      Author: jjpalacios
 */

#include "EvoLauncher.h"

// Test variables (for debugging)
#define DEBUG 1

int main(int argc, const char *argv[]) {
	FJSP::EvoLauncher *launcher;

	if(argc < 3) {
		std::cout << "ERROR: Not enough input arguments." << std::endl;
		std::cout << "\t Use: MetaFJSP <setup_file> <problem_file> [log_folder]" << std::endl;
		return 0;
	}

	try {
		launcher = new FJSP::EvoLauncher(argv[2], argv[1]);
		if(argc > 3)
			launcher->setLogFolder(argv[3]);

		launcher->optimise();

	} catch(FJSP::FJSPException &ex) {
		std::cout << ex.what() << std::endl << std::endl;
		return -2;
	}

	delete launcher;

	return 0;
}

