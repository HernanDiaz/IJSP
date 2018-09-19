/*
 * Crossover.h
 *
 *  Created on: July 14, 2017
 *      Author: jjpalacios
 */
#ifndef SRC_ECOPERATORS_CROSSOVER_H_
#define SRC_ECOPERATORS_CROSSOVER_H_

#include "Encoder.h"
#include "Population.h"


namespace FJSP {

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
			Individual *ind2, const SharedVars *svars) const=0;

	/**
	 * Apply the crossover operator to all pairs in the population. The
	 * offspring will replace their parents in the original population
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
	virtual std::vector<std::string> getName() const=0;
};





//=============================================================================
//
//	Class Crossover_JOX
//
//=============================================================================
/**
 * Job Order Crossover operator. This poerator is especifically designed to
 * work on job shop scheduling problems.
 *
 * The operator chooses a random set of jobs from parent 1 and keeps all
 * operations belonging to those jobs in their original position. The rest
 * of the chromosome is then filled by scheduling the remaining tasks following
 * the ordering given by the second parent.
 *
 * @author jjpalacios
 *
 */
class Crossover_JOX : public Crossover {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	 * Constructor using the parameters file
	 */
	Crossover_JOX(ParameterDB *parameters = NULL)
		: Crossover(parameters) { }

	/**
	 * Loads extra parameters.
	 */
	virtual void setup(ParameterDB *parameters) {
		Crossover::setup(parameters);
	}

	/**
	 * Destructor
	 */
	virtual ~Crossover_JOX() { }; 	// Nothing to destroy here



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
	 * @return The two offspring produced
	 */
	virtual void apply(Individual *ind1,
			Individual *ind2, const SharedVars *svars) const;

	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("JOX");
		setup.push_back(";Probability;" + valueToString(this->probability));
		return setup;
	}

protected:
	/**
	 * Crossover operator depending on the type of individual we are
	 * receiving
	 *
	 * @param ind1 First parent for the mating
	 * @param ind2 Second parent for the mating
	 * @param svars Shared elements of the algorithm
	 * @return The two offspring produced
	 */
	virtual void applyPermutation(IndividualArrayInt *ind1,
		IndividualArrayInt *ind2, const SharedVars *svars) const;

	virtual void applyJobPermutation(IndividualArrayInt *ind1,
		IndividualArrayInt *ind2, const SharedVars *svars) const;

};





//=============================================================================
//
//	Class Crossover_Bierwirth
//
//=============================================================================
/**
* General Order Crossover operator proposed by Bierwtih in this paper:
*		Bierwirth, C. A Generalized Permutation Approach to Jobshop Scheduling
*		with Genetic Algorithms. OR Spectrum (17), 87-92. 1995
*
* The operator chooses a random sequence of genes from parent 1. Then, in
* order to generate the offspring, it copies the genes of the second parent
* that are not included in the previously selected sequence until the first
* gene of the sequecne is found. At that point, the sequence is included and
* then the genotype refill with the remaining genes of the second parent.
* Example:
*	Parent 1: A B B A C A B C B C
*	Parent 2: B A B B C A C C B A
*	Substring in parent 1: * * * A C A B * * *
*	Locate the key gene: B A B B* C* [A] C C B A*
*	Offspring: B A B [A C A B] C C B
*		
* Bierwith establishes that the length of the substring must be always in the
* interval [1/3, 1/2] times the length of the genotype
*
* @author jjpalacios
*
*/
class Crossover_Bierwirth : public Crossover {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Constructor using the parameters file
	*/
	Crossover_Bierwirth(ParameterDB *parameters = NULL)
		: Crossover(parameters) { }

	/**
	* Loads extra parameters.
	*/
	virtual void setup(ParameterDB *parameters) {
		Crossover::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~Crossover_Bierwirth() { }; 	// Nothing to destroy here



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
	* @return The two offspring produced
	*/
	virtual void apply(Individual *ind1,
		Individual *ind2, const SharedVars *svars) const;

	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("GOX-Bierwirth");
		setup.push_back(";Probability;" + valueToString(this->probability));
		return setup;
	}

protected:
	/**
	* Crossover operator depending on the type of individual we are
	* receiving
	*
	* @param ind1 First parent for the mating
	* @param ind2 Second parent for the mating
	* @param svars Shared elements of the algorithm
	* @return The two offspring produced
	*/
	virtual void applyPermutation(IndividualArrayInt *ind1,
		IndividualArrayInt *ind2, const SharedVars *svars) const;

	virtual void applyJobPermutation(IndividualArrayInt *ind1,
		IndividualArrayInt *ind2, const SharedVars *svars) const;
};

}

#endif /* SRC_ECOPERATORS_CROSSOVER_H_ */
