/*
 * CrossoverIJSP_GPMX.h
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */
#pragma once

#include "CrossoverIJSP_Base.h"


namespace IJSP {

//=============================================================================
//
//	Class Crossover_GPMXBierwirth
//
//=============================================================================
/**
* General PMX operator proposed by Bierwtih in this paper:
*		Bierwirth C., Mattfeld D.C., Kopfer H. (1996) On permutation
*		representations for scheduling problems. In: Parallel Problem Solving
*		from Nature - PPSN IV. PPSN 1996. LNCS, vol 1141.
*
* The operator chooses a random sequence of genes from parent 1. Then, in
* order to generate the offspring, it copies the genes of the second parent
* that are not included in the previously selected sequence until the position
* where the substring is. At that point, the sequence is included and
* then the genotype refill with the remaining genes of the second parent.
* Example:
*	Parent 1: A B B A C A B C B C
*	Parent 2: B A B B C A C C B A
*	Substring in parent 1: * * * A C A B * * *
*	Locate the key gene: B A B B* C* A* C C B A*
*	Offspring: B A B [A C A B] C C B
*
* Bierwith establishes that the length of the substring must be always in the
* interval [1/3, 1/2] times the length of the genotype
*
*
*/
class Crossover_GPMXBierwirth : public CrossoverIJSP_Base {
public:
	Crossover_GPMXBierwirth(FuzzyFW::ParameterDB *parameters = NULL)
		: CrossoverIJSP_Base(parameters) {}

	virtual ~Crossover_GPMXBierwirth() {}

	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("GPMX-Bierwirth");
		return setup;
	}

	virtual Crossover* clone() const {
		return new Crossover_GPMXBierwirth();
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
