/*
 * EvaluationIJSP.h
 *
 *  Created on: June 25, 2017
 *      Author: hdiaz
 */
#pragma once

#include "Evaluation.h"
#include "DecoderIJSP.h"


namespace IJSP {

/*
* Parameters:
*	SGS to use during evaluation
*	Type of AI calculation
*	How to compare objective functions that are Intervals
*	How to compute the maximum of Intervals (Makespan)
*/
#define IJSP_EVALUATION_AI "evaluation.ai.method"
#define IJSP_EVALUATION_COMPARE "evaluation.interval.comparison"
#define IJSP_EVALUATION_MAXIMUM "evaluation.interval.maximum"


/*
* Constant values:
*	SGS to use during evaluation
*/
#define IJSP_EVAL_AI_EXACT "exact"
#define IJSP_EVAL_AI_APROX "triangle"



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
class EvaluationIJSP_Makespan : public FuzzyFW::Evaluation {
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

	/*
	* Label for the strategy to ccompare values
	*/
	const std::string compareLabel;

	/*
	* Strategy to use to compare the job completion times
	*/
	FuzzyFW::Interval::Compare intervalCompare;



	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationIJSP_Makespan(FuzzyFW::ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationIJSP_Makespan(const EvaluationIJSP_Makespan & source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~EvaluationIJSP_Makespan() { }	// Nothing new to destroy, sadly

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation * clone() {
		return new EvaluationIJSP_Makespan(*this);
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
		name.push_back(";Maximum:;"
			+ FuzzyFW::Interval::getMaximum(this->intervalMaximum));
		name.push_back(";Comparisons:;"
			+ FuzzyFW::Interval::getComparison(this->intervalCompare));
		return name;
	}
};




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
class EvaluationIJSP_Tardiness : public FuzzyFW::Evaluation {
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

	/*
	* Label for the strategy to ccompare values
	*/
	const std::string compareLabel;

	/*
	* Strategy to use to compare the job completion times
	*/
	FuzzyFW::Interval::Compare intervalCompare;



	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationIJSP_Tardiness(FuzzyFW::ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationIJSP_Tardiness(const EvaluationIJSP_Tardiness & source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~EvaluationIJSP_Tardiness() { }	// Nothing new to destroy, sadly

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation * clone() {
		return new EvaluationIJSP_Tardiness(*this);
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
		name.push_back(";Maximum:;"
			+ FuzzyFW::Interval::getMaximum(this->intervalMaximum));
		name.push_back(";Comparisons:;"
			+ FuzzyFW::Interval::getComparison(this->intervalCompare));
		return name;
	}
};


}
