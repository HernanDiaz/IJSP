/*
* NeighbourhoodJSP_N2Inter.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodJSP_N2.h"

namespace JSP {

//=============================================================================
//
//	Class NB_ParallelN2Inter_MakespanJSP
//
//=============================================================================
/**
* N2-intersection neighbourhood for Makespan minimization in JSP.
*
* N2 (union) includes boundary arcs of critical blocks from G-(sigma) OR G+(sigma).
* N2Inter (intersection) restricts to arcs that are boundary in BOTH G-(sigma)
* AND G+(sigma) simultaneously — i.e. arcs that are promising under both the
* optimistic and the pessimistic extreme scenario.
*
* Fallback: if the intersection is empty (no arc is boundary in both graphs),
* the neighbourhood falls back to the full N2 union to guarantee progress.
*
* @author hdiaz
*/
class NB_ParallelN2Inter_MakespanJSP : public NB_ParallelN2_MakespanJSP {
public:
	NB_ParallelN2Inter_MakespanJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelN2_MakespanJSP(parameters) { }

	NB_ParallelN2Inter_MakespanJSP(const NB_ParallelN2Inter_MakespanJSP & source)
		: NB_ParallelN2_MakespanJSP(source) { }

	virtual Neighbourhood * clone() const {
		return new NB_ParallelN2Inter_MakespanJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		return buildNBName("Makespan-N2Inter", this->estimator == Estimator::ESTIM_HEADTAILS);
	}

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);
};

}
