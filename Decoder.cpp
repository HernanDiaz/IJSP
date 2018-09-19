/*
 * Decoder.cpp
 *
 *  Created on: Sep 12, 2017
 *      Author: jjpalacios
 */

#include "Decoder.h"

namespace FJSP {

//=============================================================================
//
//	Abstract class DecoderFJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
DecoderFJSP::DecoderFJSP(const DecoderFJSP & source)
	: sgsLabel(source.sgsLabel), Decoder(source) {
	if (source.sgs != NULL)
		this->sgs = source.sgs->clone();
	else
		this->sgs = NULL;
}



//-----  Setup method  --------------------------------------------------------
void DecoderFJSP::setup(ParameterDB *parameters) {
	Decoder::setup(parameters);

	// Loads the SGS type to create
	std::string sgsType = parameters->getStringLower(this->sgsLabel);
	if (sgsType.length() == 0) {
		std::string errorMsg = "SGS not found. Please, specify a SGS to use";
		errorMsg += " during the evaluation of individuals";
		throw new FJSPException("Evaluation", errorMsg);
	}

	this->sgs = SchedulingClassRegister::getSGSObject(sgsType);
	if (this->sgs == NULL) {
		std::string errorMsg = "The introduced SGS is not";
		errorMsg += " recognised: \'" + sgsType + "\'";
		throw new FJSPException("Evaluation", errorMsg);
	}
	this->sgs->setup(parameters);
}





//=============================================================================
//
//	Class DecoderFJSP_Order
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main method  ---------------------------------------------------------
Solution * DecoderFJSP_Order::decode(Individual * indiv,
	const SharedVars * const svars) {

	IndividualArrayInt * ind;

	// Check for the type of encoding
	if (dynamic_cast<EncoderFJSP_Order *>(svars->encoder) == NULL) {
		std::string errorMsg = "The individual is encoded with a ";
		errorMsg += "incompatible method";
		throw new FJSPException("Decoding", errorMsg);
	}

	// Check for the type of individual
	ind = dynamic_cast<IndividualArrayInt *>(indiv);
	if (ind == NULL) {
		std::string errorMsg = "Genotype type not valid";
		throw new FJSPException("Decoding", errorMsg);
	}
	
	this->sgs->reset();
	return sgs->buildSchedule(svars, ind->getGenotype());
}





//=============================================================================
//
//	Class DecoderFJSP_JobOrder
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main method  ---------------------------------------------------------
Solution * DecoderFJSP_JobOrder::decode(Individual * indiv,
	const SharedVars * const svars) {

	int job;
	std::vector<int> solution, count;
	IndividualArrayInt * ind;

	// Check for the type of encoding
	if (dynamic_cast<EncoderFJSP_JobOrder *>(svars->encoder) == NULL) {
		std::string errorMsg = "The individual is encoded with an ";
		errorMsg += "incompatible method";
		throw new FJSPException("Decoding", errorMsg);
	}

	// Check for the type of individual
	ind = dynamic_cast<IndividualArrayInt *>(indiv);
	if (ind == NULL) {
		std::string errorMsg = "Genotype type not valid";
		throw new FJSPException("Decoding", errorMsg);
	}

	FuzzyProblem * fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	solution = ind->getGenotype();
	count.resize(fuzzyProb->getNumberJobs(), 0);
	for (size_t i = 0; i < solution.size(); i++) {
		job = solution[i];
		solution[i] = fuzzyProb->getTaskId(job, count[job]);
		count[job]++;
	}

	this->sgs->reset();
	return sgs->buildSchedule(svars, solution);
}

}
