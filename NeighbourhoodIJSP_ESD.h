/*
* NeighbourhoodFJSP_ESD.h
*
*  Created on: Oct 11, 2017
*      Author: jjpalacios
*/
#pragma once

#include "NeighbourIJSP.h"
#include "Neighbourhood.h"


namespace IJSP {

// Creation parameters defined in this header file
#define NB_ESTIMATOR_ESD_NONE "none"
#define NB_ESTIMATOR_ESD_HEADSTAILS "heads&tails"


//=============================================================================
//
//	Class NB_ParallelN1_ESDavgIJSP
//
//=============================================================================
/**
* This class defines the neighbourhood structure known as N1 for average
* ESD maximization in IJSP. This neighbourhood is done by
* splitting the fuzzy graph in three parallel graphs. Then, the longest path
* in any of them from start to the end node of a job such that AI<1, is
* considered a critical path. N1 considers the reversal of all arcs that belong
* to at least one critical path
*
* @author hdiaz
*
*/
class NB_ParallelN1_ESDavgIJSP : public FuzzyFW::Neighbourhood {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* Types of estimators for this neghbouhood
	*/
	enum Estimator { NONE, ESTIM_HEADTAILS };

	/*
	* Label for the estimator to use
	*/
	std::string estimatorLabel;

	/*
	* Estimator to use
	*/
	Estimator estimator;

	/*
	* Fuzzy Schedule to work with
	*/
	ScheduleIJSP *schedule;

	/*
	* Current fitness to work with
	*/
	FuzzyFW::Fitness *currentFitness;

	/*
	* Array of neighbours generated
	*/
	std::vector<IJSP::NeighbourIJSP_Arc *> neighbours;


	/*
	* Matrix of tails for each operation on each job
	*/
	std::vector< std::vector<FuzzyFW::Interval> > tails;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Main constructor
	*/
	NB_ParallelN1_ESDavgIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: Neighbourhood(parameters), estimatorLabel(NEIGHBOURHOOD_ESTIMATOR),
		estimator(Estimator::NONE), schedule(NULL), currentFitness(NULL) { }


	/*
	* Copy constructor
	*/
	NB_ParallelN1_ESDavgIJSP(const NB_ParallelN1_ESDavgIJSP & source);


	/**
	* Loads the needed parameters
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);


	/*
	* Clone method
	*/
	virtual Neighbourhood * clone() const {
		return new NB_ParallelN1_ESDavgIJSP(*this);
	}


	/*
	* Destructor
	*/
	~NB_ParallelN1_ESDavgIJSP();



	//=========================================================================
	//		GET / SET METHODS
	//=========================================================================
public:
	/*
	* Gets the current solution
	*/
	virtual FuzzyFW::FullSolution getCurrentSolution() {
		FuzzyFW::FullSolution solution;
		solution.first = this->schedule;
		solution.second = this->currentFitness;
		return solution;
	}


	/*
	* Gets a neighbour from the neighbourhood
	*/
	virtual FuzzyFW::Neighbour* getNeighbour(const unsigned int idx);


	/*
	* Name of the Neighbourhood structure
	*/
	virtual std::vector<std::string>getName() {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("ESDavg-N1");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_ESD_NONE;
		else if (this->estimator == Estimator::ESTIM_HEADTAILS)
			value += NB_ESTIMATOR_ESD_HEADSTAILS;
		setup.push_back(value);
		return setup;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Set initial solution to create the neighbourhood from
	*/
	virtual void setInitialSolution(FuzzyFW::Solution *solution,
		FuzzyFW::Fitness *fitness, const FuzzyFW::SharedVars *svars);

	/*
	* Find the neighbours of the given solution
	* Returns the number of neighbours found
	*/
	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);


	/*
	* Accept a neighbour and generates the new solution using
	* the one given before as base
	*/
	virtual FuzzyFW::Fitness * evaluateNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars, const bool improvement = false);

	/*
	* If an estimator is used, an estimation can be used
	*/
	virtual FuzzyFW::Fitness * getEstimation(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);


	/*
	* Accept a neighbour and moves towards it
	*/
	virtual void acceptNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);

	/*
	* Discard a specific neighbour
	*/
	virtual void discardNeighbour(const unsigned int idx);

	/*
	* Sort all neighbours by their estimation
	*/
	virtual void sortByEstimation(const FuzzyFW::SharedVars *svars);



protected:
	/*
	* Auxiliar method to apply quick sort to the neighbours
	*/
	void quickSort(const int left, const int right, const FuzzyFW::SharedVars *svars);


	/*
	* Estimate the quality by means of head and tails
	*/
	virtual void estimateHeadsTails(const ProblemIJSP *problem, unsigned int idx);
};





//=============================================================================
//
//	Class NB_ParallelN1_ESDminFJSP
//
//=============================================================================
/**
* This class defines the neighbourhood structure known as N1 for minimum
* ESD maximization in FJSP. This neighbourhood is done by
* splitting the fuzzy graph in three parallel graphs. Then, the longest path
* in any of them from start to the end node of a job such that AI == AImin, is
* considered a critical path. N1 considers the reversal of all arcs that belong
* to at least one critical path
*
* @author jjpalacios
*
*/
class NB_ParallelN1_ESDminIJSP : public NB_ParallelN1_ESDavgIJSP {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Main constructor
	*/
	NB_ParallelN1_ESDminIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelN1_ESDavgIJSP(parameters) { }


	/*
	* Copy constructor
	*/
	NB_ParallelN1_ESDminIJSP(const NB_ParallelN1_ESDminIJSP & source)
		: NB_ParallelN1_ESDavgIJSP(source) { }


	/**
	* Loads the needed parameters
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		NB_ParallelN1_ESDavgIJSP::setup(parameters);
	}


	/*
	* Clone method
	*/
	virtual Neighbourhood * clone() const {
		return new NB_ParallelN1_ESDminIJSP(*this);
	}


	/*
	* Destructor
	*/
	~NB_ParallelN1_ESDminIJSP() { }



	//=========================================================================
	//		GET / SET METHODS
	//=========================================================================
public:
	/*
	* Name of the Neighbourhood structure
	*/
	virtual std::vector<std::string> getName() {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("ESDmin-N1");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_ESD_NONE;
		setup.push_back(value);
		return setup;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Find the neighbours of the given solution
	* Returns the number of neighbours found
	*/
	virtual unsigned int findNewNeighbours(const FuzzyFW::SharedVars *svars);


	/*
	* Accept a neighbour and generates the new solution using
	* the one given before as base
	*/
	virtual FuzzyFW::Fitness * evaluateNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars, const bool improvement = false);

protected:
	/*
	* Estimate the quality by means of head and tails
	*/
	virtual void estimateHeadsTails(const ProblemIJSP *problem, unsigned int idx);

	//bool checkColas(const FuzzyFW::SharedVars *svars);
};

}
