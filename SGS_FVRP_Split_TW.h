/*
* SGS_FVRP_Split.h
*
*  Created on: June 1, 2017
*/

#pragma once

#include "SGS_FVRP.h"

namespace FVRP {

/*
* Parameters:
*	Metric to verify if demand is fulfiled
*	Threshold that the metric must meet
*/
#define FVRP_SGS_DEMAND_METRIC "sgs.split.demand.metric"
#define FVRP_SGS_DEMAND_THRESHD "sgs.split.demand.threshold"

// Types of metric to use for checking the demand
#define FVRP_DEMAND_NEC "necessity"
#define FVRP_DEMAND_POS "possibility"
#define FVRP_DEMAND_CRED "credibility"
#define FVRP_DEMAND_EV "expected-value"



//=============================================================================
//
//	Class SGS_Split_TW_FD_Time
//
//=============================================================================
/**
* The split method for creating a plan from a sequence of customers is
* very extended in the literature of the VRP, and thus is applied here.
* In this class, all the variables are considered to be crisp except the
* demand, which is fuzzy. The SGS takes into account also time windows, in case
* they are present.
* To check feasibility of routes, the vehicle must meet the demand of each
* customer, but since it is fuzzy, instead must reach a specific threshold
* for a user specified metric: Possibility, Necessity or Credibility.
* For a solution to be feasible, the vehicle must have a metric value
* higher than the threshold.
* An additional, more simplistic, measure is added: Expected value.
* In this case, feasibility is achieved if the expcted value of the demand is
* below the expected capacity of the vehicle at the moment of visiting that
* customer
* Finally, this split version is focus on optimising the total travel time
* of the solution, and not the distance itself.
*
* Finally, it is worth mention that the only source of uncertainty is in the
* demand, and therefore all the other values are assumed to be well defined.
*
*
*/

class SGS_Split_TW_FD_Time : public SGS_FVRP
{
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	enum DemandType { FD_NECESSITY, FD_POSSIBILITY, FD_CREDIBILITY, FD_EV };

	/*
	* Name of the parameter with the type of metric for demand
	*/
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



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	SGS_Split_TW_FD_Time(const FuzzyFW::ParameterDB *params = NULL);


	/*
	* Copy constructor
	*/
	SGS_Split_TW_FD_Time(const SGS_Split_TW_FD_Time &source);


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_Split_TW_FD_Time * clone() const {
		return new SGS_Split_TW_FD_Time(*this);
	}


	/*
	* Loads the Maximum strategy to follow
	*/
	virtual void setup(const FuzzyFW::ParameterDB *params);


	/*
	* Destructor
	*/
	virtual ~SGS_Split_TW_FD_Time() { }  // No pointers, no destruction



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the name and configuration of the SGS
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Split-TW-FD-Time");
		if (this->metricDemand == FD_NECESSITY)
			setup.push_back(";Demand Metric:;" + std::string(FVRP_DEMAND_NEC));
		else if (this->metricDemand == FD_POSSIBILITY)
			setup.push_back(";Demand Metric:;" + std::string(FVRP_DEMAND_POS));
		else if (this->metricDemand == FD_CREDIBILITY)
			setup.push_back(";Demand Metric:;" + std::string(FVRP_DEMAND_CRED));
		else if (this->metricDemand == FD_EV)
			setup.push_back(";Demand Metric:;" + std::string(FVRP_DEMAND_EV));
		if (this->metricDemand != FD_EV)
			setup.push_back(";Demand Threshold:;" +
				valueToString(this->demandThreshold));
		return setup;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Takes an order of operations and creates a schedule from them by
	* appending them at "the end" of the schedule. The method requires the
	* shared variables to have access to the problem to solve
	*/
	virtual RouteFVRP * buildPlan(
		const FuzzyFW::SharedVars * const svars, std::vector<int> &order);



protected:
	/*
	* Schedules a specific task just after the last task scheduled in its
	* required machine and the last task of its job
	*/
	virtual FuzzyFW::TFN addCustomer(ProblemFVRP *problem, 
		const unsigned int customerId, const unsigned int vehicle);

