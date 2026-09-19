/*
 * EvaluationJSP_Tardiness.h
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
//	Class Evaluation_Tardiness
//
//=============================================================================
/**
* This class implements the required methods to compute the Tardiness objective
* function from a given individual or population
*
* @author hdiaz
*
*/
class EvaluationJSP_Tardiness : public FuzzyFW::Evaluation {
	//=============================================================================
	//		FIELDS
	//=============================================================================
	// As in EvaluationJSP_Makespan: no maximum strategy and no comparison
	// strategy survive the move to crisp times.

	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationJSP_Tardiness(FuzzyFW::ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationJSP_Tardiness(const EvaluationJSP_Tardiness & source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~EvaluationJSP_Tardiness() { }	// Nothing new to destroy, sadly

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation * clone() {
		return new EvaluationJSP_Tardiness(*this);
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
		name.push_back("Tardiness");
		return name;
	}
};

}
