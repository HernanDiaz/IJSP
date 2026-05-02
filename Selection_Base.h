/*
 * Selection_Base.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Population.h"

namespace FuzzyFW {

// Selection parameters defined in this header file
#define SELECTION_SIZE "selection.tournament-size"
#define SELECTION_ROULETTE "selection.roulette-interpolate"
#define	ELITE_SIZE	"elite.size" //Selection mechanism to select number of elements in elite

//=============================================================================
//
//	Abstract class Selection
//
//=============================================================================
/**
 * This class provides the framework to implement the selection methods
 * to be used by evolutionary algorithms
 *
 * Classes inheriting from this one have to implement the method "apply"
 *
 * It will be able both to generate a new population by selecting individuals
 * from a given one, or just select one individual in the population.
 *
 *
 */
class Selection {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit Selection(ParameterDB *parameters=NULL);

	/**
	 * Loads the needed parameters. No default parameters needed
	 */
	virtual void setup(ParameterDB *parameters) { };

	/**
	 * Destructor
	 */
	virtual ~Selection() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Apply the selection method to choose an individual from the population
	 *
	 * @param population Population from which select an individual
	 * @param svars Shared elements of the algorithm
	 * @return The selected individual
	 */
	virtual Individual * select(Population *population,
			const SharedVars *svars) const=0;

	/**
	 * Apply the selection method to the entire population to generate
	 * a brand new one.
	 *
	 * @param population Original population of individuals
	 * @param svars Shared elements of the algorithm
	 * @return A new population with the selected individuals
	 */
	virtual Population * apply(Population *population, const unsigned int n,
			const SharedVars *svars) const;

	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const = 0;
};

} // namespace FuzzyFW
