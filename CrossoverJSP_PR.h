/*
 * CrossoverJSP_PR.h
 *
 *  Created on: Sep 24, 2026
 *      Author: hdiaz
 */
#pragma once

#include "CrossoverJSP_Base.h"


namespace JSP {

//=============================================================================
//
//	Class Crossover_PR
//
//=============================================================================
/**
* Path relinking as a crossover (I-036). The children are two points of the
* path between the parents in the space of relative orders of operations (the
* k-th occurrence of a job is one operation): each operation takes the key
* (1 - a) * posA + a * posB, and the child is the operations sorted by key,
* for a = 1/3 and a = 2/3, fixed in advance. Every pair of operations both
* parents order alike keeps that order; the pairs they disagree on are settled
* by how far apart they sit in each parent. Never the best point of the path,
* which is what the IPRTS path relinking of H-2 returned and which stays next
* to its start. A first version walked by swapping differing POSITIONS, which
* in a job sequence scrambles the relative order; the pre-launch check caught
* it (ta23 1663 against 1573) before any filter was run.
*
* @author hdiaz
*/
class Crossover_PR : public CrossoverJSP_Base {
public:
	Crossover_PR(FuzzyFW::ParameterDB *parameters = NULL)
		: CrossoverJSP_Base(parameters) {}

	virtual ~Crossover_PR() {}

	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("PR");
		return setup;
	}

	virtual Crossover* clone() const {
		return new Crossover_PR();
	}

protected:
	virtual void applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	virtual void applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
		FuzzyFW::IndividualArrayInt *ind2,
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

};

}
