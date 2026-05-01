/*
* Fitness.cpp
*
*  Created on: June 14, 2018
*/

#include "FitnessMO.h"

namespace FuzzyFW {


//=============================================================================
//
//	Class FitnessLexicographic
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Copy constructor  ====================================================
FitnessLexicographic::FitnessLexicographic(const FitnessLexicographic &source) 
: Fitness(source) {
	for (size_t i = 0; i < source.fitnessValues.size(); i++)
		this->fitnessValues.push_back(source.fitnessValues[i]->clone());
}



//=====  Destructor  ==========================================================
FitnessLexicographic::~FitnessLexicographic() {
	for (size_t i = 0; i < this->fitnessValues.size(); i++)
		delete this->fitnessValues[i];
}



//=========================================================================
//		GET / SET METHODS
//=========================================================================
//=====  add Value  =======================================================
void FitnessLexicographic::addFitness(Fitness *fitness) {
	this->fitnessValues.push_back(fitness);
}

//=====  set Value  =======================================================
void FitnessLexicographic::setFitness(const unsigned int idx, Fitness *fitness) {
	if (idx < 0 || idx > this->fitnessValues.size()) {
		std::string errorMsg = "Accessing invalid fitness value";
		throw new FuzzyFWException("FitnessMO", errorMsg);
	}
	delete this->fitnessValues[idx];
	this->fitnessValues[idx] = fitness;
}

//=====  get Value  =======================================================
Fitness * FitnessLexicographic::getFitness(const unsigned int idx) const {
	if (idx < 0 || idx > this->fitnessValues.size()) {
		std::string errorMsg = "Accessing invalid fitness value";
		throw new FuzzyFWException("FitnessMO", errorMsg);
	}
	return this->fitnessValues[idx];
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Better than or equal to  =============================================
bool FitnessLexicographic::isBetterOrEqualTo(const Fitness * f) const {
	const FitnessLexicographic *fl =
		dynamic_cast<const FitnessLexicographic *>(f);
	if(fl == NULL)
		throw new FuzzyFWException("FitnessMO",
			"Comparison of incompatible fitness values");

	if(this->getNumberFunctions() != fl->getNumberFunctions())
		throw new FuzzyFWException("FitnessMO",
			"The number of objective functions does not match");

	for (size_t i = 0; i < this->fitnessValues.size(); i++) {
		if (this->fitnessValues[i]->isBetterThan(fl->getFitness(i)))
			return true;
		if (this->fitnessValues[i]->isWorseThan(fl->getFitness(i)))
			return false;
	}
	return true;
}


//=====  Better than  =========================================================
bool FitnessLexicographic::isBetterThan(const Fitness * f) const {
	const FitnessLexicographic *fl =
		dynamic_cast<const FitnessLexicographic *>(f);
	if (fl == NULL)
		throw new FuzzyFWException("FitnessMO",
			"Comparison of incompatible fitness values");

	if (this->getNumberFunctions() != fl->getNumberFunctions())
		throw new FuzzyFWException("FitnessMO",
			"The number of objective functions does not match");

	for (size_t i = 0; i < this->fitnessValues.size(); i++) {
		if (this->fitnessValues[i]->isBetterThan(fl->getFitness(i)))
			return true;
		if (this->fitnessValues[i]->isWorseThan(fl->getFitness(i)))
			return false;
	}
	return false;
}


//=====  Equal to  ============================================================
bool FitnessLexicographic::isEqualTo(const Fitness * f) const {
	const FitnessLexicographic *fl =
		dynamic_cast<const FitnessLexicographic *>(f);
	if (fl == NULL)
		throw new FuzzyFWException("FitnessMO",
			"Comparison of incompatible fitness values");

	if (this->getNumberFunctions() != fl->getNumberFunctions())
		throw new FuzzyFWException("FitnessMO",
			"The number of objective functions does not match");

	for (size_t i = 0; i < this->fitnessValues.size(); i++) {
		if (!(this->fitnessValues[i]->isEqualTo(fl->getFitness(i))))
			return false;
	}
	return true;
}


//=====  Worse than  ==========================================================
bool FitnessLexicographic::isWorseThan(const Fitness * f) const {
	const FitnessLexicographic *fl =
		dynamic_cast<const FitnessLexicographic *>(f);
	if (fl == NULL)
		throw new FuzzyFWException("FitnessMO",
			"Comparison of incompatible fitness values");

	if (this->getNumberFunctions() != fl->getNumberFunctions())
		throw new FuzzyFWException("FitnessMO",
			"The number of objective functions does not match");

	for (size_t i = 0; i < this->fitnessValues.size(); i++) {
		if (this->fitnessValues[i]->isBetterThan(fl->getFitness(i)))
			return false;
		if (this->fitnessValues[i]->isWorseThan(fl->getFitness(i)))
			return true;
	}
	return false;
}


//=====  Worse than or equal to  ==============================================
bool FitnessLexicographic::isWorseOrEqualTo(const Fitness * f) const {
	const FitnessLexicographic *fl =
		dynamic_cast<const FitnessLexicographic *>(f);
	if (fl == NULL)
		throw new FuzzyFWException("FitnessMO",
			"Comparison of incompatible fitness values");

	if (this->getNumberFunctions() != fl->getNumberFunctions())
		throw new FuzzyFWException("FitnessMO",
			"The number of objective functions does not match");

	for (size_t i = 0; i < this->fitnessValues.size(); i++) {
		if (this->fitnessValues[i]->isBetterThan(fl->getFitness(i)))
			return false;
		if (this->fitnessValues[i]->isWorseThan(fl->getFitness(i)))
			return true;
	}
	return true;
}


//=====  conversion to string  ================================================
std::string FitnessLexicographic::toString() const {
	std::string strValue = "";
	if(this->fitnessValues.size() > 0)
		strValue += this->fitnessValues[0]->toString();
	for (size_t i = 1; i < this->fitnessValues.size(); i++)
		strValue += " | " + this->fitnessValues[i]->toString();
	return strValue;
}

}
