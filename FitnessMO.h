/*
* FitnessMO.h
*
*  Created on: Jun 14, 2018
*      Author: Juan Jose Palacios
*/
#pragma once

#include "Fitness.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class FitnessLexicographic
//
//=============================================================================
/**
* This class defines a lexicographic fitness object, in which an array
* of different single fitness are stored and then compared foloowing
* a specific order. So the solution with the best value on the first
* fitness is considered the best. In case of a tie, the second fitness
* value is considered and so on. 
*
* @author Juan Jose Palacios
*
*/
class FitnessLexicographic : public Fitness {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	/*
	* Array with all fitness values
	*/
	std::vector<Fitness *> fitnessValues;
	




	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	FitnessLexicographic()
		: Fitness(), fitnessValues() { }


	/*
	* Copy consrtuctor
	*/
	FitnessLexicographic(const FitnessLexicographic &source);


	/*
	* Destructor
	*/
	virtual ~FitnessLexicographic();


	/*
	* Clone method for inherited instances
	*/
	virtual Fitness* clone() const {
		return new FitnessLexicographic(*this);
	}



	//=========================================================================
	//		GET / SET METHODS
	//=========================================================================
	/*
	* Adds a new objective function to the Lexicographical fitness
	*/
	virtual void addFitness(Fitness *fitness);

	/*
	* Modifies one of the fitness values
	*/
	virtual void setFitness(const unsigned int idx, Fitness *fitness);
	
	/*
	* Get the fitness value of a specific position
	*/
	virtual Fitness * getFitness(const unsigned int idx) const;

	/*
	* Get the number of objective functions considered
	*/
	virtual unsigned int getNumberFunctions() const {
		return this->fitnessValues.size();
	}


	/*
	* Gets the type of the Fitness to be identified
	*/
	virtual Fitness::Type getType() const {
		return Fitness::Type::LEXICOGRAPHIC;
	}

	/*
	* Converts the Fitness into a double value
	*/
	virtual double toDouble() const {
		return this->fitnessValues[0]->toDouble();
	}

	/*
	* Converts the Fitness into a string
	*/
	virtual std::string toString() const;



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	virtual bool isBetterOrEqualTo(const Fitness * f) const;
	virtual bool isBetterThan(const Fitness * f) const;
	virtual bool isEqualTo(const Fitness * f) const;
	virtual bool isWorseThan(const Fitness * f) const;
	virtual bool isWorseOrEqualTo(const Fitness * f) const;
};

}
