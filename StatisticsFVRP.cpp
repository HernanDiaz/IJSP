/*
* StatisticsFVRP.cpp
*
*  Created on: Nec 23, 2017
*/

#include "StatisticsFVRP.h"

namespace FVRP {

//=============================================================================
//
//	Class StatisticsTimeCost
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsTimeCost::StatisticsTimeCost(STAT_TYPE flag)
	: Statistics(flag) {
	FuzzyFW::ParameterDB *newParams = new FuzzyFW::ParameterDB();
	newParams->setParameter(FUZZYFW_EVALUATION_LAMARCK, "NO");
	this->evaluation = new EvaluationFVRP_TimeCost(newParams);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsTimeCost::getValue(
	const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	FuzzyFW::Fitness *fitness, *best, *worst;
	double sum, solution;
	best = worst = NULL;

	for (unsigned int ind = 0; ind < population->size(); ind++) {
		fitness = 
			this->evaluation->evaluate(svars, population->getIndividual(ind));
		if (ind == 0) {
			best = worst = fitness;
			sum = fitness->toDouble();
		}
		else {
			sum += fitness->toDouble();
			if (fitness->isBetterThan(best)) {
				if (best != worst)
					delete best;
				best = fitness;
			}
			else if (fitness->isWorseThan(worst)) {
				if (best != worst)
					delete worst;
				worst = fitness;
			}
			else
				delete fitness;
		}
	}
	if (this->type == STAT_BEST)
		solution = best->toDouble();
	else if (this->type == STAT_WORST)
		solution = worst->toDouble();
	else if (this->type == STAT_AVG)
		solution = sum / population->size();
	if (best != worst)
		delete best;
	delete worst;

	return solution;
}





//=============================================================================
//
//	Class StatisticsDemandMinNec
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsDemandMinNec::StatisticsDemandMinNec(STAT_TYPE flag)
	: Statistics(flag), threshold(-1.0) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsDemandMinNec::getValue(
	const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	RouteFVRP * plan;
	FuzzyFW::TFN demand, stock;
	unsigned int customer;
	unsigned int capacity;
	double necessity, minNec;
	double best, worst, sum;
	
	// Obtain the threshold used for the evaluation
	if (this->threshold < 0) {
		this->threshold = svars->parameters->getDouble(FVRP_SGS_DEMAND_THRESHD);
		if (this->threshold < 0) {
			std::string errorMsg = "A threshold must be defined in the SGS";
			errorMsg += " to use this statistic value.";
			throw FVRPException("Statistics", errorMsg);
		}
	}

	ProblemFVRP * fuzzyProb =
		dynamic_cast<ProblemFVRP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy VRP.";
		throw FVRPException("SGS", errorMsg);
	}
	capacity = fuzzyProb->getMaxCapacity();

	
	// Calculate the minimum Demand Neccessity
	for (unsigned int ind = 0; ind < population->size(); ind++) {
		if (!population->getIndividual(ind)->isPhenotypeUpdated()) {
			std::string errorMsg = "Statistics cannot be applied";
			errorMsg += " if there are non evaluated individuals.";
			throw FVRPException("Statistics",errorMsg);
		}

		plan = dynamic_cast<RouteFVRP *>
			(population->getIndividual(ind)->getPhenotype());
		if (plan == NULL) {
			std::string errorMsg = "This statistic can be only computed for.";
			errorMsg += " Fuzzy VRP problems.";
			throw FVRPException("Statistics", errorMsg);
		}

		// The minimum neccessity is always in the last customer of the route
		for (unsigned int v = 0; v < plan->getNumberVehicles(); v++) {
			customer = plan->getLastCustomer(v);
			stock = plan->node[customer].stockSpent
				+ plan->node[customer].cst->demand;

			necessity = stock.necessity(capacity);
			if (v == 0 || necessity < minNec)
				minNec = necessity;
		}

		// Calculate statistical values
		if (ind == 0) {
			best = worst = sum = minNec;
		}
		else {
			sum += minNec;
			if (minNec > best) {
				best = minNec;
			}
			else if (minNec < worst) {
				worst = minNec;
			}
		}
	}

	if (this->type == STAT_BEST)
		return best;
	if (this->type == STAT_WORST)
		return worst;
	if (this->type == STAT_AVG)
		return sum / population->size();
	return -1;
}





//=============================================================================
//
//	Class StatisticsDemandMinPos
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsDemandMinPos::StatisticsDemandMinPos(STAT_TYPE flag)
	: Statistics(flag), threshold(-1.0) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsDemandMinPos::getValue(
	const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	RouteFVRP * plan;
	FuzzyFW::TFN demand, stock;
	unsigned int customer;
	unsigned int capacity;
	double possibility, minPos;
	double best, worst, sum;

	// Obtain the threshold used for the evaluation
	if (this->threshold < 0) {
		this->threshold = svars->parameters->getDouble(FVRP_SGS_DEMAND_THRESHD);
		if (this->threshold < 0) {
			std::string errorMsg = "A threshold must be defined in the SGS";
			errorMsg += " to use this statistic value.";
			throw FVRPException("Statistics", errorMsg);
		}
	}

	ProblemFVRP * fuzzyProb =
		dynamic_cast<ProblemFVRP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy VRP.";
		throw FVRPException("SGS", errorMsg);
	}
	capacity = fuzzyProb->getMaxCapacity();


