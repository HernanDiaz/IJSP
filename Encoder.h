/*
 * Encoder.h
 *
 *  Created on: September 13, 2017
 */
#pragma once

#include "Individual.h"
#include "SharedVarsEvolutionary.h"

namespace FuzzyFW {


//=============================================================================
//
//	Abstract class Encoder
//
//=============================================================================
/**
	* This class is responsible for encoding a solution into the chromosome
	* of an individual. It's the complementary of the decoder
	*
	*
	*/
class Encoder {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
		* Default constructor
		*/
	explicit Encoder(ParameterDB *parameters = NULL) {
		if (parameters != NULL)
			this->setup(parameters);
	}

	/**
	* Copy constructor
	*/
	Encoder(const Encoder &source) { }

	/**
		* Loads the needed parameters. None in this case
		*/
	virtual void setup(ParameterDB *parameters) { }

	/**
	* Clone method, in case of inheritance
	*/
	virtual Encoder * clone() const = 0;

	/**
		* Destructor
		*/
	virtual ~Encoder() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
		* Encodes the solution into the individual's genotpye
		*/
	virtual void encode(Solution *solution, Individual *indiv,
		const SharedVarsEvolutionary * const svars) const = 0;

	/**
	* Encodes the solution into a new individual
	*/
	virtual Individual * encode(Solution *solution,
		const SharedVarsEvolutionary * const svars) const = 0;

	/**
		* Get the name and setup of the encoding method
		*/
	virtual std::vector<std::string> getName() const = 0;
};

}
