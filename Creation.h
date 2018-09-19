/*
 * Creation.h
 *
 *  Created on: Aug 4, 2017
 *      Author: jjpalacios
 */
#pragma once

#include "Population.h"
#include "SharedVarsEvolutionary.h"

namespace FuzzyFW {

//=============================================================================
//
//	Abstract class Creation
//
//=============================================================================
/**
 * This class provides the framework to implement different strategies
 * to create initial solutions to a problem, and therefore populations
 *
 * @author jjpalacios
 *
 */
class Creation {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit Creation(ParameterDB *parameters = NULL) {
		if (parameters != NULL)
			this->setup(parameters);
	}

	/**
	* Copy constructor
	*/
	Creation(const Creation &source) { }

	/**
	 * Loads the needed parameters: No parameters needed
	 */
	virtual void setup(ParameterDB *parameters) { }

	/**
	* Clone method for inheriting classes
	*/
	virtual Creation * clone() const = 0;

	/**
	 * Destructor
	 */
	virtual ~Creation() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Creates an initial population using the given individual type and the
	 * size of the population
	 *
	 * @param individualType Type of individuals to create
	 * @param size Number of individuals to create
	 * @param svars Shared variables for the algorithm
	 * @return A bunch of filthy individuals
	 */
	virtual Population * createPopulation(const unsigned int popSize,
		const SharedVarsEvolutionary *svars) const;

	/**
	 * Creates an individual
	 *
	 * @param individualType Type of individuals to create
	 * @param svars Shared variables for the algorithm
	 * @return A new born individual
	 */
	virtual Individual * createIndividual(const SharedVarsEvolutionary *svars) const=0;

	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const=0;

};

}
