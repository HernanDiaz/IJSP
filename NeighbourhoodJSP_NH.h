/*
* NeighbourhoodJSP_NH.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodJSP_Base.h"

namespace JSP {

//=============================================================================
//
//	Class NB_ParallelNH_MakespanJSP
//
//=============================================================================
/**
* NH (extended) neighbourhood for Makespan minimization in JSP. Considers
* boundary arcs from all extreme critical paths across both extreme graphs.
*
* @author hdiaz
*/
class NB_ParallelNH_MakespanJSP : public NB_ParallelBase_MakespanJSP {
public:
	NB_ParallelNH_MakespanJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelBase_MakespanJSP(parameters) { }

	NB_ParallelNH_MakespanJSP(const NB_ParallelNH_MakespanJSP &source)
		: NB_ParallelBase_MakespanJSP(source) { }

	virtual Neighbourhood *clone() const {
		return new NB_ParallelNH_MakespanJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		return buildNBName("Makespan-NH", this->estimator == Estimator::ESTIM_HEADTAILS);
	}

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);

	virtual FuzzyFW::Fitness *evaluateNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars, const bool improvement = false);

	virtual void acceptNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);
};

} // namespace JSP
