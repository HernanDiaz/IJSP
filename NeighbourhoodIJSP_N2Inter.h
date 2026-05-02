/*
* NeighbourhoodIJSP_N2Inter.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodIJSP_N2.h"

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelN2Inter_MakespanIJSP
//
//=============================================================================
/**
* N2-intersection neighbourhood for Makespan minimization in IJSP.
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
class NB_ParallelN2Inter_MakespanIJSP : public NB_ParallelN2_MakespanIJSP {
public:
	NB_ParallelN2Inter_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelN2_MakespanIJSP(parameters) { }

	NB_ParallelN2Inter_MakespanIJSP(const NB_ParallelN2Inter_MakespanIJSP & source)
		: NB_ParallelN2_MakespanIJSP(source) { }

	virtual Neighbourhood * clone() const {
		return new NB_ParallelN2Inter_MakespanIJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		return buildNBName("Makespan-N2Inter", this->estimator == Estimator::ESTIM_HEADTAILS);
	}

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);
};

}
