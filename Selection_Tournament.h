/*
 * Selection_Tournament.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_Base.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionTournament
//
//=============================================================================
/**
 * This class implements the tournament selection.
 *
 * From a given population, select K individuals at random and then takes the
 * best one.
 *
 *
 */
class SelectionTournament : public Selection {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	/**
	 * Name of the parameter for the size of the tournament
	 */
	std::string sizeLabel;

	/**
	 * Size of the tournament
	 */
	unsigned int size;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit SelectionTournament(ParameterDB *parameters = NULL)
		: Selection(parameters), sizeLabel(SELECTION_SIZE), size(1) { }

	/**
	 * Loads the needed parameters: Tournament size
	 */
	virtual void setup(ParameterDB *parameters);

	/**
	 * Destructor
	 */
	virtual ~SelectionTournament() { }; 	// Nothing to destroy here



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
		setup.push_back("Tournament");
		setup.push_back(";Size;" + valueToString(this->size));
		return setup;
	}
};

} // namespace FuzzyFW
