/*
 * EvaluationIJSP_Energy.h
 *
 *  Created on: Jul 4, 2026
 *      Author: hdiaz
 */
#pragma once

#include "Evaluation.h"
#include "DecoderIJSP.h"
#include "ProblemIJSP.h"
#include "ScheduleIJSP.h"


namespace IJSP {

//=============================================================================
//
//	Class EvaluationIJSP_Energy
//
//=============================================================================
/**
* This class implements the required methods to compute the non-processing
* (idle) energy objective from a given individual or population.
*
* NPE = sum over machines k of Pp(k) * (sum of idle gaps between consecutive
* tasks scheduled on k), computed component-wise on the interval heads of the
* schedule as built by the SGS (semi-active timing). Passive powers Pp(k) come
* from the problem instance (POTENCIA PASIVA section; default 1.0, in which
* case NPE is the total weighted idle TIME).
*
* Note: NPE is not a regular measure (right-shifting tasks can reduce it
* without changing the makespan); timing optimisation is deliberately out of
* scope of this evaluation and belongs to dedicated local search operators.
*
* @author hdiaz
*/
class EvaluationIJSP_Energy : public FuzzyFW::Evaluation {
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS / DESTRUCTOR
	//=============================================================================
public:
	EvaluationIJSP_Energy() { }

	EvaluationIJSP_Energy(const EvaluationIJSP_Energy &source)
		: FuzzyFW::Evaluation(source) { }

	virtual ~EvaluationIJSP_Energy() { }

	virtual Evaluation * clone() {
		return new EvaluationIJSP_Energy(*this);
	}

	//=============================================================================
	//		METHODS
	//=============================================================================
	/**
	* Computes the non-processing energy of a schedule, component-wise.
	*
	* @param schedule Schedule whose idle gaps are measured
	* @param problem Problem providing the passive power of each machine
	* @return NPE as an Interval
	*/
	static FuzzyFW::Interval computeNPE(const ScheduleIJSP *schedule,
		const ProblemIJSP *problem);

	virtual FuzzyFW::Objective * getObjectiveFunction(
		const FuzzyFW::SharedVarsEvolutionary * const svars,
		FuzzyFW::Individual *individual) const;

	virtual FuzzyFW::Fitness * evaluate(
		const FuzzyFW::SharedVarsEvolutionary * const svars,
		FuzzyFW::Individual *individual) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> info;
		info.push_back("Objective Function:;Non-processing energy");
		return info;
	}
};

}
