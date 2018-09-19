/*
 * EvaluationFJSP.h
 *
 *  Created on: July 10, 2017
 *      Author: jjpalacios
 */
#pragma once

#include "Evaluation.h"
#include "DecoderFJSP.h"


namespace FJSP {

/*
* Parameters:
*	SGS to use during evaluation
*	Type of AI calculation
*	How to compare objective functions that are TFNs
*	How to compute the maximum of TFN (Makespan)
*/
#define FJSP_EVALUATION_AI "evaluation.ai.method"
#define FJSP_EVALUATION_COMPARE "evaluation.tfn.comparison"
#define FJSP_EVALUATION_MAXIMUM "evaluation.tfn.maximum"


/*
* Constant values:
*	SGS to use during evaluation
*/
#define FJSP_EVAL_AI_EXACT "exact"
#define FJSP_EVAL_AI_APROX "triangle"



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
class EvaluationFJSP_Makespan : public FuzzyFW::Evaluation {
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
	FuzzyFW::TFN::Maximum tfnMaximum;

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
	EvaluationFJSP_Makespan(FuzzyFW::ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationFJSP_Makespan(const EvaluationFJSP_Makespan & source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~EvaluationFJSP_Makespan() { }	// Nothing new to destroy, sadly

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation * clone() {
		return new EvaluationFJSP_Makespan(*this);
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
			+ FuzzyFW::TFN::getMaximum(this->tfnMaximum));
		name.push_back(";Comparisons:;"
			+ FuzzyFW::TFN::getComparison(this->tfnCompare));
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
class EvaluationFJSP_AImin : public FuzzyFW::Evaluation {
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
	bool isExact;



	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationFJSP_AImin(FuzzyFW::ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationFJSP_AImin(const EvaluationFJSP_AImin &source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

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
		name.push_back("AImin");
		name.push_back(";Calculation:;" +
			(isExact ? std::string(FJSP_EVAL_AI_EXACT) : std::string(FJSP_EVAL_AI_APROX)));
		return name;
	}

	/**
	* Calculates the agreement index of a completion time against a due-date
	*/
	static double agreementIndex(const FuzzyFW::TimeWindow * const dd,
		const FuzzyFW::TFN &completionTime, const bool exact);
};





//=============================================================================
//
//	Class EvaluationFJSP_AImin_ICAE
//
//=============================================================================
/**
* This class implements the required methods to compute the minimum Agreement
* Index of a given individual or population. 
* It also uses a lexicopraphical approach to solve ties. In this case, each
* AI is considered as an objective function and the lexicoprahical order is
* done by sorting the jobs from lowest to highest AI value.
*
* @author jjpalacios
*
*/
class EvaluationFJSP_AImin_ICAE : public EvaluationFJSP_AImin {
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationFJSP_AImin_ICAE(FuzzyFW::ParameterDB *parameters = NULL)
		: EvaluationFJSP_AImin(parameters) { }

	/**
	* Copy constructor
	*/
	EvaluationFJSP_AImin_ICAE(const EvaluationFJSP_AImin &source)
		: EvaluationFJSP_AImin(source) { }
	
	/**
	* Destructor
	*/
	virtual ~EvaluationFJSP_AImin_ICAE() { }

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() {
		return new EvaluationFJSP_AImin_ICAE(*this);
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Computed the fitness values for the lexicographical order
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
		name.push_back("AImin-ICAE");
		name.push_back(";Calculation:;" +
			(isExact ? std::string(FJSP_EVAL_AI_EXACT) : std::string(FJSP_EVAL_AI_APROX)));
		return name;
	}
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
	EvaluationFJSP_AIavg(FuzzyFW::ParameterDB *parameters = NULL)
		: EvaluationFJSP_AImin(parameters) { }

	/**
	* Copy constructor
	*/
	EvaluationFJSP_AIavg(const EvaluationFJSP_AIavg &source)
		: EvaluationFJSP_AImin(source) { }

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
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
		name.push_back("AIavg");
		name.push_back(";Calculation:;" +
			(this->isExact ? std::string(FJSP_EVAL_AI_EXACT)
				: std::string(FJSP_EVAL_AI_APROX)));
		return name;
	}
};





//=============================================================================
//
//	Class EvaluationFJSP_ESDmin
//
//=============================================================================
/**
* This class implements the required methods to compute the minimum
* Expected Satisfaction Degree of a given individual or population
*
* @author jjpalacios
*
*/
class EvaluationFJSP_ESDmin : public FuzzyFW::Evaluation {
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationFJSP_ESDmin(FuzzyFW::ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationFJSP_ESDmin(const EvaluationFJSP_AImin &source)
		: Evaluation(source) { }

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		Evaluation::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~EvaluationFJSP_ESDmin() { }

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() {
		return new EvaluationFJSP_ESDmin(*this);
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
		name.push_back("ESDmin");
		return name;
	}

	/**
	* Calculates the agreement index of a completion time against a due-date
	*/
	static double satisfactionDegree(const FuzzyFW::TimeWindow * const dd,
		const FuzzyFW::TFN &completionTime);
};





//=============================================================================
//
//	Class EvaluationFJSP_ESDavg
//
//=============================================================================
/**
* This class implements the required methods to compute the average Expected
* Satisfaction Degree of a given individual or population
*
* @author jjpalacios
*
*/
class EvaluationFJSP_ESDavg : public EvaluationFJSP_ESDmin {
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationFJSP_ESDavg(FuzzyFW::ParameterDB *parameters = NULL)
		: EvaluationFJSP_ESDmin(parameters) { }

	/**
	* Copy constructor
	*/
	EvaluationFJSP_ESDavg(const EvaluationFJSP_AIavg &source)
		: EvaluationFJSP_ESDmin(source) { }

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		EvaluationFJSP_ESDmin::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~EvaluationFJSP_ESDavg() { }

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() {
		return new EvaluationFJSP_ESDavg(*this);
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
		name.push_back("ESDavg");
		return name;
	}
};


}
