/*
* LocalSearchAmico.h
*
*  Created on: Sep 27, 2018
*/
#pragma once

#include "LocalSearch.h"


namespace FuzzyFW {

//=============================================================================
//
//	Class LS_Tabu_Amico
//
//=============================================================================
/**
* This class defines the method to apply Tabu Search based on the dynamic
* tabu lists proposed in the paper:
*
* Applying tabu search to the job-shop scheduling problem. M. Dell'Amico and
* M. Trubian. Annals of Operations Research 41(1993) 231-252.
*
* It works like any other tabu search algorithm, but the size of the tabu
* list varies according to the following criteria:
*
* - if the current objective function value is better than the best value 
*	found before, then set the list length to 1
* - if we are in an improving phase of the search and the length of the list
*	is greater than a threshold min, then decrease the list length by one unit
* - if we are not in an improving phase of the search and the length of the
*	list is less than a given max, then increase the list length by one unit.
*
*
*/

#define FUZZYFW_LS_AMICO_CYCLE "localsearch.Tcycle"
#define FUZZYFW_LS_AMICO_LAMBDA "localsearch.Lambda"
#define FUZZYFW_LS_AMICO_MINA "localsearch.min.lower"
#define FUZZYFW_LS_AMICO_MINB "localsearch.min.upper"
#define FUZZYFW_LS_AMICO_MAXA "localsearch.max.lower"
#define FUZZYFW_LS_AMICO_MAXB "localsearch.max.upper"

class LS_Tabu_Amico : public LS_Tabu {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	/*
	* Limit of repeated neighbours to calify as a cycle
	*/
	std::string TcycleLabel;
	unsigned int Tcycle;

	/*
	* Lambda parameter. Fixes the number of iterations with a fix list size
	*/
	std::string LambdaLabel;
	unsigned int Lambda;

	/*
	* Bounds for minimum and maximum tabu list sizes
	*/
	std::string minALabel, minBLabel;
	unsigned int minA, minB;
	std::string maxALabel, maxBLabel;
	unsigned int maxA, maxB;


	/*
	* List of visited neighbours for cycle control
	*/
	std::vector<std::pair<Neighbour *, Fitness *> > cycleControlList;

	/*
	* Number of consecutive cycles of repeated elements
	*/
	unsigned int cycleCount;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit LS_Tabu_Amico(ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	LS_Tabu_Amico(const LS_Tabu_Amico &source);

	/**
	* Loads the needed parameters: No parameters needed
	*/
	virtual void setup(ParameterDB *parameters);

	/**
	* Clone method for inheriting classes
	*/
	virtual LS_Tabu_Amico * clone() const {
		return new LS_Tabu_Amico(*this);
	}

	/**
	* Destructor. Nothing to destroy
	*/
	virtual ~LS_Tabu_Amico() {	}



	//=========================================================================
	//		METHODS
	//=========================================================================
protected:
	/**
	* Clears the list for the control of cycles
	*/
	virtual void resetCycleControl();


	/**
	* Updates the minimum and maximum tabu list sizes
	*/
	virtual void updateTabuListBounds(const SharedVars *svars);


	/**
	* Checks if movement has been done earlier
	*/
	virtual bool isRepeatedMove(Neighbour *neighbour, Fitness *fitness);


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
		setup.push_back("Tabu Search (Dell'Amico)");
		for (size_t i = 0; i < tabu.size(); i++) {
			setup.push_back(";" + tabu[i]);
		}
		setup.push_back(";Tcycle:;"
			+ valueToString(this->Tcycle));
		setup.push_back(";Lambda:;"
			+ valueToString(this->Lambda));
		setup.push_back(";Min tabu list size:;["
			+ valueToString(this->minA) + ", "
			+ valueToString(this->minB));
		setup.push_back(";Max tabu list size:;["
			+ valueToString(this->maxA) + ", "
			+ valueToString(this->maxB));
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

