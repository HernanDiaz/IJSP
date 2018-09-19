/*
* SharedVars.h
*
*  Created on: Nov 29, 2016
*      Author: Juan Jose Palacios
*/

#ifndef FJSPOBJECTS_SHAREDVARS_H_
#define FJSPOBJECTS_SHAREDVARS_H_

#include "Random.h"
#include "FuzzyProblem.h"
#include "Decoder.h"
#include "Encoder.h"

namespace FJSP {

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
		* Decoder for the algorithm
		*/
		Decoder * decoder;

		/**
		* Encoder for the algorithm
		*/
		Encoder * encoder;

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
			delete problem;
			delete parameters;
		}
	};

}

#endif /* FJSPOBJECTS_SHAREDVARS_H_ */
