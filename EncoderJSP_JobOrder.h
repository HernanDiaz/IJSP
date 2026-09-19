/*
 * EncoderJSP_JobOrder.h
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */
#pragma once

#include "EncoderJSP_Order.h"


namespace JSP {

//=============================================================================
//
//	Class EncoderJSP_JobOrder
//
//=============================================================================
/**
* This encoding method codifies a schedule as an integer permutation that
* contains the topological order of the solution in it.
* Each task is codified with the number of job it belongs to
*
* @author hdiaz
*
*/
class EncoderJSP_JobOrder : public FuzzyFW::Encoder {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit EncoderJSP_JobOrder(FuzzyFW::ParameterDB *parameters = NULL)
		: Encoder(parameters) { }

	/**
	* Copy constructor
	*/
	EncoderJSP_JobOrder(const EncoderJSP_JobOrder &source)
		: Encoder(source) { }

	/**
	* Loads the needed parameters. None in this case
	*/
	//virtual void setup(ParameterDB *parameters) { }

	/**
	* Clone method, in case of inheritance
	*/
	virtual Encoder * clone() const {
		return new EncoderJSP_JobOrder(*this);
	}

	/**
	* Destructor
	*/
	virtual ~EncoderJSP_JobOrder() { } 	// Nothing to destroy here



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
		name.push_back("Job Permutation");
		return name;
	}
};

}
