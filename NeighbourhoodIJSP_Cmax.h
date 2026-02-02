/*
* NeighbourhoodIJSP.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "NeighbourIJSP.h"
#include "Neighbourhood.h"
//#include "DebugFileWriter.h"

namespace IJSP {

// Creation parameters defined in this header file
#define NB_ESTIMATOR_NONE "none"
#define NB_ESTIMATOR_HEADSTAILS "heads&tails"


//=============================================================================
//
//	Class NB_ParallelN1_MakespanIJSP
//
//=============================================================================
/**
* This class defines the neighbourhood structure known as N1 for Makespan
* minimization in IJSP. This neighbourhood is done by splitting the fuzzy
* graph in three parallel graphs. Then, the longest path in any of them from
* start to end is called a critical path. N1 considers the reversal of all arcs
* that belong to at least one critical path
*
* @author hdiaz
*
*/
class NB_ParallelN1_MakespanIJSP : public FuzzyFW::Neighbourhood {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	//DebugFileWriter debug;
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
	FuzzyFW::FitnessInterval *currentFitness;

	/*
	* Array of neighbours generated
	*/
	std::vector<NeighbourIJSP_Arc *> neighbours;

	/*
	* Tails of every task in the schedule
	*/
	std::vector<FuzzyFW::Interval> tails;





	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Main constructor
	*/
	NB_ParallelN1_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: Neighbourhood(parameters), estimatorLabel(NEIGHBOURHOOD_ESTIMATOR),
		estimator(Estimator::NONE), schedule(NULL), currentFitness(NULL) {
	//	debug.open("Debug");
	}


	/*
	* Copy constructor
	*/
	NB_ParallelN1_MakespanIJSP(const NB_ParallelN1_MakespanIJSP & source);


	/**
	* Loads the needed parameters
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);


	/*
	* Clone method
	*/
	virtual Neighbourhood * clone() const {
		return new NB_ParallelN1_MakespanIJSP(*this);
	}


	/*
	* Destructor
	*/
	~NB_ParallelN1_MakespanIJSP();



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
	virtual std::vector<std::string> getName() {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Makespan-N1");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_NONE;
		else if (this->estimator == Estimator::ESTIM_HEADTAILS)
			value += NB_ESTIMATOR_HEADSTAILS;
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
	* Estimate the quality by means of head and tails
	*/
	virtual void estimateHeadsTails(const unsigned int idx);

	/*
	* Auxiliar method to apply quick sort to the neighbours
	*/
	void quickSort(const int left, const int right, const FuzzyFW::SharedVars *svars);
};



//=============================================================================
//
//	Class NB_ParallelN2_MakespanIJSP
//
//=============================================================================
/**
* This class defines the neighbourhood structure known as N2 for Makespan
* minimization in IJSP. This neighbourhood is done by splitting the fuzzy
* graph in two parallel graphs. Then, the longest path in any of them from
* start to end is called a critical path. N2 considers the reversal of the arcs
* that belong to the extremes of a  critical block where the task share the same machine 
* and belong to the same critical path
*
* @author hdiaz
*
*/
class NB_ParallelN2_MakespanIJSP : public FuzzyFW::Neighbourhood {
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
	FuzzyFW::FitnessInterval *currentFitness;

	/*
	* Array of neighbours generated
	*/
	std::vector<NeighbourIJSP_Arc *> neighbours;

	/*
	* Tails of every task in the schedule
	*/
	std::vector<FuzzyFW::Interval> tails;





	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Main constructor
	*/
	NB_ParallelN2_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: Neighbourhood(parameters), estimatorLabel(NEIGHBOURHOOD_ESTIMATOR),
		estimator(Estimator::NONE), schedule(NULL), currentFitness(NULL) { }


	/*
	* Copy constructor
	*/
	NB_ParallelN2_MakespanIJSP(const NB_ParallelN2_MakespanIJSP & source);


	/**
	* Loads the needed parameters
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);


	/*
	* Clone method
	*/
	virtual Neighbourhood * clone() const {
		return new NB_ParallelN2_MakespanIJSP(*this);
	}


