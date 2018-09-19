/*
* SharedVarsEvolutionary.h
*
*  Created on: Nov 29, 2016
*      Author: Juan Jose Palacios
*/
#pragma once

#include "SharedVars.h"


namespace FuzzyFW {

// Fordward declarations
class Decoder;
class Encoder;



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
class SharedVarsEvolutionary : public SharedVars {
public:
	/**
	* Decoder for the algorithm
	*/
	Decoder * decoder;

	/**
	* Encoder for the algorithm
	*/
	Encoder * encoder;

	/**
	* Default constructor
	*/
	SharedVarsEvolutionary()
	: SharedVars() {
		decoder = NULL;
		encoder = NULL;
	}

	/**
	* Destructor
	*/
	~SharedVarsEvolutionary() {
		// delete encoder;
		// delete decoder;
	}
};

}
