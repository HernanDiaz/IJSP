/*
 * Evaluation.h
 *
 *  Created on: July 10, 2017
 *      Author: jjpalacios
 */
#pragma once

#include "Population.h"
#include "SharedVarsEvolutionary.h"
#include "FitnessMO.h"


namespace FuzzyFW {

typedef Fitness Objective;


#define FUZZYFW_EVALUATION_LAMARCK "evaluation.lamarckism"


//=============================================================================
//
//	Abstract class Evaluation
//
//=============================================================================
/**
 * This class provides the framework to implement an evaluation function
 *
 * It will be responsible of assigning a fitness value to each individual
 *
 * @author jjpalacios
 *
 */
class Evaluation {
protected:
	//=============================================================================
	//		COMMON FIELDS
	//=============================================================================
	/*
	* Label to indicate if lamarckism is desired
	*/
	const std::string lamarckLabel;

	/*
	* Indicates weather or not, lamarckism is applied after evaluation
	*/
	char lamarckism;



	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	 * Default constructor
	 */
	Evaluation(ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	Evaluation(const Evaluation & source);	// Nothing to copy


	/**
	 * Loads the needed parameters. Nothing by default
	 */
	virtual void setup(ParameterDB *parameters);

	/**
	 * Destructor
	 */
	virtual ~Evaluation() { }; 	// Nothing to destroy here

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() = 0;



	//=============================================================================
	//		METHODS
	//=============================================================================
public:
	/**
	* Computed the value of the objetive function of a given individual
	*
	* @param svars Shared elements of the algorithm*
	* @param individual Individual to evaluate
	* @return the value of the objective function
	*/
	virtual Objective * getObjectiveFunction(const SharedVarsEvolutionary * const svars,
		Individual *individual) const = 0;


	/**
	 * Computed the fitness value for a given individual
	 *
	 * @param svars Shared elements of the algorithm*
	 * @param individual Individual to evaluate
	 * @return the fitness value
	 */
	virtual Fitness * evaluate(const SharedVarsEvolutionary * const svars,
		Individual *individual) const=0;

	/**
	 * Evaluates an entire population.
	 *
	 * @param population Population of individuals to evaluate
	 * @param svars Shared elements of the algorithm
	 * @param reEvaluate Force the evaluation of the individual
	 */
	virtual void evaluatePopulation(
		const SharedVarsEvolutionary * const svars,
		Population *population, bool reEvaluate=false) const;


	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const=0;
};


}
