/*
 * Evaluation.h
 *
 *  Created on: July 10, 2017
 *      Author: jjpalacios
 */
#ifndef SRC_ECOBJECTS_EVALUATION_H_
#define SRC_ECOBJECTS_EVALUATION_H_

#include "Population.h"
#include "Decoder.h"


namespace FJSP {

/*
* Parameters:
*	SGS to use during evaluation
*	Type of AI calculation
*	How to compare objective functions that are TFNs
*	How to compute the maximum of TFN (Makespan)
*/
#define EVALUATION_AI "evaluation.ai.method"
#define EVALUATION_COMPARE "evaluation.tfn.comparison"
#define EVALUATION_MAXIMUM "evaluation.tfn.maximum"
#define EVALUATION_LAMARCK "evaluation.lamarckism"


/*
* Constant values:
*	SGS to use during evaluation
*/
#define EVAL_AI_EXACT "exact"
#define EVAL_AI_APROX "triangle"



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
	 * Computed the fitness value for a given individual
	 *
	 * @param svars Shared elements of the algorithm*
	 * @param individual Individual to evaluate
	 * @return the fitness value
	 */
	virtual Fitness * evaluate(const SharedVars * const svars,
		Individual *individual) const=0;

	/**
	 * Evaluates an entire population.
	 *
	 * @param population Population of individuals to evaluate
	 * @param svars Shared elements of the algorithm
	 * @param reEvaluate Force the evaluation of the individual
	 */
	virtual void evaluatePopulation(const SharedVars * const svars,
			Population *population, bool reEvaluate=false) const;


	/**
	 * Get the name and setup of the operator
	 *
	 * @return A string of parameter values. The first string is the name of
	 * the operator
	 */
	virtual std::vector<std::string> getName() const=0;
};





//=============================================================================
//
//	Class Evaluation_Makespan
//
//=============================================================================
/**
* This class implements the required methods to compute the makespan objective
* function from a given individual or population
*
* @author jjpalacios
*
*/
class EvaluationFJSP_Makespan : public Evaluation {
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
	TFN::Maximum tfnMaximum;

	/*
	* Label for the strategy to ccompare values
	*/
	const std::string compareLabel;

	/*
	* Strategy to use to compare the job completion times
	*/
	TFN::Compare tfnCompare;


	
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationFJSP_Makespan(ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationFJSP_Makespan(const EvaluationFJSP_Makespan & source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~EvaluationFJSP_Makespan() { }	// Nothing new to destroy, sadly

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() {
		return new EvaluationFJSP_Makespan(*this);
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
	virtual Fitness * evaluate(const SharedVars * const svars,
		Individual *individual) const;


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("Makespan");
		name.push_back(";Maximum:;" + TFN::getMaximum(this->tfnMaximum));
		name.push_back(";Comparisons:;" + TFN::getComparison(this->tfnCompare));
		return name;
	}
};





//=============================================================================
//
//	Class EvaluationFJSP_AImin
//
//=============================================================================
/**
* This class implements the required methods to compute the minimum Agreement
* Index of a given individual or population
*
* @author jjpalacios
*
*/
class EvaluationFJSP_AImin : public Evaluation {
	//=============================================================================
	//		FIELDS
	//=============================================================================
protected:
	/*
	* Label for knowing how to compute the AI
	*/
	const std::string exactLabel;

	/*
	* Indicates if the calculation of the AI is exact of approximated
	*/
	char isExact;



	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationFJSP_AImin(ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationFJSP_AImin(const EvaluationFJSP_AImin &source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~EvaluationFJSP_AImin() { }

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() {
		return new EvaluationFJSP_AImin(*this);
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
	virtual Fitness * evaluate(const SharedVars * const svars,
		Individual *individual) const;


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("AImin");
		name.push_back(";Calculation:;" +
			(isExact ? std::string(EVAL_AI_EXACT) : std::string(EVAL_AI_APROX)));
		return name;
	}


protected:
	/**
	* Calculates the agreement index of a completion time against a due-date
	*/
	double agreementIndex(const DueDate * const dd, const TFN &completionTime)
		const;
};





//=============================================================================
//
//	Class EvaluationFJSP_AIavg
//
//=============================================================================
/**
* This class implements the required methods to compute the average Agreement
* Index of a given individual or population
*
* @author jjpalacios
*
*/
class EvaluationFJSP_AIavg : public EvaluationFJSP_AImin {
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationFJSP_AIavg(ParameterDB *parameters = NULL)
		: EvaluationFJSP_AImin(parameters) { }

	/**
	* Copy constructor
	*/
	EvaluationFJSP_AIavg(const EvaluationFJSP_AIavg &source)
		: EvaluationFJSP_AImin(source) { }

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(ParameterDB *parameters) {
		EvaluationFJSP_AImin::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~EvaluationFJSP_AIavg() { }

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() {
		return new EvaluationFJSP_AIavg(*this);
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
	virtual Fitness * evaluate(const SharedVars * const svars,
		Individual *individual) const;


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("AIavg");
		name.push_back(";Calculation:;" +
			(this->isExact ? std::string(EVAL_AI_EXACT) : std::string(EVAL_AI_APROX)));
		return name;
	}
};


}

#endif /* SRC_ECOBJECTS_EVALUATION_H_ */
