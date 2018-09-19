/*
 * DecoderFVRP.h
 *
 *  Created on: November 23, 2017
 *      Author: jjpalacios
 */
#pragma once

#include "FVRPClassRegister.h"
#include "Decoder.h"
#include "EncoderFVRP.h"


namespace FVRP {

#define DECODING_SGS "decode.sgs"

//=============================================================================
//
//	Abstract class DecoderFVRP
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype and generates
* a fuzzy VRP solution from it by using a SGS
*
* @author jjpalacios
*
*/
class DecoderFVRP : public FuzzyFW::Decoder {
protected:
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
	/*
	* Label to identify the SGS type
	*/
	const std::string sgsLabel;

	/*
	* SGS to create schedules from task orderings
	*/
	SGS_FVRP * sgs;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	DecoderFVRP(FuzzyFW::ParameterDB *parameters = NULL)
		: Decoder(parameters), sgsLabel(DECODING_SGS) { }

	/**
	* Copy constructor
	*/
	DecoderFVRP(const DecoderFVRP & source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~DecoderFVRP() {
		delete this->sgs;
	}
};





//=============================================================================
//
//	Class DecoderFVRP_Split
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype in the form
* of a customers order and generates a solution using a specific Split SGS
*
* @author jjpalacios
*
*/
class DecoderFRVP_Split : public DecoderFVRP {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	DecoderFRVP_Split(FuzzyFW::ParameterDB *parameters = NULL)
		: DecoderFVRP(parameters) { } 


	/**
	* Copy constructor
	*/
	DecoderFRVP_Split(const DecoderFRVP_Split & source)
		: DecoderFVRP(source) { }

	/**
	* Loads the needed parameters.
	*/
	//virtual void setup(ParameterDB *parameters);


	/**
	* Clone method, in case of inheritance
	*/
	virtual Decoder * clone() const {
		return new DecoderFRVP_Split(*this);
	}


	/**
	* Destructor
	*/
	virtual ~DecoderFRVP_Split() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Decodes an individual to create a full schedule
	*/
	virtual FuzzyFW::Solution * decode(FuzzyFW::Individual * indiv,
		const FuzzyFW::SharedVarsEvolutionary * const svars);


	/**
	* Get the name and setup of the operator
	*
	* @return A string of parameter values. The first string is the name of
	* the operator
	*/
	virtual std::vector<std::string> getName() const {
		std::vector<std::string> name;
		std::vector<std::string> sgsName = this->sgs->getName();
		name.push_back("Split");
		name.push_back(";SGS:;" + sgsName[0]);
		for (size_t i = 1; i < sgsName.size(); i++)
			name.push_back(";" + sgsName[i]);
		return name;
	}
};

}
