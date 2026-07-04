/*
 * EvaluationIJSP_MakespanEnergy.h
 *
 *  Created on: Jul 4, 2026
 *      Author: hdiaz
 */
#pragma once

#include "Evaluation.h"
#include "DecoderIJSP.h"
#include "ProblemIJSP.h"
#include "ScheduleIJSP.h"
#include "FitnessMO.h"
#include "EvaluationIJSP_Makespan.h"


namespace IJSP {

//=============================================================================
//
//	Class EvaluationIJSP_MakespanEnergy
//
//=============================================================================
/**
* Lexicographic bi-objective evaluation: makespan first, non-processing
* energy second (goal-free lexicographic scenario, cf. Afsar et al. 2024/25
* for the interval green FJSP). Produces a FitnessLexicographic whose
* components are two FitnessInterval values ranked with the globally
* configured interval comparison (evaluation.interval.comparison).
*
* @author hdiaz
*/
class EvaluationIJSP_MakespanEnergy : public FuzzyFW::Evaluation {
	//=============================================================================
	//		FIELDS
	//=============================================================================
protected:
	/*
	* Label for the strategy to compute the maximum
	*/
	const std::string maximumLabel;

	/*
	* Strategy to use to compute the maximum of job completion times
	*/
	FuzzyFW::Interval::Maximum intervalMaximum;

	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS / DESTRUCTOR
	//=============================================================================
public:
	EvaluationIJSP_MakespanEnergy();

	EvaluationIJSP_MakespanEnergy(const EvaluationIJSP_MakespanEnergy &source);

	virtual ~EvaluationIJSP_MakespanEnergy() { }

	virtual void setup(FuzzyFW::ParameterDB *parameters);

	virtual Evaluation * clone() {
		return new EvaluationIJSP_MakespanEnergy(*this);
	}

	//=============================================================================
	//		METHODS
	//=============================================================================
	/**
	* Builds the lexicographic (makespan, NPE) fitness of an already-built
	* schedule. Used both by evaluate() and at the local-search boundary,
	* where the single-objective makespan LS returns a schedule whose full
	* fitness must be recomputed.
	*
	* @param schedule Schedule to evaluate
	* @param problem Problem providing job count and passive powers
	* @return A FitnessLexicographic [makespan, NPE] (caller owns it)
	*/
	FuzzyFW::Fitness * evaluateSchedule(const ScheduleIJSP *schedule,
		const ProblemIJSP *problem) const;

	virtual FuzzyFW::Objective * getObjectiveFunction(
		const FuzzyFW::SharedVarsEvolutionary * const svars,
		FuzzyFW::Individual *individual) const;

	virtual FuzzyFW::Fitness * evaluate(
		const FuzzyFW::SharedVarsEvolutionary * const svars,
		FuzzyFW::Individual *individual) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> info;
		info.push_back("Objective Function:;Makespan, then NPE (lexicographic)");
		return info;
	}
};

}
