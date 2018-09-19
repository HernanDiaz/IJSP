/*
* CustomerFVRP.h
*
*  Created on: Nov 23, 2017
*      Author: Juan Jose Palacios
*/
#pragma once

#include "TFN.h"
#include "TimeWindow.h"


namespace FVRP {

//=============================================================================
//
//	Class CustomerFVRP
//
//=============================================================================
/**
* Objects of this class contain the constant values for customers
* of a fuzzy VRP instance. That is, it represents values such as demand
* as Fuzzy Numbers (TFNs in this versions)
*
* @author Juan Jose Palacios
*
*/

class CustomerFVRP {

	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/**
	* Customer identifier
	*/
	unsigned int id;

	/**
	* Time Window
	*/
	FuzzyFW::TimeWindow * timeWindow;

	/**
	* Demand
	*/
	FuzzyFW::TFN demand;

	/**
	* Service time
	*/
	FuzzyFW::TFN serviceTime;

	/**
	* Position in the space
	*/
	double x;
	double y;

	

	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	CustomerFVRP();


	/**
	* Copy constructor
	*/
	CustomerFVRP(const CustomerFVRP &source);


	/**
	* Destructor
	*/
	~CustomerFVRP() {
		delete this->timeWindow;
	}



	//=========================================================================
	//		OPERATORS
	//=========================================================================
public:
	CustomerFVRP & operator=(const CustomerFVRP &src);
};

}
