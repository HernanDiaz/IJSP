/*
* NeighbourhoodIJSP_NH.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodIJSP_Base.h"

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelNH_MakespanIJSP
//
//=============================================================================
/**
* NH (extended) neighbourhood for Makespan minimization in IJSP. Considers
* boundary arcs from all extreme critical paths across both extreme graphs.
*
* @author hdiaz
*/
class NB_ParallelNH_MakespanIJSP : public NB_ParallelBase_MakespanIJSP {
public:
	NB_ParallelNH_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelBase_MakespanIJSP(parameters) { }

	NB_ParallelNH_MakespanIJSP(const NB_ParallelNH_MakespanIJSP &source)
		: NB_ParallelBase_MakespanIJSP(source) { }

	virtual Neighbourhood *clone() const {
		return new NB_ParallelNH_MakespanIJSP(*this);
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

} // namespace IJSP
