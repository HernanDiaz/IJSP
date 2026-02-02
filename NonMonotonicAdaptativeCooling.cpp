/*
 * SimulatedCooling.cpp
 *
 *  Created on: May 5, 2022
 *      Author: hdiaz
 */

#include "NonMonotonicAdaptativeCooling.h"

namespace FuzzyFW {

	//=============================================================================
	//
	//	Class Class Adaptive_Non_Mon_Cooling
	//
	//=============================================================================
	//=============================================================================
	//		METHODS
	//=============================================================================
	//=====  getAdaptativeFactor 1+ (f*-f(SI))/f*   ==================================================

	double Adaptive_Non_Mon_Cooling::getAdaptativeFactor(const FuzzyFW::Individual* origin,	const FuzzyFW::Individual* destiny) const {
		return 1 + (origin->getFitness()->toDouble() - destiny->getFitness()->toDouble()) / origin->getFitness()->toDouble();
	}

	std::vector<std::string> Adaptive_Non_Mon_Cooling::getName() const {
		std::vector<std::string> setup;
		setup.push_back("Adaptive_Non_Mon_Cooling");
		if (this->monotonic != NULL) {
			std::vector<std::string> setupmon = this->monotonic->getName();
			setup.insert(setup.end(), setupmon.begin(), setupmon.end());
		}
		return setup;
	}

	//=============================================================================
//
//	Class Class Adaptive_Quadratic_Non_Mon_Cooling
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  getAdaptativeFactor 1+ (f*-f(SI))/f*   ==================================================

	double Adaptive_Quadratic_Non_Mon_Cooling::getAdaptativeFactor(const FuzzyFW::Individual* origin, const FuzzyFW::Individual* destiny) const {
		return (1 + (origin->getFitness()->toDouble() - destiny->getFitness()->toDouble()) / origin->getFitness()->toDouble())*(1 + (origin->getFitness()->toDouble() - destiny->getFitness()->toDouble()) / origin->getFitness()->toDouble());
	}

	std::vector<std::string> Adaptive_Quadratic_Non_Mon_Cooling::getName() const {
		std::vector<std::string> setup;
		setup.push_back("Adaptive_Quadratic_Non_Mon_Cooling");
		if (this->monotonic != NULL) {
			std::vector<std::string> setupmon = this->monotonic->getName();
			setup.insert(setup.end(), setupmon.begin(), setupmon.end());
		}
		return setup;
	}


	//	Class Disabled_Non_Mon_Cooling
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  getAdaptativeFactor    ==================================================

	double Disabled_Non_Mon_Cooling::getAdaptativeFactor(const FuzzyFW::Individual* origin, const FuzzyFW::Individual* destiny) const {
		return 1;
	}

	std::vector<std::string> Disabled_Non_Mon_Cooling::getName() const {
		std::vector<std::string> setup;
		setup.push_back("Disabled_Non_Mon_Cooling");
		if (this->monotonic != NULL) {
			std::vector<std::string> setupmon = this->monotonic->getName();
			setup.insert(setup.end(), setupmon.begin(), setupmon.end());
		}
		return setup;
	}
	
}