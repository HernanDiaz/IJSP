/*
 * Selection.h
 *
 *  Created on: Aug 2, 2017
 *      Author: jjpalacios
 */
#pragma once


#include "Population.h"


namespace FuzzyFW {

// Selection parameters defined in this header file
#define SELECTION_SIZE "selection.tournament-size"
#define SELECTION_ROULETTE "selection.roulette-interpolate"



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
 * @author jjpalacios
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
 * @author jjpalacios
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
 * @author jjpalacios
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
 * [J.E. Baker, “Reducing Bias and Inefficiency in the Selection Algorithm”,
 * in Genetic Algorithms and their Applications: Proc. of the 2nd Int. Conf on
 * Genetic Algorithms, pp. 14-21, 1987]
 *
 * @author jjpalacios
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





//=============================================================================
//
//	Class SelectionShuffle
//
//=============================================================================
/**
 * This class implements the shuffle selection. In this method all the
 * individuals from the original population are selected, but changing their
 * order so they are randomly sorted in the following population
 *
 * @author jjpalacios
 *
 */
class SelectionShuffle : public Selection {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit SelectionShuffle(ParameterDB *parameters=NULL)
		: Selection(parameters) { }


	/**
	* Loads the needed parameters: None in this case
	*/
	virtual void setup(ParameterDB *parameters) {
		Selection::setup(parameters);
	}

	/**
	 * Destructor
	 */
	virtual ~SelectionShuffle() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * This strategy is designed to create new populations, thus this
	 * method does not make sense here. Ii will return a random individual
	 *
	 * @param population Population from which select an individual
	 * @param svars Shared elements of the algorithm
	 * @return The selected individual
	 */
	virtual Individual * select(Population *population,
			const SharedVars *svars) const;

	/**
	 * Apply the selection method to the entire population to generate
	 * a brand new one by randomly shuffle the individuals
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
		setup.push_back("Shuffle");
		return setup;
	}
};





//=============================================================================
//
//	Class SelectionRandom
//
//=============================================================================
/**
 * This class implements a random selection. It just chooses one individual
 * at random
 *
 * @author jjpalacios
 *
 */
class SelectionRandom : public Selection {
public:
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit SelectionRandom(ParameterDB *parameters=NULL)
		: Selection(parameters) { }

	/**
	 * Destructor
	 */
	virtual ~SelectionRandom() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Choose one individual at random
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
		setup.push_back("Random");
		return setup;
	}
};


}

