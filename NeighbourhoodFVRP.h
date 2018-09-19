/*
* NeighbourhoodFVRP.h
*
*  Created on: Nov 27, 2017
*      Author: jjpalacios
*/
#pragma once

#include "NeighbourFVRP.h"
#include "Neighbourhood.h"
#include "EncoderFVRP.h"


namespace FVRP {

// Creation parameters defined in this header file
#define NB_FVRP_ESTIMATOR_NONE "none"
#define NB_FVRP_ESTIMATOR_CONST "constant"


// Parameters for neighbourhood N1
#define NB_FVRP_N1_MINSIZE "neighbourhood.n1.min-size"
#define NB_FVRP_N1_MAXSIZE "neighbourhood.n1.max-size"
#define NB_FVRP_N1_MINSIZE_DEFAULT 0
#define NB_FVRP_N1_MAXSIZE_DEFAULT 2

// Paremeters for demand satisfaction
#define NB_FVRP_DEMAND_METRIC "neighbourhood.demand.metric"
#define NB_FVRP_DEMAND_THRESHD "neighbourhood.demand.threshold"

// Types of metric to use for checking the demand
#define NB_FVRP_DEMAND_NEC "necessity"
#define NB_FVRP_DEMAND_POS "possibility"
#define NB_FVRP_DEMAND_CRED "credibility"
#define NB_FVRP_DEMAND_EV "expected-value"



//=============================================================================
//
//	Class NB_Vidal_FVRP
//
//=============================================================================
/**
* This class defines the neighbourhood structure known as N1 for Makespan
* minimization in FJSP. This neighbourhood is done by splitting the fuzzy
* graph in three parallel graphs. Then, the longest path in any of them from
* start to end is called a critical path. N1 considers the reversal of all arcs
* that belong to at least one critical path
*
* @author jjpalacios
*
*/
class NB_Vidal_FVRP_FD : public FuzzyFW::Neighbourhood {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* Types of estimators for this neghbouhood
	*/
	enum Estimator { NONE, ESTIM_CONST };

	/*
	* Label for the estimator to use
	*/
	std::string estimatorLabel;

	/*
	* Estimator to use
	*/
	Estimator estimator;

	/*
	* Label for the minimum/maximum size of sequence
	*/
	std::string minSeqSizeLabel;
	std::string maxSeqSizeLabel;

	/*
	* Minimum/maximum size of sequence
	*/
	unsigned int minSeqSize;
	unsigned int maxSeqSize;


	/*
	* Name of the parameter with the type of metric for demand
	*/
	enum DemandType { FD_NECESSITY, FD_POSSIBILITY, FD_CREDIBILITY, FD_EV };
	const std::string metricDemandLabel;

	/*
	* Type of metric for demand
	*/
	DemandType metricDemand;

	/*
	* Name of the parameter with the type of metric for demand
	*/
	const std::string thresholdDemandLabel;

	/*
	* Type of metric for demand
	*/
	double demandThreshold;

	/*
	* Fuzzy Schedule to work with
	*/
	RouteFVRP *route;

	/*
	* Current fitness to work with
	*/
	FuzzyFW::FitnessTFN *currentFitness;

	/*
	* Array of neighbours generated
	*/
	std::vector<NeighbourFVRP_SegSwap *> neighbours;
	
	/*
	* Matrix of costs
	*/
	std::vector< std::vector<FuzzyFW::TFN> > durationMatrix;

	/*
	* Matrix of stock
	*/
	std::vector< std::vector<FuzzyFW::TFN> > stockMatrix;

	/*
	* Matrix of distances
	*/
	std::vector< std::vector<double> > distanceMatrix;


	/*
	* Segments. The position [i,j] contains the customer that is
	* reached from i, after j visits
	*/
	std::vector< std::vector<int> > segments;





	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Main constructor
	*/
	NB_Vidal_FVRP_FD(FuzzyFW::ParameterDB *parameters = NULL)
		: Neighbourhood(parameters), estimatorLabel(NEIGHBOURHOOD_ESTIMATOR),
		minSeqSizeLabel(NB_FVRP_N1_MINSIZE), minSeqSize(0),
		maxSeqSizeLabel(NB_FVRP_N1_MAXSIZE), maxSeqSize(-1),
		metricDemandLabel(NB_FVRP_DEMAND_METRIC), metricDemand(FD_EV),
		thresholdDemandLabel(NB_FVRP_DEMAND_THRESHD), demandThreshold(1.0),
		estimator(Estimator::ESTIM_CONST) { }


	/*
	* Copy constructor
	*/
	NB_Vidal_FVRP_FD(const NB_Vidal_FVRP_FD & source);


	/**
	* Loads the needed parameters
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);


	/*
	* Clone method
	*/
	virtual Neighbourhood * clone() const {
		return new NB_Vidal_FVRP_FD(*this);
	}


	/*
	* Destructor
	*/
	~NB_Vidal_FVRP_FD();



	//=========================================================================
	//		GET / SET METHODS
	//=========================================================================
public:
	/*
	* Gets the current solution
	*/
	virtual FuzzyFW::FullSolution getCurrentSolution() {
		FuzzyFW::FullSolution solution;
		solution.first = this->route;
		solution.second = this->currentFitness;
		return solution;
	}


	/*
	* Name of the Neighbourhood structure
	*/
	virtual std::vector<std::string> getName() {
		std::vector<std::string> setup;
		std::string value;
		setup.push_back("Cost-Vidal-FD");
		value = "Estimator:;";
		if (this->estimator == Estimator::NONE)
			value += NB_FVRP_ESTIMATOR_NONE;
		else if (this->estimator == Estimator::ESTIM_CONST)
			value += NB_FVRP_ESTIMATOR_CONST;
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
	* Generates the new solution using the one given before as base
	*/
	virtual FuzzyFW::Fitness * evaluateNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars, const bool improvement = false);

	/*
	* Accept a neighbour and moves towards it
	*/
	virtual void acceptNeighbour(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);


	/*
	* If an estimator is used, an estimation can be used
	*/
	virtual FuzzyFW::Fitness * getEstimation(const unsigned int idx,
		const FuzzyFW::SharedVars *svars);

	/*
	* Sort all neighbours by their estimation
	*/
	virtual void sortByEstimation(const FuzzyFW::SharedVars *svars);

	/*
	* Discard a specific neighbour. This deletes all the structures
	* created for that neighbour. That is: estimation and evaluation
	*/
	virtual void discardNeighbour(const unsigned int idx);

	/*
	* Gets a neighbour from the neighbourhood
	*/
	virtual FuzzyFW::Neighbour* getNeighbour(const unsigned int idx);



protected:
	/*
	* Auxiliar method to apply quick sort to the neighbours
	*/
	void quickSort(const int left, const int right, FuzzyFW::Random *rng);

	/*
	* Given a demand and the capacity of the vehicle,
	* indicates if the demand is under the capacity of the vehicle
	* with the specified "metric"
	*/
	double getDemandMetric(FuzzyFW::TFN demand, double capacity);
};





}
