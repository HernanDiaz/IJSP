/*
 * EncoderFVRP.h
 *
 *  Created on: November 23, 2017
 */
#pragma once

#include "Encoder.h"
#include "RouteFVRP.h"



namespace FVRP {
//=============================================================================
//
//	Class EncoderFJSP_Order
//
//=============================================================================
/**
* This encoding method codifies a FVRP solution as an integer permutation that
* contains the paths followed by each vehicle in a sequential fashion.
* Customers are numbered from 1 to n, being n the number of customers.
*
*
*/
class EncoderFVRP_Order : public FuzzyFW::Encoder {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit EncoderFVRP_Order(FuzzyFW::ParameterDB *parameters = NULL)
		: FuzzyFW::Encoder(parameters) { }

	/**
	* Copy constructor
	*/
	EncoderFVRP_Order(const EncoderFVRP_Order &source)
		: FuzzyFW::Encoder(source) { }

	/**
	* Loads the needed parameters. None in this case
	*/
	//virtual void setup(ParameterDB *parameters);

	/**
	* Clone method, in case of inheritance
	*/
	virtual FuzzyFW::Encoder * clone() const {
		return new EncoderFVRP_Order(*this);
	}

	/**
	* Destructor
	*/
	virtual ~EncoderFVRP_Order() { } 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Encodes the solution into the individual's genotpye
	*/
	virtual void encode(FuzzyFW::Solution *solution,
		FuzzyFW::Individual *indiv, 
		const FuzzyFW::SharedVarsEvolutionary * const svars) const;


	/**
	* Encodes the solution into a new individual
	*/
	virtual FuzzyFW::Individual * encode(FuzzyFW::Solution *solution,
		const FuzzyFW::SharedVarsEvolutionary * const svars) const;


	/**
	* Get the name and setup of the encoding method
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		name.push_back("Visit Order");
		return name;
	}
};


}
