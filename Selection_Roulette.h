/*
 * Selection_Roulette.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_Base.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionRoulette
//
//=============================================================================
/**
 * This class implements the roulette selection. We use the variant with
 * linear interpolation. This way we avoid issues with the scale of the
 * objective function and also when minimizing the fitness value.
 * The population is sorted according to the quality of the individuals and
 * each individual is assigned an integer value increasingly, so the best
 * individual takes value 1, and the worst one takes value "size of the population".
 *
 *
 */
class SelectionRoulette : public Selection {
protected:
	//=========================================================================
	//		FIELDS
	//=========================================================================
	/**
	* Name of the parameter for the size of the tournament
	*/
	std::string interpolateLabel;

	/**
	* Size of the tournament
	*/
	char interpolate;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit SelectionRoulette(ParameterDB *parameters=NULL)
		: interpolateLabel(SELECTION_ROULETTE), interpolate(true),
		Selection(parameters) { }

	/**
	* Loads the needed parameters: Interpolation
	*/
	virtual void setup(ParameterDB *parameters);

	/**
	 * Destructor
	 */
	virtual ~SelectionRoulette() { }; 	// Nothing to destroy here



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
			const SharedVars *svars) const;


	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Roulette");
		if (this->interpolate)
			setup.push_back(";Interpolate:;Yes");
		else
			setup.push_back(";Interpolate:;No");
		return setup;
	}

protected:
	/**
	* Apply the selection method to choose an individual from the population
	* by using a linear interpolation
	*
	* @param population Population from which select an individual
	* @param svars Shared elements of the algorithm
	* @return The selected individual
	*/
	virtual Individual * selectInterpolate(Population *population,
		const SharedVars *svars) const;

	/**
	* Apply the selection method to choose an individual from the population
	* by using the tradiotional method
	*
	* @param population Population from which select an individual
	* @param svars Shared elements of the algorithm
	* @return The selected individual
	*/
	virtual Individual * selectClassic(Population *population,
		const SharedVars *svars) const;
};

} // namespace FuzzyFW
