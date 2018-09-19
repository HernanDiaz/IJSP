/*
* Mutation.h
*
*  Created on: August 1, 2017
*      Author: jjpalacios
*/

#ifndef SRC_ECOPERATORS_MUTATION_H_
#define SRC_ECOPERATORS_MUTATION_H_

#include "heading.h"
#include "SharedVars.h"
#include "Population.h"


namespace FJSP {

// Mutation parameters defined in this header file
#define MUTATION_PROBABILITY "mutation-probability"





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
* @author jjpalacios
*
*/
class Mutation {
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
	explicit Mutation(ParameterDB *parameters = NULL);

	/**
	* Loads the needed parameters.
	* Loads the mutation probability
	*/
	virtual void setup(ParameterDB *parameters);

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
	* Apply the mutation operator to all individuals in the population.
	*
	* @param population Population of individuals to mate
	* @param svars Shared elements of the algorithm
	*/
	virtual void apply(Population *population, const SharedVars *svars) const;

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
 * @author jjpalacios
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
		setup.push_back(";Probability;" + valueToString(this->probability));
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
* @author jjpalacios
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
		setup.push_back(";Probability;" + valueToString(this->probability));
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
* @author jjpalacios
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
		setup.push_back(";Probability;" + valueToString(this->probability));
		return setup;
	}
};


}

#endif /* SRC_ECOPERATORS_MUTATION_H_ */
