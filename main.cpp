/*
 * MetaFJSP.cpp
 *
 *  Created on: Sep 22, 2017
 *      Author: jjpalacios
 */
#include <iostream>
using namespace std;


#include "EvoLauncher.h"
#include "ProblemIJSP.h"
#include "ProblemIJSP.h"
//#include "mainTest.cpp"

int main(int argc, const char *argv[]) {
	FuzzyFW::EvoLauncher *launcher;
	IJSP::ProblemIJSP *problem;

	if(argc < 3) {
		std::cout << "ERROR: Not enough input arguments." << std::endl;
		std::cout << "\t Use: MetaIJSP <setup_file> <problem_file> [log_folder]" << std::endl;
		return 0;
	}

	try {
		launcher = new FuzzyFW::EvoLauncher(argv[1]);
		if(argc > 3)
			launcher->setLogFolder(argv[3]);
		/*
		if (launcher->isInTestMode()) {
			mainTest::testMain();
			system("PAUSE");
			return 0;
		}*/

		problem = new IJSP::ProblemIJSP(argv[2]);
		launcher->optimise(problem);

	} catch(IJSP::IJSPException &ex) {
		std::cout << ex.what() << std::endl << std::endl;
		//system("PAUSE");
		return -2;
	}

	delete launcher;

	//system("PAUSE");
	return 0;
}

