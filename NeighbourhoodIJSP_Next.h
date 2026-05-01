/*
* NeighbourhoodIJSP_Next.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourhoodIJSP_N2.h"

namespace IJSP {

//=============================================================================
//
//	Class NB_ParallelNext_MakespanIJSP
//
//=============================================================================
/**
* Extended N2 neighbourhood for Makespan minimization in IJSP.
* Considers the same boundary arcs as N2, plus interior critical-block arcs
* that pass a fast heads-and-tails viability filter.
*
* @author hdiaz
*/
class NB_ParallelNext_MakespanIJSP : public NB_ParallelN2_MakespanIJSP {
public:
	NB_ParallelNext_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelN2_MakespanIJSP(parameters) { }

	NB_ParallelNext_MakespanIJSP(const NB_ParallelNext_MakespanIJSP & source)
		: NB_ParallelN2_MakespanIJSP(source) { }

	virtual Neighbourhood * clone() const {
		return new NB_ParallelNext_MakespanIJSP(*this);
	}

	virtual std::vector<std::string> getName() {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Makespan-Next");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_NONE;
		else if (this->estimator == Estimator::ESTIM_HEADTAILS)
			value += NB_ESTIMATOR_HEADSTAILS;
		setup.push_back(value);
		return setup;
	}

	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);

private:
	bool isViableSwap(unsigned int x, unsigned int y);
};

}
