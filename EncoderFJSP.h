/*
 * EncoderFJSP.h
 *
 *  Created on: September 13, 2017
 */
#pragma once

#include "Encoder.h"
#include "ScheduleFJSP.h"



namespace FJSP {
//=============================================================================
//
//	Class EncoderFJSP_Order
//
//=============================================================================
/**
* This encoding method codifies a schedule as an integer permutation that
* contains the topological order of the solution in it. 
* Tasks are numbered from 0 to n-1, begin n the number of tasks or operations.
*
*
*/
class EncoderFJSP_Order : public FuzzyFW::Encoder {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit EncoderFJSP_Order(FuzzyFW::ParameterDB *parameters = NULL)
		: FuzzyFW::Encoder(parameters) { }

	/**
	* Copy constructor
	*/
	EncoderFJSP_Order(const EncoderFJSP_Order &source)
		: FuzzyFW::Encoder(source) { }

	/**
	* Loads the needed parameters. None in this case
	*/
	//virtual void setup(ParameterDB *parameters);

	/**
	* Clone method, in case of inheritance
	*/
	virtual FuzzyFW::Encoder * clone() const {
		return new EncoderFJSP_Order(*this);
	}

	/**
	* Destructor
	*/
	virtual ~EncoderFJSP_Order() { } 	// Nothing to destroy here



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





//=============================================================================
//
//	Class EncoderFJSP_JobOrder
//
//=============================================================================
/**
* This encoding method codifies a schedule as an integer permutation that
* contains the topological order of the solution in it.
* Each task is codified with the number of job it belongs to
*
*
*/
class EncoderFJSP_JobOrder : public FuzzyFW::Encoder {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit EncoderFJSP_JobOrder(FuzzyFW::ParameterDB *parameters = NULL)
		: Encoder(parameters) { }

	/**
	* Copy constructor
	*/
	EncoderFJSP_JobOrder(const EncoderFJSP_Order &source)
		: Encoder(source) { }

	/**
	* Loads the needed parameters. None in this case
	*/
	//virtual void setup(ParameterDB *parameters) { }

	/**
	* Clone method, in case of inheritance
	*/
	virtual Encoder * clone() const {
		return new EncoderFJSP_JobOrder(*this);
	}

	/**
	* Destructor
	*/
	virtual ~EncoderFJSP_JobOrder() { } 	// Nothing to destroy here



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
