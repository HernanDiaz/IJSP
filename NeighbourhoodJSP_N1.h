/*
* NeighbourhoodJSP_N1.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodJSP_Base.h"

namespace JSP {

//=============================================================================
//
//	Class NB_ParallelN1_MakespanJSP
//
//=============================================================================
/**
* N1 neighbourhood for Makespan minimization in JSP. Considers the reversal
* of all arcs that belong to at least one extreme critical path.
*
* @author hdiaz
*/
class NB_ParallelN1_MakespanJSP : public NB_ParallelBase_MakespanJSP {
public:
	NB_ParallelN1_MakespanJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelBase_MakespanJSP(parameters) { }

	NB_ParallelN1_MakespanJSP(const NB_ParallelN1_MakespanJSP &source)
		: NB_ParallelBase_MakespanJSP(source) { }

	virtual Neighbourhood *clone() const {
		return new NB_ParallelN1_MakespanJSP(*this);
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

} // namespace JSP
