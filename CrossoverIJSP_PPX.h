/*
 * CrossoverIJSP_PPX.h
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
//	Class Crossover_PPXBierwirth
//
//=============================================================================
/**
* PPX operator proposed by Bierwtih in this paper:
*		Bierwirth C., Mattfeld D.C., Kopfer H. (1996) On permutation
*		representations for scheduling problems. In: Parallel Problem Solving
*		from Nature - PPSN IV. PPSN 1996. LNCS, vol 1141.
*
* The operator creates a sequence of 1's and 2's as long as the genotype.
* This string is used to determine when to take genes from the first or
* the second parent.
* Example:
*	Parent 1: C B B B C A A A C
*	Parent 2: A A C B B A B C C
*	Sequence mask: 1 1 2 2 2 2 1 1 1
*	Offspring: C B A A B A B C C
*
*
*/
class Crossover_PPXBierwirth : public FuzzyFW::Crossover {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Constructor using the parameters file
	*/
	Crossover_PPXBierwirth(FuzzyFW::ParameterDB *parameters = NULL)
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
	virtual ~Crossover_PPXBierwirth() { }; 	// Nothing to destroy here



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
		setup.push_back("PPX-Bierwirth");
		return setup;
	}

	virtual Crossover* clone() const {
		return new Crossover_PPXBierwirth();
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