	/*
	* Destructor
	*/
	~NB_ParallelN2_MakespanIJSP();



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
	virtual std::vector<std::string> getName() {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Makespan-N2");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_NONE;
		else if (this->estimator == Estimator::ESTIM_HEADTAILS)
			value += NB_ESTIMATOR_HEADSTAILS;
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
	* Estimate the quality by means of head and tails
	*/
	virtual void estimateHeadsTails(const unsigned int idx);

	/*
	* Auxiliar method to apply quick sort to the neighbours
	*/
	void quickSort(const int left, const int right, const FuzzyFW::SharedVars *svars);
};


//=============================================================================
//
//	Class NB_ParallelN2_MakespanIJSP
//
//=============================================================================
/**
* This class defines the neighbourhood structure known as N3 for Makespan
* minimization in IJSP. This neighbourhood is done by splitting the fuzzy
* graph in two parallel graphs. Then, the longest path in any of them from
* start to end is called a critical path. N2 considers the reversal of the arcs
* that belong to the extremes of a  critical block where the task share the same machine
* and belong to the same critical path
*
* @author hdiaz
*
*/
class NB_ParallelN3_MakespanIJSP : public FuzzyFW::Neighbourhood {
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
	FuzzyFW::FitnessInterval *currentFitness;

	/*
	* Array of neighbours generated
	*/
	std::vector<NeighbourIJSP_Arc *> neighbours;

	/*
	* Tails of every task in the schedule
	*/
	std::vector<FuzzyFW::Interval> tails;





	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Main constructor
	*/
	NB_ParallelN3_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: Neighbourhood(parameters), estimatorLabel(NEIGHBOURHOOD_ESTIMATOR),
		estimator(Estimator::NONE), schedule(NULL), currentFitness(NULL) { }


	/*
	* Copy constructor
	*/
	NB_ParallelN3_MakespanIJSP(const NB_ParallelN3_MakespanIJSP & source);


	/**
	* Loads the needed parameters
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);


	/*
	* Clone method
	*/
	virtual Neighbourhood * clone() const {
		return new NB_ParallelN3_MakespanIJSP(*this);
	}


	/*
	* Destructor
	*/
	~NB_ParallelN3_MakespanIJSP();



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
	virtual std::vector<std::string> getName() {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Makespan-N2");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_NONE;
		else if (this->estimator == Estimator::ESTIM_HEADTAILS)
			value += NB_ESTIMATOR_HEADSTAILS;
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
	* Estimate the quality by means of head and tails
	*/
	virtual void estimateHeadsTails(const unsigned int idx);

	void addNeighbour(const unsigned int x, const unsigned int y, const unsigned int z = 0, const unsigned int tipo = 0);

	/*
	* Auxiliar method to apply quick sort to the neighbours
	*/
	void quickSort(const int left, const int right, const FuzzyFW::SharedVars *svars);
};



//=============================================================================
//
//	Class NB_ParallelNH_MakespanIJSP
//
//=============================================================================
/**
* This class defines the neighbourhood structure known as N2 for Makespan
* minimization in IJSP. This neighbourhood is done by splitting the fuzzy
* graph in three parallel graphs. Then, the longest path in any of them from
* start to end is called a critical path. N2 considers the reversal of the arcs
* that belong to the extremes of a  critical block where the task share the same machine
* and belong to the same critical path
*
* @author hdiaz
*
*/
class NB_ParallelNH_MakespanIJSP : public FuzzyFW::Neighbourhood {
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
	FuzzyFW::FitnessInterval *currentFitness;

	/*
	* Array of neighbours generated
	*/
	std::vector<NeighbourIJSP_Arc *> neighbours;

	/*
	* Tails of every task in the schedule
	*/
	std::vector<FuzzyFW::Interval> tails;





	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Main constructor
	*/
	NB_ParallelNH_MakespanIJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: Neighbourhood(parameters), estimatorLabel(NEIGHBOURHOOD_ESTIMATOR),
		estimator(Estimator::NONE), schedule(NULL), currentFitness(NULL) { }


	/*
	* Copy constructor
	*/
	NB_ParallelNH_MakespanIJSP(const NB_ParallelNH_MakespanIJSP & source);


	/**
	* Loads the needed parameters
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);


	/*
	* Clone method
	*/
	virtual Neighbourhood * clone() const {
		return new NB_ParallelNH_MakespanIJSP(*this);
	}


	/*
	* Destructor
	*/
	~NB_ParallelNH_MakespanIJSP();



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
	virtual std::vector<std::string> getName() {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Makespan-NH");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_ESTIMATOR_NONE;
		else if (this->estimator == Estimator::ESTIM_HEADTAILS)
			value += NB_ESTIMATOR_HEADSTAILS;
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
	* Estimate the quality by means of head and tails
	*/
	virtual void estimateHeadsTails(const unsigned int idx);

	/*
	* Auxiliar method to apply quick sort to the neighbours
	*/
	void quickSort(const int left, const int right, const FuzzyFW::SharedVars *svars);
};

}