	/*
	* Given a demand and the capacity of the vehicle, 
	* indicates if the demand is under the capacity of the vehicle
	* with the specified "metric"
	*/
	double getDemandMetric(FuzzyFW::TFN demand, double capacity);

	/*
	* Given a visit time and a time window, indicates the minimum visit time which
	* is larger than the current one, that would be above a given "metric" threshold.
	* Returns -1 if it is not possible
	*/
	double getVisitTime(double visitTime, const FuzzyFW::TimeWindow *tw);


	/*
	* Get the crisp value associated to a variable stored as TFN
	*/
	double getCrispValue(FuzzyFW::TFN value) {
		return value.b;
	}
};





//=============================================================================
//
//	Class SGS_Split_TW_FD_Distance
//
//=============================================================================
/**
* The split method for creating a plan from a sequence of customers is
* very extended in the literature of the VRP, and thus is applied here.
* In this class, all the variables are considered to be crisp except the
* demand, which is fuzzy. The SGS takes into account also time windows, in case
* they are present.
* To check feasibility of routes, the vehicle must meet the demand of each
* customer, but since it is fuzzy, instead must reach a specific threshold
* for a user specified metric: Possibility, Necessity or Credibility.
* For a solution to be feasible, the vehicle must have a metric value
* higher than the threshold.
* An additional, more simplistic, measure is added: Expected value.
* In this case, feasibility is achieved if the expcted value of the demand is
* below the expected capacity of the vehicle at the moment of visiting that
* customer
* Finally, this split version is focus on optimising the total distance,
* indpendently of the travel times or service times
*
*
*/

class SGS_Split_TW_FD_Distance : public SGS_Split_TW_FD_Time
{
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	SGS_Split_TW_FD_Distance(const FuzzyFW::ParameterDB *params = NULL)
		: SGS_Split_TW_FD_Time(params) { }


	/*
	* Copy constructor
	*/
	SGS_Split_TW_FD_Distance(const SGS_Split_TW_FD_Distance &source)
		: SGS_Split_TW_FD_Time(source) { }


	/*
	* Clone method to replicate inherited instances
	*/
	virtual SGS_Split_TW_FD_Distance * clone() const {
		return new SGS_Split_TW_FD_Distance(*this);
	}


	/*
	* Loads the Maximum strategy to follow
	*/
	virtual void setup(const FuzzyFW::ParameterDB *params) {
		SGS_Split_TW_FD_Time::setup(params);
	}


	/*
	* Destructor
	*/
	virtual ~SGS_Split_TW_FD_Distance() { }  // No pointers, no destruction



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/*
	* Get the name and configuration of the SGS
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Split-TW-FD-Distance");
		if (this->metricDemand == FD_NECESSITY)
			setup.push_back(";Demand Metric:;" + std::string(FVRP_DEMAND_NEC));
		else if (this->metricDemand == FD_POSSIBILITY)
			setup.push_back(";Demand Metric:;" + std::string(FVRP_DEMAND_POS));
		else if (this->metricDemand == FD_CREDIBILITY)
			setup.push_back(";Demand Metric:;" + std::string(FVRP_DEMAND_CRED));
		else if (this->metricDemand == FD_EV)
			setup.push_back(";Demand Metric:;" + std::string(FVRP_DEMAND_EV));
		if (this->metricDemand != FD_EV)
			setup.push_back(";Demand Threshold:;" +
				valueToString(this->demandThreshold));
		return setup;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Takes an order of operations and creates a schedule from them by
	* appending them at "the end" of the schedule. The method requires the
	* shared variables to have access to the problem to solve
	*/
	virtual RouteFVRP * buildPlan(
		const FuzzyFW::SharedVars * const svars, std::vector<int> &order);
};

}
