/*
 * SimulatedCooling.cpp
 *
 *  Created on: May 5, 2022
 *      Author: hdiaz
 */

#include "MonotonicAdaptativeCooling.h"

namespace FuzzyFW {

	//=============================================================================
	//
	//	Class Exponential_Multiplicative_Cooling
	//
	//=============================================================================
	//=============================================================================
	//		METHODS
	//=============================================================================
	//=====  getTemperature   ==================================================

	double Exponential_Multiplicative_Cooling::getTemperature(const int generation) const {
		return (this->t0)*std::pow(this->a, generation);
	}

	std::vector<std::string> Exponential_Multiplicative_Cooling::getName() const{
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Exponential_Multiplicative_Cooling");
		setup.push_back("a:");
		setup.push_back(valueToString(this->a));
		setup.push_back("t0:");
		setup.push_back(valueToString(this->t0));
		setup.push_back("tn:");
		setup.push_back("not used");
		return setup;
	}

	//=============================================================================
	//
	//	Class Logaritmic_Multiplicative_Cooling
	//
	//=============================================================================
	//=============================================================================
	//		METHODS 
	//=============================================================================
	//=====  getTemperature ==================================================

	double Logaritmic_Multiplicative_Cooling::getTemperature(const int generation) const {
		return (this->t0) / (1 + this->a * std::log(1 + generation));
	}

	std::vector<std::string> Logaritmic_Multiplicative_Cooling::getName() const {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Logaritmic_Multiplicative_Cooling");
		setup.push_back("a:");
		setup.push_back(valueToString(this->a));
		setup.push_back("t0:");
		setup.push_back(valueToString(this->t0));
		setup.push_back("tn:");
		setup.push_back("not used");
		return setup;
	}

	//=============================================================================
	//
	//	Class Linear_Cooling
	//
	//=============================================================================
	//=============================================================================
	//		METHODS 
	//=============================================================================
	//=====  getTemperature ==================================================

	double Linear_Cooling::getTemperature(const int generation) const {
		if (generation >= this->t0) return 0;
		return (this->t0 - generation) / this->t0;
	}

	std::vector<std::string> Linear_Cooling::getName() const {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Linear_Cooling");
		setup.push_back("a:");
		setup.push_back(valueToString(this->a));
		setup.push_back("t0:");
		setup.push_back(valueToString(this->t0));
		setup.push_back("tn:");
		setup.push_back("not used");
		return setup;
	}

	//=============================================================================
//
//	Class Linear_Multiplicative_Cooling
//
//=============================================================================
//=============================================================================
//		METHODS 
//=============================================================================
//=====  getTemperature ==================================================

	double Linear_Multiplicative_Cooling::getTemperature(const int generation) const {
		return this->t0 / (1 + this->a*generation);
	}

	std::vector<std::string> Linear_Multiplicative_Cooling::getName() const {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Linear_Multiplicative_Cooling");
		setup.push_back("a:");
		setup.push_back(valueToString(this->a));
		setup.push_back("t0:");
		setup.push_back(valueToString(this->t0));
		setup.push_back("tn:");
		setup.push_back("not used");
		return setup;
	}

	//=============================================================================
//
//	Class Quadratic_Multiplicative_Cooling
//
//=============================================================================
//=============================================================================
//		METHODS 
//=============================================================================
//=====  getTemperature ==================================================

	double Quadratic_Multiplicative_Cooling::getTemperature(const int generation) const {
		return this->t0 / (1 + this->a*generation*generation);
	}

	std::vector<std::string> Quadratic_Multiplicative_Cooling::getName() const {
		std::vector<std::string> setup;
		setup.push_back("Quadratic_Multiplicative_Cooling");
		setup.push_back("a:");
		setup.push_back(valueToString(this->a));
		setup.push_back("t0:");
		setup.push_back(valueToString(this->t0));
		setup.push_back("tn:");
		setup.push_back("not used");
		return setup;
	}

	//=============================================================================
//
//	Class Linear_Additive_Cooling
//
//=============================================================================
//=============================================================================
//		METHODS 
//=============================================================================
//=====  getTemperature ==================================================

	double Linear_Additive_Cooling::getTemperature(const int generation) const {
		return this->tn + (this->t0 - this->tn)*(this->a - generation) / this->a;
	}

	std::vector<std::string> Linear_Additive_Cooling::getName() const {
		std::vector<std::string> setup;
		setup.push_back("Linear_Additive_Cooling");
		setup.push_back("a:");
		setup.push_back(valueToString(this->a));
		setup.push_back("t0:");
		setup.push_back(valueToString(this->t0));
		setup.push_back("tn:");
		setup.push_back(valueToString(this->tn));
		return setup;
	}



	//=============================================================================
	//
	//	Class Quadratic_Additive_Cooling
	//
	//=============================================================================
	//=============================================================================
	//		METHODS 
	//=============================================================================
	//=====  getTemperature ==================================================

	double Quadratic_Additive_Cooling::getTemperature(const int generation) const {
		return this->tn + (this->t0 - this->tn)*((this->a - generation) / this->a)*((this->a - generation) / this->a);
	}
	std::vector<std::string> Quadratic_Additive_Cooling::getName() const {
		std::vector<std::string> setup;
		setup.push_back("Quadratic_Additive_Cooling");
		setup.push_back("a:");
		setup.push_back(valueToString(this->a));
		setup.push_back("t0:");
		setup.push_back(valueToString(this->t0));
		setup.push_back("tn:");
		setup.push_back(valueToString(this->tn));
		return setup;
	}


	//=============================================================================
//
//	Class Trigonometric_Additive_Cooling
//
//=============================================================================
//=============================================================================
//		METHODS 
//=============================================================================
//=====  getTemperature ==================================================

	double Trigonometric_Additive_Cooling::getTemperature(const int generation) const {
		return this->tn + (this->t0 - this->tn)*(1+std::cos(generation*M_PI/this->a))/2;
	}

	std::vector<std::string> Trigonometric_Additive_Cooling::getName() const {
		std::vector<std::string> setup;
		setup.push_back("Trigonometric_Additive_Cooling");
		setup.push_back("a:");
		setup.push_back(valueToString(this->a));
		setup.push_back("t0:");
		setup.push_back(valueToString(this->t0));
		setup.push_back("tn:");
		setup.push_back(valueToString(this->tn));
		return setup;
	}
}