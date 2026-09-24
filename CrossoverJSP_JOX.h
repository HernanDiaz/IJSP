/*
 * CrossoverJSP_JOX.h
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */
#pragma once

#include "CrossoverJSP_Base.h"


namespace JSP {

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
// I-039. How many jobs JOX keeps in place from the first parent.
//   "half"    (default, unchanged): each job with probability 1/2
//   "uniform" : the probability is drawn from U(0,1) at every crossing, so
//               children land at every distance between the parents, with
//               the same mean. No constant to tune.
#define JOX_MASK "crossover.jox.mask"

class Crossover_JOX : public CrossoverJSP_Base {
protected:
	bool uniformMask;

public:
	Crossover_JOX(FuzzyFW::ParameterDB *parameters = NULL)
		: CrossoverJSP_Base(parameters), uniformMask(false) {}

	virtual void setup(FuzzyFW::ParameterDB *parameters);

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
		Crossover_JOX *copy = new Crossover_JOX();
		copy->uniformMask = this->uniformMask;
		return copy;
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
