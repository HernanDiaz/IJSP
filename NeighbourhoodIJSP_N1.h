/*
* NeighbourhoodIJSP_N1.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodIJSP_Base.h"

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelN1_MakespanIJSP
//
//=============================================================================
/**
* N1 neighbourhood for Makespan minimization in IJSP. Considers the reversal
* of all arcs that belong to at least one extreme critical path.
*
* @author hdiaz
*/
class NB_ParallelN1_MakespanIJSP : public NB_ParallelBase_MakespanIJSP {
public:
	NB_ParallelN1_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelBase_MakespanIJSP(parameters) { }

	NB_ParallelN1_MakespanIJSP(const NB_ParallelN1_MakespanIJSP &source)
		: NB_ParallelBase_MakespanIJSP(source) { }

	virtual Neighbourhood *clone() const {
		return new NB_ParallelN1_MakespanIJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		return buildNBName("Makespan-N1", this->estimator == Estimator::ESTIM_HEADTAILS);
	}

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);

	virtual FuzzyFW::Fitness *evaluateNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars, const bool improvement = false);

	virtual void acceptNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);
};

} // namespace IJSP
