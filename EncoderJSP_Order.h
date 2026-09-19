/*
 * EncoderJSP_Order.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "Encoder.h"
#include "ScheduleJSP.h"


namespace JSP {

//=============================================================================
//
//	Class EncoderJSP_Order
//
//=============================================================================
/**
* This encoding method codifies a schedule as an integer permutation that
* contains the topological order of the solution in it.
* Tasks are numbered from 0 to n-1, begin n the number of tasks or operations.
*
* @author hdiaz
*
*/
class EncoderJSP_Order : public FuzzyFW::Encoder {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit EncoderJSP_Order(FuzzyFW::ParameterDB *parameters = NULL)
		: FuzzyFW::Encoder(parameters) { }

	/**
	* Copy constructor
	*/
	EncoderJSP_Order(const EncoderJSP_Order &source)
		: FuzzyFW::Encoder(source) { }

	/**
	* Loads the needed parameters. None in this case
	*/
	//virtual void setup(ParameterDB *parameters);

	/**
	* Clone method, in case of inheritance
	*/
	virtual FuzzyFW::Encoder * clone() const {
		return new EncoderJSP_Order(*this);
	}

	/**
	* Destructor
	*/
	virtual ~EncoderJSP_Order() { } 	// Nothing to destroy here



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
		name.push_back("Task Permutation");
		return name;
	}
};

}
