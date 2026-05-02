/*
* RouteFVRP.cpp
*
*  Created on: May 16, 2017
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

	this->node.resize(problem->getNumberCustomers());
	for (unsigned int i = 0; i < problem->getNumberCustomers(); i++) {
		this->node[i].arrivalTime = FuzzyFW::TFN(-1, -1, -1);
		this->node[i].pred = -1;
		this->node[i].succ = -1;
		this->node[i].distance = 0.0;
		this->node[i].stockSpent = FuzzyFW::TFN(0, 0, 0);
		this->node[i].vechicle = -1;
		this->node[i].cst = problem->getCustomer(i);
	}
	this->node[0].arrivalTime = FuzzyFW::TFN(0, 0, 0);
}


//====  Copy constructor  =====================================================
RouteFVRP::RouteFVRP(const RouteFVRP & source)
	:nVehicles(source.nVehicles), visitedCustomers(source.visitedCustomers),
	stockUpdated(source.stockUpdated) {

	this->problem = source.problem;
	
	this->node.resize(source.node.size());
	for (size_t i = 0; i < source.node.size(); i++) {
			this->node[i] = source.node[i];
	}

	this->routes.resize(source.routes.size());
	for (size_t i = 0; i < source.routes.size(); i++) {
		for (size_t j = 0; j < source.routes[i].size(); j++)
			this->routes[i].push_back(source.routes[i][j]);
	}
}





//=============================================================================
//		GET/SET METHODS
//=============================================================================
//====  Get the route of a vehicler  ==========================================
std::vector<int> RouteFVRP::getRoute(const unsigned int vehicle) const {
	std::vector<int> route;

	if (vehicle < 0 || vehicle >= this->routes.size()) {
		std::string errorMsg = "Trying to access unexisting route or vehicle: ";
		errorMsg += valueToString(vehicle);
		throw FVRPException("Routing", errorMsg);
	}

	for (size_t i = 0; i < this->routes[vehicle].size(); i++)
		route.push_back(this->routes[vehicle][i]);
	return route;
}


//====  Get the length of a route  ============================================
unsigned int RouteFVRP::getRouteSize(const unsigned int vehicle) const {
	std::vector<int> route;

	if (vehicle < 0) {
		std::string errorMsg = "Trying to access an incorrect route or vehicle: ";
		errorMsg += valueToString(vehicle);
		throw FVRPException("Routing", errorMsg);
	}
	if (vehicle >= this->routes.size())
		return 0;
	return this->routes[vehicle].size();
}


//====  Get the total time of a route  ========================================
FuzzyFW::TFN RouteFVRP::getRouteTime(const unsigned int vehicle) const {
	if (vehicle < 0 || vehicle >= this->routes.size()) {
		std::string errorMsg = "Trying to access unexisting route or vehicle: ";
		errorMsg += valueToString(vehicle);
		throw FVRPException("Routing", errorMsg);
	}

	unsigned int lastCustomer = this->getLastCustomer(vehicle);
	return this->node[lastCustomer].arrivalTime
		+ this->problem->getServiceTime(lastCustomer)
		+ this->problem->getTravelTime(0, lastCustomer);
}


//====  Get the total distance of a route  ====================================
double RouteFVRP::getRouteDistance(const unsigned int vehicle) const {
	if (vehicle < 0 || vehicle >= this->routes.size()) {
		std::string errorMsg = "Trying to access unexisting route or vehicle: ";
		errorMsg += valueToString(vehicle);
		throw FVRPException("Routing", errorMsg);
	}

	unsigned int lastCustomer = this->getLastCustomer(vehicle);
	return this->node[lastCustomer].distance
		+ this->problem->getDistance(0, lastCustomer);
}


//====  Get the first cutomer in a route  =====================================
unsigned int RouteFVRP::getFirstCustomer(const unsigned int vehicle) const {
	if (vehicle < 0) {
		std::string errorMsg = "Trying to access incorrect route or vehicle: ";
		errorMsg += valueToString(vehicle);
		throw FVRPException("Routing", errorMsg);
	}
	if(vehicle >= this->routes.size() || this->routes[vehicle].size() == 0) {
		return 0;
	}
	return this->routes[vehicle][0];
}


//====  Get the last cutomer in a route  ======================================
unsigned int RouteFVRP::getLastCustomer(const unsigned int vehicle) const {
	if (vehicle < 0) {
		std::string errorMsg = "Trying to access incorrect route or vehicle: ";
		errorMsg += valueToString(vehicle);
		throw FVRPException("Routing", errorMsg);
	}
	if (vehicle >= this->routes.size() || this->routes[vehicle].size() == 0) {
		return 0;
	}
	return this->routes[vehicle][this->routes.size() - 1];
}


//====  Get the remaining capacity of the vehicle  ============================
FuzzyFW::TFN RouteFVRP::getStockSpent(const unsigned int vehicle) const {
	if (vehicle < 0 || vehicle >= this->routes.size()) {
		std::string errorMsg = "Trying to access unexisting route or vehicle: ";
		errorMsg += valueToString(vehicle);
		throw FVRPException("Routing", errorMsg);
	}

	unsigned int lastCustomer =
		this->routes[vehicle][this->routes[vehicle].size() - 1];
	return this->node[lastCustomer].stockSpent + this->node[lastCustomer].cst->demand;
}


//====  Get if a customer has been visited  ===================================
bool RouteFVRP::isVisited(const unsigned int customerId) const {
	if (customerId < 0 || customerId >= this->node.size()) {
		std::string errorMsg = "Trying to access unexisting customer: ";
		errorMsg += valueToString(customerId);
		throw FVRPException("Routing", errorMsg);
	}
	return this->node[customerId].vechicle >= 0;
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

	this->node.clear();
	this->node.resize(source.node.size());
	for (size_t i = 0; i < source.node.size(); i++) {
		this->node[i] = source.node[i];
	}

	this->routes.clear();
	this->routes.resize(source.routes.size());
	for (size_t i = 0; i < source.routes.size(); i++) {
		for (size_t j = 0; j < source.routes[i].size(); j++)
			this->routes[i].push_back(source.routes[i][j]);
	}

	return *this;
}





//=============================================================================
//		METHODS
//=============================================================================
//====  addTask Method  =======================================================
void RouteFVRP::addCustomer(const unsigned int customerId,
	const unsigned int vehicle, FuzzyFW::TFN & visitTime,
	unsigned const int predecessor) {

	size_t position;
	unsigned int succ, pred;
	bool found;
	
	if (customerId < 0 || customerId >= this->node.size()) {
		std::string errorMsg = "Trying to visit an unexisting customer: ";
		errorMsg += valueToString(customerId);
		throw FVRPException("Routing", errorMsg);
	}
	if (this->node[customerId].vechicle >= 0) {
		std::string errorMsg = "This node has been already visited in";
		errorMsg += " another route";
		throw FVRPException("Routing", errorMsg);
	}

	// Update customer data
	this->node[customerId].arrivalTime = visitTime;
	this->node[customerId].vechicle = vehicle;
	this->node[customerId].pred = predecessor;

	// Append the new customer to the route (or generate route if
	// it does not exist yet)
	if (vehicle >= this->nVehicles) {
		this->routes.resize(vehicle + 1);
		this->nVehicles = vehicle + 1;
	}
	this->routes[vehicle].push_back(customerId);
	position = this->routes[vehicle].size() - 1;

	// Find its place in the route
	found = false;
	while (!found && position > 0) {
		if (this->routes[vehicle][position - 1] == predecessor)
			found = true;
		else {
			if (stockUpdated)
				this->node[this->routes[vehicle][position - 1]].stockSpent =
				this->node[this->routes[vehicle][position - 1]].stockSpent
				+ this->node[customerId].cst->demand;

			std::swap(this->routes[vehicle][position - 1],
				this->routes[vehicle][position]);
			position--;
		}
	}

	if (predecessor > 0 && position == 0) {
		std::string errorMsg = "The specified predecessor is not in the same route";
		throw FVRPException("Routing", errorMsg);
	}

	// Update predecessor
	if (position > 0) {
		this->node[predecessor].succ = customerId;
		this->node[customerId].distance = this->node[predecessor].distance
			+ this->problem->getDistance(predecessor, customerId);
		this->node[customerId].stockSpent =
			this->node[predecessor].stockSpent + this->node[predecessor].cst->demand;
	}
	else {
		this->node[customerId].stockSpent = FuzzyFW::TFN(0, 0, 0);
		this->node[customerId].distance = this->problem->getDistance(0, customerId);
	}

	// Update succecessor
	if (position < this->routes[vehicle].size() - 1) {
		succ = this->routes[vehicle][position + 1];
		this->node[customerId].succ = succ;
		this->node[succ].pred = customerId;
		
		// Update distances
		pred = customerId;
		while (succ != 0) {
			this->node[succ].distance = this->node[pred].distance
				+ this->problem->getDistance(succ, pred);
			pred = succ;
			succ = this->node[pred].succ;
		}
	}
	else
		this->node[customerId].succ = 0;

	this->visitedCustomers++;
}



//====  Updates the stocks in all customers  ==================================
void RouteFVRP::updateStock(const FuzzyFW::TFN::Maximum maxType) {
	unsigned int customer;
	FuzzyFW::TFN stock;
	for (size_t v = 0; v < this->routes.size(); v++) {
		if (this->routes[v].size() > 0) {
			stock = FuzzyFW::TFN(0, 0, 0);
			customer = this->routes[v][0];
			while (customer > 0) {
				this->node[customer].stockSpent = stock;
				stock = stock + this->node[customer].cst->demand;
				customer = this->node[customer].succ;
			}
		}
	}
	this->stockUpdated = true;
}


//====  Update routes  ========================================================
void RouteFVRP::updateRoute(unsigned int v, unsigned int firstCustomer) {
	if (firstCustomer < 0 || firstCustomer >= this->node.size()) {
		std::string errorMsg = "Reference customer non-existing: ";
		errorMsg += valueToString(firstCustomer);
		throw FVRPException("Routing", errorMsg);
	}

	// Re-adapt the size of the routing structure for the vehicle
	if (v >= this->nVehicles) {
		this->routes.resize(v + 1);
		this->nVehicles = v + 1;
	}
	
	if (this->node[firstCustomer].vechicle != v) {
		std::string errorMsg = "Error while trying to update a route.";
		errorMsg += valueToString(firstCustomer);
		throw FVRPException("Routing", errorMsg);
	}

	unsigned int succ;
	this->routes[v].clear();
	this->routes[v].push_back(firstCustomer);
	succ = this->node[firstCustomer].succ;
	while (succ > 0) {
		this->routes[v].push_back(firstCustomer);
		succ = this->node[succ].succ;
	}
}



//====  reset Method  =========================================================
void RouteFVRP::reset() {

	this->nVehicles = 0;
	this->visitedCustomers = 0;
	this->stockUpdated = true;

	for (size_t i = 0; i < this->node.size(); i++) {
		this->node[i].arrivalTime = FuzzyFW::TFN(-1, -1, -1);
		this->node[i].pred = -1;
		this->node[i].succ = -1;
		this->node[i].stockSpent = FuzzyFW::TFN(0, 0, 0);
		this->node[i].vechicle = -1;
		this->node[i].cst = problem->getCustomer(i);
	}
	this->node[0].arrivalTime = FuzzyFW::TFN(0, 0, 0);

	for (size_t i = 0; i < this->routes.size(); i++)
		this->routes[i].clear();
	this->routes.clear();
}

}
