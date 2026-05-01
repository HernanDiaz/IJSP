/*
 * CrossoverIJSP_GOX.h
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */
#pragma once

#include "EncoderIJSP.h"
#include "Crossover.h"


namespace IJSP {

//=============================================================================
//
//	Class Crossover_GOXBierwirth
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
*
*/
class Crossover_GOXBierwirth : public FuzzyFW::Crossover {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Constructor using the parameters file
	*/
	Crossover_GOXBierwirth(FuzzyFW::ParameterDB *parameters = NULL)
		: Crossover(parameters) { }

	/**
	* Loads extra parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		Crossover::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~Crossover_GOXBierwirth() { }; 	// Nothing to destroy here



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
	virtual void apply(FuzzyFW::Individual *ind1,
		FuzzyFW::Individual *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("GOX-Bierwirth");
		return setup;
	}

	virtual Crossover* clone() const {
		return new Crossover_GOXBierwirth();
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
	virtual void applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	virtual void applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;
};

}
