/*
 * MetaJSP.cpp
 *
 *  Created on: Sep 22, 2017
 */
#include <iostream>
using namespace std;


#include "EvoLauncher.h"
#include "ProblemJSP.h"
#include "ProblemJSP.h"
//#include "mainTest.cpp"

int main(int argc, const char *argv[]) {
	FuzzyFW::EvoLauncher *launcher;
	JSP::ProblemJSP *problem;

	if(argc < 3) {
		std::cout << "ERROR: Not enough input arguments." << std::endl;
		std::cout << "\t Use: MetaJSP <setup_file> <problem_file> [log_folder]" << std::endl;
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

		problem = new JSP::ProblemJSP(argv[2]);
		launcher->optimise(problem);

	} catch(JSP::JSPException &ex) {
		std::cout << "JSP Error: " << ex.what() << std::endl << std::endl;
		return -2;
	} catch(FuzzyFW::FuzzyFWException &ex) {
		std::cout << "FuzzyFW Error: " << ex.what() << std::endl << std::endl;
		return -3;
	} catch(std::exception &ex) {
		std::cout << "Error: " << ex.what() << std::endl << std::endl;
		return -4;
	}

	delete launcher;

	//system("PAUSE");
	return 0;
}

