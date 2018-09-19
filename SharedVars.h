/*
* SharedVars.h
*
*  Created on: Nov 29, 2016
*      Author: Juan Jose Palacios
*/
#pragma once

#include "Problem.h"

namespace FuzzyFW {


/**
* This struct will keep those objects that are shared by all the
* components of any evolutionary algorithm
*
* By having all those objects together, we won't need to pass them
* one by one from one component to another.
*
* A clear example of one of this objects that are commonly shared is
* the random number generator.
*
* @author jjpalacios
*
*/
class SharedVars {
public:
	/**
	* Random Number Generator
	*/
	Random * rng;

	/**
	* Parameter database
	*/
	ParameterDB * parameters;

	/**
	* Problem to solve
	*/
	Problem * problem;

	/**
	* Default constructor
	*/
	SharedVars() {
		rng = NULL;
		parameters = NULL;
		problem = NULL;
	}

	/**
	* Destructor
	*/
	~SharedVars() {
		delete rng;
		delete parameters;
	}
};

}
