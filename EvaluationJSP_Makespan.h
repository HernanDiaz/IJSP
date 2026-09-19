/*
 * EvaluationJSP_Makespan.h
 *
 *  Created on: June 25, 2017
 *      Author: hdiaz
 */
#pragma once

#include "Evaluation.h"
#include "DecoderJSP.h"


namespace JSP {

#ifndef JSP_EVALUATION_AI
#define JSP_EVALUATION_AI "evaluation.ai.method"
#define JSP_EVALUATION_COMPARE "evaluation.interval.comparison"
#define JSP_EVALUATION_MAXIMUM "evaluation.interval.maximum"
#define JSP_EVAL_AI_EXACT "exact"
#define JSP_EVAL_AI_APROX "triangle"
#endif


//=============================================================================
//
//	Class Evaluation_Makespan
//
//=============================================================================
/**
* This class implements the required methods to compute the makespan objective
* function from a given individual or population
*
*
*/
class EvaluationJSP_Makespan : public FuzzyFW::Evaluation {
	//=============================================================================
	//		FIELDS
	//=============================================================================
	// The strategies for the maximum and for the comparison are gone with the
	// intervals: on crisp completion times std::max is the maximum and there is
	// one order. evaluation.interval.maximum and evaluation.interval.comparison
	// are still accepted in a setup file and ignored.

	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationJSP_Makespan(FuzzyFW::ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationJSP_Makespan(const EvaluationJSP_Makespan & source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~EvaluationJSP_Makespan() { }	// Nothing new to destroy, sadly

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation * clone() {
		return new EvaluationJSP_Makespan(*this);
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Computed the value of the objetive function of a given individual
	*
	* @param svars Shared elements of the algorithm*
	* @param individual Individual to evaluate
	* @return the value of the objective function
	*/
	virtual FuzzyFW::Objective * getObjectiveFunction(
		const FuzzyFW::SharedVarsEvolutionary * const svars,
		FuzzyFW::Individual *individual) const;


	/**
	* Computed the fitness value for a given individual
	*
	* @param svars Shared elements of the algorithm*
	* @param individual Individual to evaluate
	* @return the fitness value
	*/
	virtual FuzzyFW::Fitness * evaluate(
		const FuzzyFW::SharedVarsEvolutionary * const svars,
		FuzzyFW::Individual *individual) const;


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("Makespan");
		return name;
	}
};

}
