/*
* Mutation.h
*
*  Created on: August 1, 2017
*/
#pragma once

#include "Population.h"


namespace FuzzyFW {



//=============================================================================
//
//	Abstract class Mutation
//
//=============================================================================
/**
* This class provides the framework to implement mutation operators to
* be used by evolutionary algorithms.
*
* Classes inheriting from this one have to implement the method "apply"
*
* The mutation operator will modify slightly the genotype of the individuals
* with a given probability
*
*
*/
class Mutation {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit Mutation(ParameterDB *parameters = NULL) {
		if (parameters != NULL)
			this->setup(parameters);
	}

	/**
	* Loads the needed parameters.
	* Nothing to load here
	*/
	virtual void setup(ParameterDB *parameters) { }

	/**
	* Destructor
	*/
	virtual ~Mutation() { } 	// Nothing to destroy here



//=========================================================================
//		METHODS
//=========================================================================
public:
	/**
	* Apply the mutation operator to an individual
	*
	* @param individual Individual to mutate
	* @param svars Shared elements of the algorithm
	*/
	virtual void apply(Individual *individual, const SharedVars *svars)
		const = 0;

	/**
	* Apply the mutation operator to a number of random individuals
	* in the population.
	*
	* @param population Population of individuals to mate
	* @param svars Shared elements of the algorithm
	*/
	virtual void apply(Population *population, const double probability, 
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
//	Class MutationInsertion
//
//=============================================================================
/**
 * This class inherits from Mutation and implements a insertion mutation.
 *
 * It selects a random gene in the genotype and inserts it in a random
 * position
 *
 *
 */
class MutationInsertion : public Mutation {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Default constructor
	 */
	explicit MutationInsertion(ParameterDB *parameters = NULL)
		: Mutation(parameters) { }

	/**
	 * Loads the needed parameters: Mutation per gene
	 */
	virtual void setup(ParameterDB *parameters) {
		Mutation::setup(parameters);
	}

	/**
	 * Destructor
	 */
	virtual ~MutationInsertion() { } 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	 * Apply the mutation operator to an individual
	 *
	 * @param individual Individual to mutate
	 * @param svars Shared elements of the algorithm
	 */
	virtual void apply(Individual *individual, const SharedVars *svars) const;


	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Insertion");
		return setup;
	}
};





//=============================================================================
//
//	Class MutationSwap
//
//=============================================================================
/**
* This class inherits from Mutation and implements a swap mutation.
*
* It selects two random genes in the genotype and swaps them
*
*
*/
class MutationSwap : public Mutation {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit MutationSwap(ParameterDB *parameters = NULL)
		: Mutation(parameters) { }

	/**
	* Loads the needed parameters: Mutation per gene
	*/
	virtual void setup(ParameterDB *parameters) {
		Mutation::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~MutationSwap() { } 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Apply the mutation operator to an individual
	*
	* @param individual Individual to mutate
	* @param svars Shared elements of the algorithm
	*/
	virtual void apply(Individual *individual, const SharedVars *svars) const;


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Swap");
		return setup;
	}
};





//=============================================================================
//
//	Class MutationInversion
//
//=============================================================================
/**
* This class inherits from Mutation and implements an inversion mutation.
*
* It selects two random positions and inverts the chain of genes bounded by
* those positions
*
*
*/
class MutationInversion : public Mutation {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit MutationInversion(ParameterDB *parameters = NULL)
		: Mutation(parameters) { }

	/**
	* Loads the needed parameters: Mutation per gene
	*/
	virtual void setup(ParameterDB *parameters) {
		Mutation::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~MutationInversion() { } 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Apply the mutation operator to an individual
	*
	* @param individual Individual to mutate
	* @param svars Shared elements of the algorithm
	*/
	virtual void apply(Individual *individual, const SharedVars *svars) const;


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Inversion");
		return setup;
	}
};


}
