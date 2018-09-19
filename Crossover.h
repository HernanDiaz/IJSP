/*
 * Crossover.h
 *
 *  Created on: July 14, 2017
 *      Author: jjpalacios
 */
#pragma once

#include "Population.h"
#include "SharedVarsEvolutionary.h"


namespace FuzzyFW {

// Crossover parameters defined in this header file
#define CROSSOVER_PROBABILITY "crossover.probability"





//=============================================================================
//
//	Abstract class Crossover
//
//=============================================================================
/**
 * This class provides the framework to implement crossover operators to
 * be used by evolutionary algorithms.
 *
 * Classes inheriting from this one have to implement the method "apply"
 *
 * The crossover operator will generate 2 offspring that will replace their
 * parents automatically
 *
 * @author jjpalacios
 *
 */
class Crossover {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
public:
	/**
	 * Name of the probability parameter to get from the configuration file
	 */
	std::string probLabel;

	/**
	 * Probability to apply the operator
	 */
	double probability;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit Crossover(ParameterDB *parameters = NULL);

	/**
	 * Loads the needed parameters.
	 * Loads the crossover probability
	 */
	virtual void setup(ParameterDB *parameters);

	/**
	 * Destructor
	 */
	virtual ~Crossover() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Apply the crossover operator to a pair of individuals and produces two
	 * offspring. The offspring will automatically replace their parents
	 *
	 * @param ind1 First parent for the mating
	 * @param ind2 Second parent for the mating
	 * @param svars Shared elements of the algorithm
	 */
	virtual void apply(Individual *ind1,
			Individual *ind2, const SharedVarsEvolutionary *svars) const=0;

	/**
	 * Apply the crossover operator to all pairs in the population. The
	 * offspring will replace their parents in the original population
	 *
	 * @param population Population of individuals to mate
	 * @param svars Shared elements of the algorithm
	 */
	virtual void apply(Population *population,
		const SharedVarsEvolutionary *svars) const;

	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const=0;
};

}
