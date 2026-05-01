/*
* SGS_FVRP_Split.cpp
*
*  Created on: October 31, 2017
*/

#include "SGS_FVRP_Split.h"

namespace FVRP {

//=============================================================================
//
//	Class SGS_Split_FD_Time
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_Split_FD_Time::SGS_Split_FD_Time(const FuzzyFW::ParameterDB *params)
	: metricDemandLabel(FVRP_SGS_DEMAND_METRIC), metricDemand(FD_EV),
	thresholdDemandLabel(FVRP_SGS_DEMAND_THRESHD), demandThreshold(1.0),
	SGS_FVRP(params) {
}


//=====  Copy constructor  ====================================================
SGS_Split_FD_Time::SGS_Split_FD_Time(const SGS_Split_FD_Time &source)
	: metricDemandLabel(source.metricDemandLabel),
	metricDemand(source.metricDemand),
	thresholdDemandLabel(source.thresholdDemandLabel),
	demandThreshold(source.demandThreshold),
	SGS_FVRP(source) { }


//=====  Setup method  ========================================================
void SGS_Split_FD_Time::setup(const FuzzyFW::ParameterDB *params) {
	SGS_FVRP::setup(params);

	// Load metric for feasibility on demand
	std::string value = params->getStringLower(this->metricDemandLabel);
	if (value == FVRP_DEMAND_NEC)
		this->metricDemand = DemandType::FD_NECESSITY;
	else if (value == FVRP_DEMAND_POS)
		this->metricDemand = DemandType::FD_POSSIBILITY;
	else if (value == FVRP_DEMAND_CRED)
		this->metricDemand = DemandType::FD_CREDIBILITY;
	else if (value == FVRP_DEMAND_EV)
		this->metricDemand = DemandType::FD_EV;
	else {
		std::string errorMsg = "Unkown value for parameter \'";
		errorMsg += this->metricDemandLabel + "\' or parameter not found.";
		throw new FVRPException("SGS", errorMsg);
	}

	this->demandThreshold = params->getDouble(this->thresholdDemandLabel, -1.0);
	if(this->demandThreshold < 0 && this->metricDemand != DemandType::FD_EV) {
		std::string errorMsg = "Parameter \'" + this->thresholdDemandLabel;
		errorMsg += "\' not found";
		throw new FVRPException("SGS", errorMsg);
	}
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Build plan  ==========================================================
RouteFVRP * SGS_Split_FD_Time::buildPlan(
	const FuzzyFW::SharedVars * const svars, std::vector<int> &order) {

	unsigned int currentVehicle = 0;
	FuzzyFW::TFN demand, routeCost;
	FuzzyFW::TFN::Compare compare =
		FuzzyFW::FitnessTFN::FitnessCompareStrategy;

	unsigned int dest;

	ProblemFVRP * fuzzyProb =
		dynamic_cast<ProblemFVRP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy VRP.";
		throw new FVRPException("SGS", errorMsg);
	}

	// Minimum cost to reah each node
	std::vector<FuzzyFW::TFN> minCost(fuzzyProb->getNumberCustomers(),
		FuzzyFW::TFN(Infd, Infd, Infd));

	// Vehicle serving each customer
	std::vector<unsigned int> split(fuzzyProb->getNumberCustomers());
	std::vector<unsigned int> splitMarks;


	if (this->isCreated)
		this->routePlan->reset();
	else {
		this->routePlan = new RouteFVRP(fuzzyProb);
		this->isCreated = true;
	}

	
	for (size_t i = 0; i < order.size(); i++) {
		routeCost = fuzzyProb->getTravelTime(0, order[i])
			+ fuzzyProb->getServiceTime(order[i])
			+ fuzzyProb->getTravelTime(0, order[i]);
		demand = fuzzyProb->getDemand(order[i]);

		dest = i + 1;
		while (dest < fuzzyProb->getNumberCustomers()) {
			routeCost = routeCost - fuzzyProb->getTravelTime(0, order[dest - 1])
				+ fuzzyProb->getTravelTime(order[dest - 1], order[dest])
				+ fuzzyProb->getServiceTime(order[dest])
				+ fuzzyProb->getTravelTime(0, order[dest]);
			demand = demand + fuzzyProb->getDemand(order[dest]);

			// Check the constraints
			if (compareDouble(getDemandMetric(demand, fuzzyProb->getMaxCapacity()),
				this->demandThreshold) > 0) {
				if (i == 0
					|| minCost[dest].isGreaterThan(minCost[i - 1] + routeCost, compare)) {
					minCost[dest] = routeCost;
					split[dest] = i;
				}
			}
			dest++;
		}
	}

	unsigned int pos = split.size() - 1;
	unsigned int vehicle = 0;
	FuzzyFW::TFN arrivalTFN;
	while (pos >= 0) {
		splitMarks.push_back(split[pos]);
		pos = split[pos] - 1;
	}

	pos = 0;
	for (size_t i = splitMarks.size() - 1; i >= 0; i--) {
		for (unsigned int j = pos; j < splitMarks[i]; j++) {
			this->addCustomer(fuzzyProb, order[j], vehicle);
		}
		pos = splitMarks[i];
		vehicle++;
	}

	return this->routePlan;
}



//=====  Schedule a task  =====================================================
FuzzyFW::TFN SGS_Split_FD_Time::addCustomer(ProblemFVRP *problem,
	const unsigned int customerId, const unsigned int vehicle) {

	FuzzyFW::TFN visitTime;
	unsigned int pred;

	// Find the predecessor
	pred = this->routePlan->getLastCustomer(vehicle);
	visitTime = this->routePlan->node[pred].arrivalTime
		+ problem->getServiceTime(pred)
		+ problem->getTravelTime(pred, customerId);

	this->routePlan->addCustomer(customerId, vehicle, visitTime, pred);
	return visitTime;
}


//=====  Get the demand metric  ===============================================
double SGS_Split_FD_Time::getDemandMetric(FuzzyFW::TFN demand, double capacity) {
	if (this->metricDemand == DemandType::FD_NECESSITY)
		return demand.necessity(capacity);
	if (this->metricDemand == DemandType::FD_POSSIBILITY)
		return demand.possibility(capacity);
	if (this->metricDemand == DemandType::FD_CREDIBILITY)
		return demand.credibility(capacity);
	if (this->metricDemand == DemandType::FD_EV)
		return compareDouble(demand.expectedValue(), capacity) <= 0;
	return -1.0;
}





//=============================================================================
//
//	Class SGS_Split_FD_Distance
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Build plan  ==========================================================
RouteFVRP * SGS_Split_FD_Distance::buildPlan(
	const FuzzyFW::SharedVars * const svars, std::vector<int> &order) {

	unsigned int currentVehicle = 0;
	double routeDist;
	FuzzyFW::TFN demand;

	unsigned int dest;

	ProblemFVRP * fuzzyProb =
		dynamic_cast<ProblemFVRP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy VRP.";
		throw new FVRPException("SGS", errorMsg);
	}

