/*
 * DecoderJSP.h
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */
#pragma once

#include "JSPClassRegister.h"
#include "Decoder.h"
#include <memory>
#include "EncoderJSP.h"


namespace JSP {

#define DECODING_SGS "decode.sgs"

//=============================================================================
//
//	Abstract class DecoderJSP
//
//=============================================================================
/**
* This class creates a method apply that receives a genotype and generates
* a fuzzy schedule from it by using a SGS
*
* @author hdiaz
*
*/
class DecoderJSP : public FuzzyFW::Decoder {
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
	std::unique_ptr<SGS_JSP> sgs;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	DecoderJSP(FuzzyFW::ParameterDB *parameters = NULL)
		: Decoder(parameters), sgsLabel(DECODING_SGS) { }

	/**
	* Copy constructor
	*/
	DecoderJSP(const DecoderJSP & source);

	/**
	* Loads the needed parameters.
	*/
	virtual void setup(FuzzyFW::ParameterDB *parameters);

	/**
	* Destructor
	*/
	virtual ~DecoderJSP() = default;

protected:
	std::vector<std::string> buildDecoderName(const std::string &name) const;
};

}

#include "DecoderJSP_Order.h"
#include "DecoderJSP_JobOrder.h"
