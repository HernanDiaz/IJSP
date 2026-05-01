/*
* RouteFVRP.h
*
*  Created on: Oct 27, 2017
*/
#pragma once

#include "ProblemFVRP.h"
#include "Solution.h"


namespace FVRP {


//=========================================================================
//
//	Struct CustomerInfo
//
//=========================================================================
/**
* This is an auxiliary struct containing all data of the customers that
* is dynamic and dependent on the proposed solution to the VRP problem
*
*/
struct CustomerInfo {
	CustomerFVRP * cst;	// Customer
	int vechicle;	// Vehicle that visits the customer
	FuzzyFW::TFN arrivalTime;	// Estimated arrival time of the vehicle
	int pred;	// Customer visited before this one
	int succ;	// Customer visited after this one
	FuzzyFW::TFN stockSpent;	// Stock spent by the evhicle when it ARRIVES
};





//=========================================================================
//
//	Class RouteFVRP
//
//=========================================================================
/**
* Objects of this class represent a [partial] solution to a Fuzzy VRP.
* We consider a partial solution as allocation of vehicles and the
* assignmente of a route to each of them. 
*
* To avoid repating computations, for each customer, some dynamic
* information is stored, such as the arrival time at the customer or the
* vehicle serving it.
*
* The class stores all the info in a matrix, such that each row i
* represents the route to follow by vehicle i.
*
*
*/
class RouteFVRP : public FuzzyFW::Solution
{
	//=====================================================================
	//		FIELDS
	//=====================================================================
public:
	/**
	* Dynamic information of already visited customers
	*/
	std::vector< std::vector<CustomerInfo> > route;

	

protected:
	/**
	* Number of vehicles needed in the solution
	*/
	unsigned int nVehicles;

	/**
	* Number of customers visited until now
	*/
	unsigned int visitedCustomers;

	/**
	* Problem that this schedule solves
	*/
	const ProblemFVRP *problem;

	/**
	* Flag indicating the visited customers
	*/
	std::vector<char> visited;

	/**
	* Stocks updated
	*/
	bool stockUpdated;



	//=====================================================================
	//		CONSTRUCTORS
	//=====================================================================
public:
	/**
	* Main constructor
	*/
	RouteFVRP(const ProblemFVRP * problem = NULL);

	/**
	* Copy constructor
	*/
	RouteFVRP(const RouteFVRP & source);

	/**
	* Destructor
	*/
	virtual ~RouteFVRP() {
		visited.clear();
		route.clear();
	}


	/**
	* Clone methods (like in Java)
	*/
	virtual FuzzyFW::Solution * clone() const {
		return new RouteFVRP(*this);
	}



	//=====================================================================
	//		GET/SET METHODS
	//=====================================================================
public:
	/**
	* Get the number of planned vehicles
	*/
	unsigned int getNumberVehicles() const {
		return this->nVehicles;
	}

	/**
	* Get the number of visited customers until now
	*/
	unsigned int getVisitedCustomer() const {
		return this->visitedCustomers;
	}

	/**
	* Get if the customer has been visited
	*/
	bool isVisited(const unsigned int customerId) const;

	/**
	* Get the order of customers id's visited by a vehicle
	*/
	std::vector<int> & getRoute(const unsigned int vehicle) const;

	/**
	* Get the completion time of the route of a vehicle
	*/
	FuzzyFW::TFN getRouteTime(const unsigned int vehicle) const;

	/**
	* Get the completion time of a stock remaining after finishing a rout
	*/
	FuzzyFW::TFN getStockSpent(const unsigned int vechicle) const;



	//=====================================================================
	//		OPERATORS
	//=====================================================================
public:
	/**
	* Assignment overload
	*/
	RouteFVRP & operator=(const RouteFVRP & source);

	/**
	* Converts the solution to a string
	*/
	virtual std::string toString() const {
		std::string str;
		str += valueToString(this->nVehicles);

		for (size_t i = 0; i < this->route.size(); i++) {
			str += " |";
			for (size_t j = 0; j < this->route[i].size(); j++)
				str += " " + this->route[i][j].cst->id;
		}
		return str;
	}



	//=====================================================================
	//		METHODS
	//=====================================================================
public:
	/**
	* Add a new customer to a vehicle route
	*/
	void addCustomer(const unsigned int customer,
		const unsigned int vehicle,
		FuzzyFW::TFN & visitTime,
		unsigned const int predecessor);

	/**
	* Update the stock levels of all customers
	*/
	void updateStock(const FuzzyFW::TFN::Maximum maxType);


	/**
	* Clears the routing plan
	*/
	void reset();
};

}