	// Minimum cost to reah each node
	std::vector<double> minCost(fuzzyProb->getNumberCustomers(), Infd);

	// Vehicle serving each customer
	std::vector<unsigned int> split(fuzzyProb->getNumberCustomers());
	std::vector<unsigned int> splitMarks;


	if (this->isCreated)
		this->routePlan->reset();
	else {
		this->routePlan = new RouteFVRP(fuzzyProb);
		this->isCreated = true;
	}


	for (size_t i = 0; i < order.size(); i++) {
		routeDist = 2 * fuzzyProb->getDistance(0, order[i]);
		demand = fuzzyProb->getDemand(order[i]);

		dest = i + 1;
		while (dest < fuzzyProb->getNumberCustomers()) {
			demand = demand + fuzzyProb->getDemand(order[dest]);
			routeDist = routeDist - fuzzyProb->getDistance(0, order[dest - 1])
				+ fuzzyProb->getDistance(order[dest - 1], order[dest])
				+ fuzzyProb->getDistance(0, order[dest]);

			// Check the constraints (no time windows yet)
			if (compareDouble(getDemandMetric(demand, fuzzyProb->getMaxCapacity()),
				this->demandThreshold) > 0) {
				if (i == 0
					|| compareDouble(minCost[dest], minCost[i - 1] + routeDist) > 0) {
					minCost[dest] = routeDist;
					split[dest] = i;
				}
			}
			dest++;
		}
	}

	unsigned int pos = split.size() - 1;
	unsigned int vehicle = 0;
	while (pos >= 0) {
		splitMarks.push_back(split[pos]);
		pos = split[pos] - 1;
	}

	pos = 0;
	for (size_t i = splitMarks.size() - 1; i >= 0; i--) {
		for (unsigned int j = pos; j < splitMarks[i]; j++) {
			this->addCustomer(fuzzyProb, order[j], vehicle);
		}
		pos = splitMarks[i];
		vehicle++;
	}

	return this->routePlan;
}



}
