/*
* EvaluationFVRP.cpp
*
*  Created on: Nov 23, 2017
*      Author: Juan Jose Palacios
*/

#include "EvaluationFVRP.h"

namespace FVRP {


//=============================================================================
//
//	Class EvaluationFVRP_TimeCost
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
EvaluationFVRP_TimeCost::EvaluationFVRP_TimeCost(
	FuzzyFW::ParameterDB *parameters)
	: compareLabel(FVRP_EVALUATION_COMPARE), tfnCompare(FuzzyFW::TFN::C_EV),
	Evaluation(parameters) {
	FVRPClassRegister::registerClasses();
}


//-----  Copy constructor  ----------------------------------------------------
EvaluationFVRP_TimeCost::EvaluationFVRP_TimeCost(
	const EvaluationFVRP_TimeCost & source)
	: compareLabel(source.compareLabel), tfnCompare(source.tfnCompare),
	Evaluation(source) { }



//-----  Setup method  --------------------------------------------------------
void EvaluationFVRP_TimeCost::setup(FuzzyFW::ParameterDB *parameters) {
	Evaluation::setup(parameters);

	std::string compareName, maxName;

	// Load comparison strategy parameter
	compareName = parameters->getString(this->compareLabel);
	if (compareName.length() == 0) {
		std::string errorMsg = this->compareLabel + " parameter not found.";
		throw new FVRPException("Evaluation", errorMsg);
	}
	this->tfnCompare = FuzzyFW::TFN::getComparison(compareName);
	if (this->tfnCompare == FuzzyFW::TFN::C_Err) {
		std::string errorMsg = "Invalid value for parameter ";
		errorMsg += "\'" + this->compareLabel + "\': \'";
		errorMsg += compareName + "\'";
		throw new FVRPException("Evaluation", errorMsg);
	}
	FuzzyFW::FitnessTFN::FitnessCompareStrategy = this->tfnCompare;
}



//=============================================================================
//		METHODS
//=============================================================================
//----- Get Objective Function  -----------------------------------------------
FuzzyFW::Objective * EvaluationFVRP_TimeCost::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	RouteFVRP * route;
	ProblemFVRP *fuzzyProb;
	FuzzyFW::TFN cost = FuzzyFW::TFN(0, 0, 0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	route = dynamic_cast<RouteFVRP *>(solution);
	if (route == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy VRP Problems.";
		throw new FVRPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemFVRP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FVRPException("Ealuation", errorMsg);
	}

	// Compute the travel cost
	for (unsigned int v = 0; v < route->getNumberVehicles(); v++)
		cost = cost + route->getRouteTime(v);

	return new FuzzyFW::FitnessTFN(cost, false);
}


//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationFVRP_TimeCost::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	RouteFVRP * route;
	ProblemFVRP *fuzzyProb;
	FuzzyFW::TFN cost = FuzzyFW::TFN(0, 0, 0);

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	route = dynamic_cast<RouteFVRP *>(solution);
	if (route == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy VRP Problems.";
		throw new FVRPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemFVRP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FVRPException("Ealuation", errorMsg);
	}

	// Compute the travel cost
	for (unsigned int v = 0; v < route->getNumberVehicles(); v++)
		cost = cost + route->getRouteTime(v);

	if (this->lamarckism)
		svars->encoder->encode(route, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(route->clone());
	return new FuzzyFW::FitnessTFN(cost, false);
}





//=============================================================================
//
//	Class EvaluationFVRP_CTW_TimeCost
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Setup method  --------------------------------------------------------
void EvaluationFVRP_CTW_TimeCost::setup(FuzzyFW::ParameterDB *parameters) {
	EvaluationFVRP_TimeCost::setup(parameters);

	// Load penalty value for time warps
	penalty = parameters->getDouble(this->penaltyLabel, -1.0);
	if (penalty < 0) {
		std::string errorMsg = this->penaltyLabel + " parameter not found.";
		throw new FVRPException("Evaluation", errorMsg);
	}
}



//=============================================================================
//		METHODS
//=============================================================================
//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationFVRP_CTW_TimeCost::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	RouteFVRP * route;
	ProblemFVRP *fuzzyProb;
	FuzzyFW::TFN cost = FuzzyFW::TFN(0, 0, 0);
	const FuzzyFW::TimeWindowCrisp *tw;
	double sumWarp, warp, arrival;
	unsigned int customer;

	// Evaluate the individual to find the phenotype
	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	route = dynamic_cast<RouteFVRP *>(solution);
	if (route == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Fuzzy VRP Problems.";
		throw new FVRPException("Evaluation", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemFVRP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FVRPException("Ealuation", errorMsg);
	}

	// Compute the travel cost
	for (unsigned int v = 0; v < route->getNumberVehicles(); v++)
		cost = cost + route->getRouteTime(v);

	// Penalize the fitness if the TWs are not met
	sumWarp = 0.0;
	for (unsigned int v = 0; v < route->getNumberVehicles(); v++) {
		customer = route->getFirstCustomer(v);

		warp = 0.0;
		while (customer != 0) {
			tw = dynamic_cast<const FuzzyFW::TimeWindowCrisp *>
				(fuzzyProb->getTimeWindow(customer));
			arrival = route->node[customer].arrivalTime.expectedValue()
				- warp;
			warp += std::max(0.0, arrival - tw->lateTime);
			customer = route->node[customer].succ;
		}
		sumWarp += warp;
	}

	sumWarp *= this->penalty;
	cost = cost + FuzzyFW::TFN(sumWarp, sumWarp, sumWarp);

	if (this->lamarckism)
		svars->encoder->encode(route, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(route->clone());
	return new FuzzyFW::FitnessTFN(cost, false);
}


}
