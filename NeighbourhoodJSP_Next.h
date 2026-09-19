/*
* NeighbourhoodJSP_Next.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodJSP_N2.h"

namespace JSP {

//=============================================================================
//
//	Class NB_ParallelNext_MakespanJSP
//
//=============================================================================
/**
* Extended N2 neighbourhood for Makespan minimization in JSP.
* Considers the same boundary arcs as N2, plus interior critical-block arcs
* that pass a fast heads-and-tails viability filter.
*
* @author hdiaz
*/
class NB_ParallelNext_MakespanJSP : public NB_ParallelN2_MakespanJSP {
public:
	NB_ParallelNext_MakespanJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelN2_MakespanJSP(parameters) { }

	NB_ParallelNext_MakespanJSP(const NB_ParallelNext_MakespanJSP & source)
		: NB_ParallelN2_MakespanJSP(source) { }

	virtual Neighbourhood * clone() const {
		return new NB_ParallelNext_MakespanJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		return buildNBName("Makespan-Next", this->estimator == Estimator::ESTIM_HEADTAILS);
	}

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);

private:
	bool isViableSwap(unsigned int x, unsigned int y);
};

}
