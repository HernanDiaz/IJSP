/*
* RouteFVRP.cpp
*
*  Created on: May 16, 2017
*      Author: Juan Jose Palacios
*/

#include "RouteFVRP.h"


namespace FVRP {

//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Main constructor  =====================================================
RouteFVRP::RouteFVRP(const ProblemFVRP * problem)
	: stockUpdated(true), nVehicles(0), visitedCustomers(0) {

	this->problem = problem;

	this->visited.resize(problem->getNumberCustomers(), false);
}


//====  Copy constructor  =====================================================
RouteFVRP::RouteFVRP(const RouteFVRP & source)
	:nVehicles(source.nVehicles), visitedCustomers(source.visitedCustomers),
	stockUpdated(source.stockUpdated) {

	this->problem = source.problem;

	this->visited = source.visited;

	this->route.resize(source.route.size());
	for (size_t i = 0; i < source.route.size(); i++) {
		for (size_t j = 0; j < source.route[i].size(); j++)
			this->route[i].push_back(source.route[i][j]);
	}
}





//=============================================================================
//		GET/SET METHODS
//=============================================================================
//====  Get the route of a vehicler  ==========================================
std::vector<int> & RouteFVRP::getRoute(const unsigned int vehicle) const {
	std::vector<int> route;

	if (vehicle < 0 || vehicle >= this->route.size()) {
		std::string errorMsg = "Trying to access unexisting route or vehicle: ";
		errorMsg += valueToString(vehicle);
		throw new FVRPException("Routing", errorMsg);
	}

	for (size_t i = 0; i < this->route[vehicle].size(); i++)
		route.push_back(this->route[vehicle][i].cst->id);
	return route;
}


//====  Get the total time of a route  ========================================
FuzzyFW::TFN RouteFVRP::getRouteTime(const unsigned int vehicle) const {
	if (vehicle < 0 || vehicle >= this->route.size()) {
		std::string errorMsg = "Trying to access unexisting route or vehicle: ";
		errorMsg += valueToString(vehicle);
		throw new FVRPException("Routing", errorMsg);
	}

	CustomerInfo lastCustomer =
		this->route[vehicle][this->route[vehicle].size() - 1];
	return lastCustomer.arrivalTime;
}


//====  Get the remaining capacity of the vehicle  ============================
FuzzyFW::TFN RouteFVRP::getStockSpent(const unsigned int vehicle) const {
	if (vehicle < 0 || vehicle >= this->route.size()) {
		std::string errorMsg = "Trying to access unexisting route or vehicle: ";
		errorMsg += valueToString(vehicle);
		throw new FVRPException("Routing", errorMsg);
	}

	CustomerInfo lastCustomer =
		this->route[vehicle][this->route[vehicle].size() - 1];
	return lastCustomer.stockSpent + lastCustomer.cst->demand;
}


//====  Get if a customer has been visited  ===================================
bool RouteFVRP::isVisited(const unsigned int customerId) const {
	if (customerId < 0 || customerId >= this->visited.size()) {
		std::string errorMsg = "Trying to access unexisting customer: ";
		errorMsg += valueToString(customerId);
		throw new FVRPException("Routing", errorMsg);
	}
	return this->visited[customerId];
}




//=============================================================================
//		OPERATORS
//=============================================================================
//====  Assignment overload  ==================================================
RouteFVRP & RouteFVRP::operator=(const RouteFVRP & source) {
	this->nVehicles = source.nVehicles;
	this->visitedCustomers = source.visitedCustomers;
	this->stockUpdated = source.stockUpdated;

	this->problem = source.problem;

	this->visited = source.visited;

	this->route.clear();
	this->route.resize(source.route.size());
	for (size_t i = 0; i < source.route.size(); i++)
		this->route[i] = source.route[i];

	return *this;
}





//=============================================================================
//		METHODS
//=============================================================================
//====  addTask Method  =======================================================
void RouteFVRP::addCustomer(const unsigned int customerId,
	const unsigned int vehicle, FuzzyFW::TFN & visitTime,
	unsigned const int predecessor) {

	CustomerInfo newCustomer;
	size_t position;
	unsigned int pred;
	bool found;
	
	newCustomer.cst = this->problem->getCustomer(customerId);
	newCustomer.arrivalTime = visitTime;
	newCustomer.vechicle = vehicle;

	// Insert the new customer in the route
	if (vehicle >= this->nVehicles)
		this->route.resize(vehicle + 1);
	this->route[vehicle].push_back(newCustomer);
	position = this->route[vehicle].size() - 1;

	// Find its place in the route
	found = false;
	while (!found && position > 0) {
		if (this->route[vehicle][position - 1].cst->id == predecessor)
			found = true;
		else {
			if (stockUpdated)
				this->route[vehicle][position - 1].stockSpent =
				this->route[vehicle][position - 1].stockSpent
				+ this->route[vehicle][position - 1].cst->demand;

			std::swap(this->route[vehicle][position - 1],
				this->route[vehicle][position]);
			position--;
		}
	}

	// Update predecessor
	if (position > 0) {
		pred = this->route[vehicle][position - 1].cst->id;
		this->route[vehicle][position].pred = pred;
		this->route[vehicle][position - 1].succ = customerId;

		this->route[vehicle][position].stockSpent =
			this->route[vehicle][position - 1].stockSpent + this->problem->getDemand(pred);
	}
	else {
		this->route[vehicle][position].pred = -1;
		this->route[vehicle][position - 1].stockSpent =	FuzzyFW::TFN(0,0,0);
	}

	// Update succecessors
	if (position < this->route[vehicle].size() - 1) {
		this->route[vehicle][position].succ = this->route[vehicle][position + 1].cst->id;
		this->route[vehicle][position + 1].pred = this->route[vehicle][position].cst->id;
	}
	else
		this->route[vehicle][position].succ = -1;

	if (!this->visited[customerId]) {
		this->visitedCustomers++;
		this->visited[customerId] = true;
	}
}


//====  Updates the stocks in all customers  ==================================
void RouteFVRP::updateStock(const FuzzyFW::TFN::Maximum maxType) {
	for (size_t v = 0; v < this->route.size(); v++) {
		if (this->route[v].size() > 0) {
			this->route[v][0].stockSpent = FuzzyFW::TFN(0,0,0);
		}
		for (size_t j = 1; j < this->route[v].size(); j++) {
			this->route[v][j].stockSpent =
				this->route[v][j - 1].stockSpent +
				this->route[v][j - 1].cst->demand;
		}
	}
	this->stockUpdated = true;
}



//====  reset Method  =========================================================
void RouteFVRP::reset() {

	this->nVehicles = 0;
	this->visitedCustomers = 0;
	this->stockUpdated = true;

	for (size_t i = 0; i < this->visited.size(); i++) {
		this->visited[i] = false;
	}

	for (size_t i = 0; i < this->route.size(); i++)
		this->route[i].clear();
	this->route.clear();
}

}
