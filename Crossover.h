/*
 * Crossover.h
 *
 *  Created on: July 14, 2017
 */
#pragma once

#include "Population.h"
#include "SharedVarsEvolutionary.h"


namespace FuzzyFW {
	


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
 *
 */
class Crossover {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
public:



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
	 * Nothing to load here
	 */
	virtual void setup(ParameterDB *parameters) { }

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
	virtual void apply(Population *population, const double probability,
		const SharedVarsEvolutionary *svars) const;

	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const=0;

	/**
 * Clone operator for the case of inheritance
 */
	virtual Crossover* clone() const = 0;
};





//=============================================================================
//
//	Abstract class Crossover_OBC
//
//=============================================================================
/**
* Order-based crossover for permutations
*
*
*/
class Crossover_OBC : public Crossover {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit Crossover_OBC(ParameterDB *parameters = NULL)
		: Crossover(parameters) { }

	/**
	* Loads the needed parameters.
	* Loads the crossover probability
	*/
	//virtual void setup(ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~Crossover_OBC() { }; 	// Nothing to destroy here



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
		Individual *ind2, const SharedVarsEvolutionary *svars) const;


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("Order-Based");
		return name;
	}

	/**
* Clone operator for the case of inheritance
*/
	virtual Crossover* clone() const {
		return new Crossover_OBC();
	}
};





//=============================================================================
//
//	Abstract class Crossover_CBC
//
//=============================================================================
/**
* Cycle-based crossover for permutations
*
*
*/
class Crossover_CBC : public Crossover {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit Crossover_CBC(ParameterDB *parameters = NULL)
		: Crossover(parameters) { }

	/**
	* Loads the needed parameters.
	* Loads the crossover probability
	*/
	//virtual void setup(ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~Crossover_CBC() { }; 	// Nothing to destroy here



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
		Individual *ind2, const SharedVarsEvolutionary *svars) const;

	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("Cycle-Based");
		return name;
	}

	virtual Crossover* clone() const {
		return new Crossover_CBC();
	}
};





//=============================================================================
//
//	Abstract class Crossover_PMX
//
//=============================================================================
/**
* PMX crossover for permutations
*
*
*/
class Crossover_PMX : public Crossover {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit Crossover_PMX(ParameterDB *parameters = NULL)
		: Crossover(parameters) { }

	/**
	* Loads the needed parameters.
	* Loads the crossover probability
	*/
	//virtual void setup(ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~Crossover_PMX() { }; 	// Nothing to destroy here



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
		Individual *ind2, const SharedVarsEvolutionary *svars) const;


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("PMX");
		return name;
	}

	virtual Crossover* clone() const {
		return new Crossover_PMX();
	}
};



}
