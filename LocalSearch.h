/*
* LocalSearch.h
*
*  Created on: Oct 11, 2017
*      Author: jjpalacios
*/
#pragma once

#include "Neighbourhood.h"
#include "TabuList.h"


namespace FuzzyFW {

// Creation parameters defined in this header file
#define FUZZYFW_LOCAL_SEARCH_ITER "localsearch.max-iterations"
#define FUZZYFW_LOCAL_SEARCH_EVAL "localsearch.max-evaluations"
#define FUZZYFW_LOCAL_SEARCH_TIME "localsearch.max-time"

#define  FUZZYFW_LOCAL_SEARCH_DRIVE "localsearch.estimation-guide"
#define  FUZZYFW_LOCAL_SEARCH_FILTER "localsearch.filter"

#define FUZZYFW_LOCAL_SEARCH_TABUITER "localsearch.bad-iterations"



//=============================================================================
//
//	Abstract class LocalSearch
//
//=============================================================================
/**
* This class defines how Local Search strategies must be implemented.
* Local Search strategies take a solution as input and improve it
* iteratively until a stopping criterion is met.
*
* For any kind of Local Search, we look for these parameters:
*  -	max-iterations: Maximum number of iterations for the local search. It
*  		is an optional parameter.
*	-	max-evaluations: Maxmimum number of evaluations for the local search.
		It is an optional parameter
* 	-	threshold: Minimum improvement to consider a neighbor as an improving
* 			neighbor. Optional parameter
*
* @author jjpalacios
*
*/
class LocalSearch {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* Maximum number of iterations to be done by LS
	*/
	std::string iterationLabel;
	int maxIterations;

	/*
	* Maximum number of full evaluations to be done by LS
	*/
	std::string evaluationLabel;
	int maxEvaluations;

	/*
	* Maximum time to run the local search for
	*/
	std::string timeLabel;
	double maxTime;

	/*
	* Indicates if the local search is guided by full evaluation or estimations
	*/
	std::string guideLabel;
	char estimationGuided;

	/*
	* Indicates if the estimation is used as filtering mechanism
	*/
	std::string filterLabel;
	char estimationFilter;


	/*
	* Neighbourhood to use for the Local Search
	*/
	Neighbourhood * neighbourhood;

	//! Number of neighbours completely evaluated
	unsigned int evaluations;

	//! Number of generated neighbours
	unsigned int neighbours;

	//! Number of iterations
	unsigned int iterations;

	//! Runtime
	clock_t runtime;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit LocalSearch(ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	LocalSearch(const LocalSearch &source);

	/**
	* Loads the needed parameters: No parameters needed
	*/
	virtual void setup(ParameterDB *parameters);

	/**
	* Clone method for inheriting classes
	*/
	virtual LocalSearch * clone() const = 0;

	/**
	* Destructor
	*/
	virtual ~LocalSearch() {
		delete neighbourhood;
	}



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	* Get the number of solutions evaluated in the last run
	*/
	unsigned int getEvaluations() const {
		return this->evaluations;
	}

	/**
	* Get the number of iterations in the last run
	*/
	unsigned int getIterations() const {
		return this->iterations;
	}

	/**
	* Get the number of neighbours generated
	*/
	unsigned int getNeighbours() const {
		return this->neighbours;
	}

	/**
	* Establishes the neighbourhood to use
	*/
	void setNeighbourhood(Neighbourhood *n) {
		this->neighbourhood = n;
	}


protected:
	/**
	* Indicates if the stopping criteria has been met
	*/
	virtual bool stoppingCriteria();




	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Apply the local search to a given solution. The method returns the best
	* solution found during the local search procedure. It also updates the
	* inner structures regarding the data of the run
	*/
	virtual FullSolution apply(const Solution *solution, const Fitness *fitness,
		const SharedVars *svars) = 0;


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const = 0;

};





//=============================================================================
//
//	Class LS_HillClimbing
//
//=============================================================================
/**
* This class defines the method to apply the famous Hill Climbing algorithm
* to a given solution. Roughly speaking, the algorithm generates the
* neighbourhood and then iterates randomly over the individuals until it
* finds the first one that improves the current solution.
*
* @author jjpalacios
*
*/
class LS_HillClimbing : public LocalSearch {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit LS_HillClimbing(ParameterDB *parameters = NULL)
		: LocalSearch(parameters) { }

	/**
	* Copy constructor
	*/
	LS_HillClimbing(const LS_HillClimbing &source)
		: LocalSearch(source) { }

	/**
	* Loads the needed parameters: No parameters needed
	*/
	virtual void setup(ParameterDB *parameters) {
		LocalSearch::setup(parameters);
	}

	/**
	* Clone method for inheriting classes
	*/
	virtual LS_HillClimbing * clone() const {
		return new LS_HillClimbing(*this);
	}

