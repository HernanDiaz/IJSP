/*
 * EvaluationFVRP.h
 *
 *  Created on: Nov 23, 2017
 */
#pragma once

#include "Evaluation.h"
#include "DecoderFVRP.h"

#define FVRP_EVALUATION_COMPARE "evaluation.tfn.comparison"
#define FVRP_EVALUATION_PENALTY "evaluation.tw.penalty"


namespace FVRP {



//=============================================================================
//
//	Class EvaluationFVRP_TimeCost
//
//=============================================================================
/**
* This class implements the required methods to compute the cost objective
* function from a given individual or population.
* This evaluation function evaluates the total travel time needed to complete
* all the routes.
*
*
*/
class EvaluationFVRP_TimeCost : public FuzzyFW::Evaluation {
	//=============================================================================
	//		FIELDS
	//=============================================================================
protected:
	/*
	* Label for the strategy to ccompare values
	*/
	const std::string compareLabel;

	/*
	* Strategy to use to compare the job completion times
	*/
	FuzzyFW::TFN::Compare tfnCompare;



	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationFVRP_TimeCost(FuzzyFW::ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationFVRP_TimeCost(const EvaluationFVRP_TimeCost & source);


	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~EvaluationFVRP_TimeCost() { }	// Nothing new to destroy, sadly

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation * clone() {
		return new EvaluationFVRP_TimeCost(*this);
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
		name.push_back("Travel Cost");
		name.push_back(";Comparisons:;"
			+ FuzzyFW::TFN::getComparison(this->tfnCompare));
		return name;
	}
};





//=============================================================================
//
//	Class EvaluationFVRP_CTW_TimeCost
//
//=============================================================================
/**
* This class implements the required methods to compute the cost objective
* function from a given individual or population.
* This evaluation function evaluates the total travel time needed to complete
* all the routes.
* Time Windows are present and considered to be crisp.
*
*
*/
class EvaluationFVRP_CTW_TimeCost : public EvaluationFVRP_TimeCost {
	//=============================================================================
	//		FIELDS
	//=============================================================================
protected:
	/*
	* Label for the strategy to ccompare values
	*/
	const std::string penaltyLabel;

	/*
	* Strategy to use to compare the job completion times
	*/
	double penalty;



	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationFVRP_CTW_TimeCost(FuzzyFW::ParameterDB *parameters = NULL)
		: penaltyLabel(FVRP_EVALUATION_PENALTY), penalty(1.0),
		EvaluationFVRP_TimeCost(parameters) { }

	/**
	* Copy constructor
	*/
	EvaluationFVRP_CTW_TimeCost(const EvaluationFVRP_CTW_TimeCost & source)
		: penaltyLabel(source.penaltyLabel), penalty(source.penalty),
		EvaluationFVRP_TimeCost(source) { }


	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~EvaluationFVRP_CTW_TimeCost() { }	// Nothing new to destroy, sadly

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation * clone() {
		return new EvaluationFVRP_CTW_TimeCost(*this);
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
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
		name.push_back("Travel Cost (Crisp TW)");
		name.push_back(";Comparisons:;"
			+ FuzzyFW::TFN::getComparison(this->tfnCompare));
		return name;
	}
};


}
