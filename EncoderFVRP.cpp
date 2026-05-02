/*
 * EncoderFVRP.cpp
 *
 *  Created on: Nov 23, 2017
 */

#include "EncoderFVRP.h"

namespace FVRP {

//=============================================================================
//
//	Class EncoderFJSP_Order
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Encoding method  -----------------------------------------------------
void EncoderFVRP_Order::encode(FuzzyFW::Solution *solution,
	FuzzyFW::Individual *indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {

	RouteFVRP *route;
	FuzzyFW::IndividualArrayInt * intIndiv;
	std::vector<int> genotype, path;
	
	route = dynamic_cast<RouteFVRP *>(solution);
	if (route == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy VRP problems.";
		throw FVRPException("Encoding", errorMsg);
	}

	intIndiv = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (intIndiv == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "integer array individuals.";
		throw FVRPException("Encoding", errorMsg);
	}
	
	for (unsigned int v = 0; v < route->getNumberVehicles(); v++) {
		path = route->getRoute(v);
		for (size_t i = 0; i < path.size(); i++)
			genotype.push_back(path[i]);
	}

	intIndiv->updateGenotype(genotype);
}



//-----  Encoding method  -----------------------------------------------------
FuzzyFW::Individual * EncoderFVRP_Order::encode(FuzzyFW::Solution *solution,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {

	RouteFVRP *route;
	std::vector<int> genotype, path;

	route = dynamic_cast<RouteFVRP *>(solution);
	if (route == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy VRP problems.";
		throw FVRPException("Encoding", errorMsg);
	}

	for (unsigned int v = 0; v < route->getNumberVehicles(); v++) {
		path = route->getRoute(v);
		for (size_t i = 0; i < path.size(); i++)
			genotype.push_back(path[i]);
	}

	return new FuzzyFW::IndividualArrayInt(genotype);
}

}
