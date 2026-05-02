/*
 * CrossoverIJSP_JOX.h
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */
#pragma once

#include "CrossoverIJSP_Base.h"


namespace IJSP {

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
 * @author hdiaz
 *
 */
class Crossover_JOX : public CrossoverIJSP_Base {
public:
	Crossover_JOX(FuzzyFW::ParameterDB *parameters = NULL)
		: CrossoverIJSP_Base(parameters) {}

	virtual ~Crossover_JOX() {}

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
		setup.push_back("JOX");
		return setup;
	}

	virtual Crossover* clone() const {
		return new Crossover_JOX();
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
