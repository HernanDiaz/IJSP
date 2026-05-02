/*
 * Selection_SUS.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_Roulette.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionSUS
//
//=============================================================================
/**
 * This class implements the SUS selection. We use the variant with
 * linear interpolation. This way we avoid issues with the scale of the
 * objective function and also when minimizing the fitness value.
 * The population is sorted according to the quality of the individuals and
 * each individual is assigned an integer value increasingly, so the best
 * individual takes value 1, and the worst one takes value "size of the population".
 *
 * [J.E. Baker, "Reducing Bias and Inefficiency in the Selection Algorithm",
 * in Genetic Algorithms and their Applications: Proc. of the 2nd Int. Conf on
 * Genetic Algorithms, pp. 14-21, 1987]
 *
 *
 */
class SelectionSUS : public SelectionRoulette {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit SelectionSUS(ParameterDB *parameters=NULL)
		: SelectionRoulette(parameters) { }

	/**
	* Loads the needed parameters: None in this case
	*/
	virtual void setup(ParameterDB *parameters) {
		SelectionRoulette::setup(parameters);
	}

	/**
	 * Destructor
	 */
	virtual ~SelectionSUS() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Apply the selection method to the entire population to generate
	 * a brand new one following the SUS algorithm.
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
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("SUS");
		if (this->interpolate)
			setup.push_back(";Interpolate:;Yes");
		else
			setup.push_back(";Interpolate:;No");
		return setup;
	}
};

} // namespace FuzzyFW
