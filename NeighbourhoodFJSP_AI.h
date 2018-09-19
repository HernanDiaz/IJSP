/*
* NeighbourhoodFJSP_AI.h
*
*  Created on: Oct 11, 2017
*      Author: jjpalacios
*/
#pragma once

#include "NeighbourFJSP.h"
#include "Neighbourhood.h"


namespace FJSP {

// Creation parameters defined in this header file
#define NB_ESTIMATOR_AI_NONE "none"


//=============================================================================
//
//	Class NB_ParallelN1_AIavgFJSP
//
//=============================================================================
/**
* This class defines the neighbourhood structure known as N1 for average
* agreement index maximization in FJSP. This neighbourhood is done by
* splitting the fuzzy graph in three parallel graphs. Then, the longest path
* in any of them from start to the end node of a job such that AI<1, is
* considered a critical path. N1 considers the reversal of all arcs that belong
* to at least one critical path
*
* @author jjpalacios
*
*/
class NB_ParallelN1_AIavgFJSP : public FuzzyFW::Neighbourhood {
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
	ScheduleFJSP *schedule;

	/*
	* Current fitness to work with
	*/
	FuzzyFW::Fitness *currentFitness;

	/*
	* Array of neighbours generated
	*/
	std::vector<NeighbourFJSP_Arc *> neighbours;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Main constructor
	*/
	NB_ParallelN1_AIavgFJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: Neighbourhood(parameters), estimatorLabel(NEIGHBOURHOOD_ESTIMATOR),
		estimator(Estimator::NONE), schedule(NULL), currentFitness(NULL) { }


	/*
	* Copy constructor
	*/
	NB_ParallelN1_AIavgFJSP(const NB_ParallelN1_AIavgFJSP & source);


	/**
	* Loads the needed parameters
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);


	/*
	* Clone method
	*/
	virtual Neighbourhood * clone() const {
		return new NB_ParallelN1_AIavgFJSP(*this);
	}


	/*
	* Destructor
	*/
	~NB_ParallelN1_AIavgFJSP();



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
		setup.push_back("AIavg-N1");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_AI_NONE;
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
	void quickSort(const int left, const int right, FuzzyFW::Random *rng);
};





//=============================================================================
//
//	Class NB_ParallelN1_AIminFJSP
//
//=============================================================================
/**
* This class defines the neighbourhood structure known as N1 for minimum
* agreement index maximization in FJSP. This neighbourhood is done by
* splitting the fuzzy graph in three parallel graphs. Then, the longest path
* in any of them from start to the end node of a job such that AI == AImin, is
* considered a critical path. N1 considers the reversal of all arcs that belong
* to at least one critical path
*
* @author jjpalacios
*
*/
class NB_ParallelN1_AIminFJSP : public NB_ParallelN1_AIavgFJSP {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Main constructor
	*/
	NB_ParallelN1_AIminFJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelN1_AIavgFJSP(parameters) { }


	/*
	* Copy constructor
	*/
	NB_ParallelN1_AIminFJSP(const NB_ParallelN1_AIminFJSP & source)
		: NB_ParallelN1_AIavgFJSP(source) { }


	/**
	* Loads the needed parameters
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		NB_ParallelN1_AIavgFJSP::setup(parameters);
	}


	/*
	* Clone method
	*/
	virtual Neighbourhood * clone() const {
		return new NB_ParallelN1_AIminFJSP(*this);
	}


	/*
	* Destructor
	*/
	~NB_ParallelN1_AIminFJSP() { }



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
		setup.push_back("AImin-N1");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_AI_NONE;
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

	/*
	* If an estimator is used, an estimation can be used
	*/
	virtual FuzzyFW::Fitness * getEstimation(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);
};





//=============================================================================
//
//	Class NB_ParallelN1_AIminICAE
//
//=============================================================================
/**
* This class defines the neighbourhood structure known as N1 for minimum
* agreement index maximization in FJSP. This neighbourhood is done by
* splitting the fuzzy graph in three parallel graphs. Then, the longest path
* in any of them from start to the end node of a job such that AI == AImin, is
* considered a critical path. N1 considers the reversal of all arcs that belong
* to at least one critical path
*
* @author jjpalacios
*
*/
class NB_ParallelN1_AIminICAE : public NB_ParallelN1_AIminFJSP {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Main constructor
	*/
	NB_ParallelN1_AIminICAE(FuzzyFW::ParameterDB *parameters = NULL)
		: NB_ParallelN1_AIminFJSP(parameters) { }


	/*
	* Copy constructor
	*/
	NB_ParallelN1_AIminICAE(const NB_ParallelN1_AIminICAE & source)
		: NB_ParallelN1_AIminFJSP(source) { }


	/**
	* Loads the needed parameters
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters) {
		NB_ParallelN1_AIminFJSP::setup(parameters);
	}


	/*
	* Clone method
	*/
	virtual Neighbourhood * clone() const {
		return new NB_ParallelN1_AIminICAE(*this);
	}


	/*
	* Destructor
	*/
	~NB_ParallelN1_AIminICAE() { }



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
		setup.push_back("AImin-N1-ICAE");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_AI_NONE;
		setup.push_back(value);
		return setup;
	}

	/*
	* If an estimator is used, an estimation can be used
	*/
	virtual FuzzyFW::Fitness * getEstimation(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);




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
};

}
