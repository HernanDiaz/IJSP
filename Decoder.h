/*
 * Decoder.h
 *
 *  Created on: September 12, 2017
 *      Author: jjpalacios
 */
#pragma once

#include "Encoder.h"


namespace FuzzyFW {


//=============================================================================
//
//	Abstract class Decoder
//
//=============================================================================
/**
 * This class creates a method apply that receives a genotype and generates
 * a solution from it
 *
 * @author jjpalacios
 *
 */
class Decoder {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit Decoder(ParameterDB *parameters = NULL) {
		if (parameters != NULL)
			this->setup(parameters);
	}

	/**
	* Copy constructor
	*/
	Decoder(const Decoder &source) { }

	/**
	 * Loads the needed parameters.
	 * Loads the crossover probability
	 */
	virtual void setup(ParameterDB *parameters) { }

	/**
	* Clone method, in case of inheritance
	*/
	virtual Decoder * clone() const = 0;

	/**
	 * Destructor
	 */
	virtual ~Decoder() { } 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Apply the decodification to the genotype to get a full solution to the
	 * problem
	 */
	virtual Solution * decode(Individual * indiv,
		const SharedVarsEvolutionary * const svars) = 0;

	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const=0;
};

}
