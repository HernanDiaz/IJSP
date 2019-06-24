/*
 * EvaluationIJSP.h
 *
 *  Created on: July 10, 2017
 *      Author: jjpalacios
 */
#pragma once

#include "Evaluation.h"
#include "DecoderIJSP.h"


namespace IJSP {

/*
* Parameters:
*	SGS to use during evaluation
*	Type of AI calculation
*	How to compare objective functions that are TFNs
*	How to compute the maximum of TFN (Makespan)
*/
#define IJSP_EVALUATION_AI "evaluation.ai.method"
#define IJSP_EVALUATION_COMPARE "evaluation.tfn.comparison"
#define IJSP_EVALUATION_MAXIMUM "evaluation.tfn.maximum"


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
* @author jjpalacios
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
			+ FuzzyFW::TFN::getMaximum(this->tfnMaximum));
		name.push_back(";Comparisons:;"
			+ FuzzyFW::TFN::getComparison(this->tfnCompare));
		return name;
	}
};





//=============================================================================
//
//	Class EvaluationIJSP_AImin
//
//=============================================================================
/**
* This class implements the required methods to compute the minimum Agreement
* Index of a given individual or population
*
* @author jjpalacios
*
*/
class EvaluationIJSP_AImin : public FuzzyFW::Evaluation {
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
	EvaluationIJSP_AImin(FuzzyFW::ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationIJSP_AImin(const EvaluationIJSP_AImin &source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~EvaluationIJSP_AImin() { }

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() {
		return new EvaluationIJSP_AImin(*this);
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
			(isExact ? std::string(IJSP_EVAL_AI_EXACT) : std::string(IJSP_EVAL_AI_APROX)));
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
//	Class EvaluationIJSP_AImin_ICAE
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
class EvaluationIJSP_AImin_ICAE : public EvaluationIJSP_AImin {
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationIJSP_AImin_ICAE(FuzzyFW::ParameterDB *parameters = NULL)
		: EvaluationIJSP_AImin(parameters) { }

	/**
	* Copy constructor
	*/
	EvaluationIJSP_AImin_ICAE(const EvaluationIJSP_AImin &source)
		: EvaluationIJSP_AImin(source) { }
	
	/**
	* Destructor
	*/
	virtual ~EvaluationIJSP_AImin_ICAE() { }

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() {
		return new EvaluationIJSP_AImin_ICAE(*this);
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
			(isExact ? std::string(IJSP_EVAL_AI_EXACT) : std::string(IJSP_EVAL_AI_APROX)));
		return name;
	}
};





//=============================================================================
//
//	Class EvaluationIJSP_AIavg
//
//=============================================================================
/**
* This class implements the required methods to compute the average Agreement
* Index of a given individual or population
*
* @author jjpalacios
*
*/
class EvaluationIJSP_AIavg : public EvaluationIJSP_AImin {
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationIJSP_AIavg(FuzzyFW::ParameterDB *parameters = NULL)
		: EvaluationIJSP_AImin(parameters) { }

	/**
	* Copy constructor
	*/
	EvaluationIJSP_AIavg(const EvaluationIJSP_AIavg &source)
		: EvaluationIJSP_AImin(source) { }

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		EvaluationIJSP_AImin::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~EvaluationIJSP_AIavg() { }

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() {
		return new EvaluationIJSP_AIavg(*this);
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
			(this->isExact ? std::string(IJSP_EVAL_AI_EXACT)
				: std::string(IJSP_EVAL_AI_APROX)));
		return name;
	}
};





//=============================================================================
//
//	Class EvaluationIJSP_ESDmin
//
//=============================================================================
/**
* This class implements the required methods to compute the minimum
* Expected Satisfaction Degree of a given individual or population
*
* @author jjpalacios
*
*/
class EvaluationIJSP_ESDmin : public FuzzyFW::Evaluation {
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationIJSP_ESDmin(FuzzyFW::ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	EvaluationIJSP_ESDmin(const EvaluationIJSP_AImin &source)
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
	virtual ~EvaluationIJSP_ESDmin() { }

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() {
		return new EvaluationIJSP_ESDmin(*this);
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
//	Class EvaluationIJSP_ESDavg
//
//=============================================================================
/**
* This class implements the required methods to compute the average Expected
* Satisfaction Degree of a given individual or population
*
* @author jjpalacios
*
*/
class EvaluationIJSP_ESDavg : public EvaluationIJSP_ESDmin {
	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
public:
	/**
	* Default constructor
	*/
	EvaluationIJSP_ESDavg(FuzzyFW::ParameterDB *parameters = NULL)
		: EvaluationIJSP_ESDmin(parameters) { }

	/**
	* Copy constructor
	*/
	EvaluationIJSP_ESDavg(const EvaluationIJSP_AIavg &source)
		: EvaluationIJSP_ESDmin(source) { }

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		EvaluationIJSP_ESDmin::setup(parameters);
	}

	/**
	* Destructor
	*/
	virtual ~EvaluationIJSP_ESDavg() { }

	/*
	* Clone method for inherited operators
	*/
	virtual Evaluation* clone() {
		return new EvaluationIJSP_ESDavg(*this);
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