	// Calculate the minimum Demand Neccessity
	for (unsigned int ind = 0; ind < population->size(); ind++) {
		if (!population->getIndividual(ind)->isPhenotypeUpdated()) {
			std::string errorMsg = "Statistics cannot be applied";
			errorMsg += " if there are non evaluated individuals.";
			throw FVRPException("Statistics", errorMsg);
		}

		plan = dynamic_cast<RouteFVRP *>
			(population->getIndividual(ind)->getPhenotype());
		if (plan == NULL) {
			std::string errorMsg = "This statistic can be only computed for.";
			errorMsg += " Fuzzy VRP problems.";
			throw FVRPException("Statistics", errorMsg);
		}

		// The minimum neccessity is always in the last customer of the route
		for (unsigned int v = 0; v < plan->getNumberVehicles(); v++) {
			customer = plan->getLastCustomer(v);
			stock = plan->node[customer].stockSpent
				+ plan->node[customer].cst->demand;

			possibility = stock.possibility(capacity);
			if (v == 0 || possibility < minPos)
				minPos = possibility;
		}

		// Calculate statistical values
		if (ind == 0) {
			best = worst = sum = minPos;
		}
		else {
			sum += minPos;
			if (minPos > best) {
				best = minPos;
			}
			else if (minPos < worst) {
				worst = minPos;
			}
		}
	}

	if (this->type == STAT_BEST)
		return best;
	if (this->type == STAT_WORST)
		return worst;
	if (this->type == STAT_AVG)
		return sum / population->size();
	return -1;
}





//=============================================================================
//
//	Class StatisticsDemandMinCred
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsDemandMinCred::StatisticsDemandMinCred(STAT_TYPE flag)
	: Statistics(flag), threshold(-1.0) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsDemandMinCred::getValue(
	const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	RouteFVRP * plan;
	FuzzyFW::TFN demand, stock;
	unsigned int customer;
	unsigned int capacity;
	double credibility, minCred;
	double best, worst, sum;

	// Obtain the threshold used for the evaluation
	if (this->threshold < 0) {
		this->threshold = svars->parameters->getDouble(FVRP_SGS_DEMAND_THRESHD);
		if (this->threshold < 0) {
			std::string errorMsg = "A threshold must be defined in the SGS";
			errorMsg += " to use this statistic value.";
			throw FVRPException("Statistics", errorMsg);
		}
	}

	ProblemFVRP * fuzzyProb =
		dynamic_cast<ProblemFVRP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy VRP.";
		throw FVRPException("SGS", errorMsg);
	}
	capacity = fuzzyProb->getMaxCapacity();


	// Calculate the minimum Demand Neccessity
	for (unsigned int ind = 0; ind < population->size(); ind++) {
		if (!population->getIndividual(ind)->isPhenotypeUpdated()) {
			std::string errorMsg = "Statistics cannot be applied";
			errorMsg += " if there are non evaluated individuals.";
			throw FVRPException("Statistics", errorMsg);
		}

		plan = dynamic_cast<RouteFVRP *>
			(population->getIndividual(ind)->getPhenotype());
		if (plan == NULL) {
			std::string errorMsg = "This statistic can be only computed for.";
			errorMsg += " Fuzzy VRP problems.";
			throw FVRPException("Statistics", errorMsg);
		}

		// The minimum neccessity is always in the last customer of the route
		for (unsigned int v = 0; v < plan->getNumberVehicles(); v++) {
			customer = plan->getLastCustomer(v);
			stock = plan->node[customer].stockSpent
				+ plan->node[customer].cst->demand;

			credibility = stock.credibility(capacity);
			if (v == 0 || credibility < minCred)
				minCred = credibility;
		}

		// Calculate statistical values
		if (ind == 0) {
			best = worst = sum = minCred;
		}
		else {
			sum += minCred;
			if (minCred > best) {
				best = minCred;
			}
			else if (minCred < worst) {
				worst = minCred;
			}
		}
	}

	if (this->type == STAT_BEST)
		return best;
	if (this->type == STAT_WORST)
		return worst;
	if (this->type == STAT_AVG)
		return sum / population->size();
	return -1;
}



}