	/**
	* Destructor
	*/
	virtual ~LS_HillClimbing() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Apply the local search to a given solution. The method returns the best
	* solution found during the local search procedure. It also updates the
	* inner structures regarding the data of the run
	*/
	virtual FullSolution apply(const Solution *solution, const Fitness *fitness,
		const SharedVars *svars);


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		std::string line;
		setup.push_back("BF-HillClimbing");
		if(this->maxIterations >= 0)
			setup.push_back(";Max. Iterations:;"
				+ valueToString(this->maxIterations));
		if (this->maxEvaluations >= 0)
			setup.push_back(";Max. Evaluations:;"
				+ valueToString(this->maxEvaluations));
		if (this->maxTime >= 0)
			setup.push_back(";Max. Time:;"
				+ valueToString(this->maxTime));
		if (this->estimationGuided)
			setup.push_back(";Guide:;Estimations");
		else
			setup.push_back(";Guide:;Real fitness");
		if (this->estimationFilter)
			setup.push_back(";Filter:;Yes");
		else
			setup.push_back(";Filter:;No");
		return setup;
	}
};





//=============================================================================
//
//	Class LS_GradientDescent
//
//=============================================================================
/**
* This class defines the method to apply the famous Hill Climbing algorithm
* to a given solution. Roughly speaking, the algorithm generates the
* neighbourhood and then iterates randomly over the individuals until it
* finds the first one that improves the current solution.
*
* @author jjpalacios
*
*/
class LS_GradientDescent : public LocalSearch {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit LS_GradientDescent(ParameterDB *parameters = NULL)
		: LocalSearch(parameters) { }

	/**
	* Copy constructor
	*/
	LS_GradientDescent(const LS_HillClimbing &source)
		: LocalSearch(source) { }

	/**
	* Loads the needed parameters: No parameters needed
	*/
	virtual void setup(ParameterDB *parameters) {
		LocalSearch::setup(parameters);
	}

	/**
	* Clone method for inheriting classes
	*/
	virtual LS_GradientDescent * clone() const {
		return new LS_GradientDescent(*this);
	}

	/**
	* Destructor
	*/
	virtual ~LS_GradientDescent() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Apply the local search to a given solution. The method returns the best
	* solution found during the local search procedure. It also updates the
	* inner structures regarding the data of the run
	*/
	virtual FullSolution apply(const Solution *solution, const Fitness *fitness,
		const SharedVars *svars);


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		std::string line;
		setup.push_back("HillClimbing");
		if (this->maxIterations >= 0)
			setup.push_back(";Max. Iterations:;"
				+ valueToString(this->maxIterations));
		if (this->maxEvaluations >= 0)
			setup.push_back(";Max. Evaluations:;"
				+ valueToString(this->maxEvaluations));
		if (this->maxTime >= 0)
			setup.push_back(";Max. Time:;"
				+ valueToString(this->maxTime));
		if (this->estimationGuided)
			setup.push_back(";Guide:;Estimations");
		else
			setup.push_back(";Guide:;Real fitness");
		if (this->estimationFilter)
			setup.push_back(";Filter:;Yes");
		else
			setup.push_back(";Filter:;No");
		return setup;
	}
};





//=============================================================================
//
//	Class LS_Tabu
//
//=============================================================================
/**
* This class defines the method to apply Tabu Search. Roughly speaking, 
* the algorithm generates the neighbourhood and then finds the best
* individual. The algorithm stops after a fixed number of generations
* without improving the best solution found so far.
*
* @author jjpalacios
*
*/
class LS_Tabu : public LocalSearch {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* List of tabu movements
	*/
	TabuList *tabuList;

	/*
	* Maximum number of iterations without an improvement
	*/
	unsigned int maxBadIterations;
	std::string badIterationsLabel;

	/*
	* Current number of iterations without improvement
	*/
	unsigned int badIterations;

	

	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit LS_Tabu(ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	LS_Tabu(const LS_Tabu &source);

	/**
	* Loads the needed parameters: No parameters needed
	*/
	virtual void setup(ParameterDB *parameters);

	/**
	* Clone method for inheriting classes
	*/
	virtual LS_Tabu * clone() const {
		return new LS_Tabu(*this);
	}

	/**
	* Destructor
	*/
	virtual ~LS_Tabu() {
		delete this->tabuList;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
protected:
	/**
	* Indicates if the stopping criteria has been met
	*/
	virtual bool stoppingCriteria();


public:
	/**
	* Apply the local search to a given solution. The method returns the best
	* solution found during the local search procedure. It also updates the
	* inner structures regarding the data of the run
	*/
	virtual FullSolution apply(const Solution *solution, const Fitness *fitness,
		const SharedVars *svars);


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup, tabu;
		std::string line;

		tabu = this->tabuList->getName();
		setup.push_back("Tabu Search");
		for (size_t i = 0; i < tabu.size(); i++) {
			setup.push_back(";" + tabu[i]);
		}
		setup.push_back(";Max. Iterations without Improve:;"
			+ valueToString(this->maxBadIterations));
		if (this->maxIterations >= 0)
			setup.push_back(";Max. Iterations:;"
				+ valueToString(this->maxIterations));
		if (this->maxEvaluations >= 0)
			setup.push_back(";Max. Evaluations:;"
				+ valueToString(this->maxEvaluations));
		if (this->maxTime >= 0)
			setup.push_back(";Max. Time:;"
				+ valueToString(this->maxTime));
		if (this->estimationGuided)
			setup.push_back(";Guide:;Estimations");
		else
			setup.push_back(";Guide:;Real fitness");
		if (this->estimationFilter)
			setup.push_back(";Filter:;Yes");
		else
			setup.push_back(";Filter:;No");
		return setup;
	}
};

}

