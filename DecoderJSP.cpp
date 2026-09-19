/*
 * DecoderJSP.cpp
 *
 *  Created on: June 25, 2019
 *      Author: Hernan Diaz
 */

#include "DecoderJSP.h"

namespace JSP {

//=============================================================================
//
//	Abstract class DecoderJSP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
DecoderJSP::DecoderJSP(const DecoderJSP & source)
	: sgsLabel(source.sgsLabel), Decoder(source) {
	if (source.sgs)
		this->sgs.reset(source.sgs->clone());
}



//-----  Setup method  --------------------------------------------------------
std::vector<std::string> DecoderJSP::buildDecoderName(const std::string &name) const {
	std::vector<std::string> result;
	std::vector<std::string> sgsName = this->sgs->getName();
	result.push_back(name);
	result.push_back(";SGS:;" + sgsName[0]);
	for (size_t i = 1; i < sgsName.size(); i++)
		result.push_back(";" + sgsName[i]);
	return result;
}

void DecoderJSP::setup(FuzzyFW::ParameterDB *parameters) {
	Decoder::setup(parameters);

	// Loads the SGS type to create
	std::string sgsType = parameters->getStringLower(this->sgsLabel);
	if (sgsType.length() == 0) {
		std::string errorMsg = "SGS not found. Please, specify a SGS to use";
		errorMsg += " during the evaluation of individuals";
		throw JSPException("Evaluation", errorMsg);
	}
	this->sgs.reset(JSPClassRegister::getSGSObject(sgsType));
	if (this->sgs == NULL) {
		std::string errorMsg = "The introduced SGS is not";
		errorMsg += " recognised: \'" + sgsType + "\'";
		throw JSPException("Evaluation", errorMsg);
	}
	this->sgs->setup(parameters);
}





//=============================================================================
//
//	Class DecoderJSP_Order
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main method  ---------------------------------------------------------
FuzzyFW::Solution * DecoderJSP_Order::decode(FuzzyFW::Individual * indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) {

	FuzzyFW::IndividualArrayInt * ind;

	// Check for the type of encoding
	if (dynamic_cast<EncoderJSP_Order *>(svars->encoder) == NULL) {
		std::string errorMsg = "The individual is encoded with a ";
		errorMsg += "incompatible method";
		throw JSPException("Decoding", errorMsg);
	}

	// Check for the type of individual
	ind = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (ind == NULL) {
		std::string errorMsg = "Genotype type not valid";
		throw JSPException("Decoding", errorMsg);
	}
	
	this->sgs->reset();
	return sgs->buildSchedule(svars, ind->getGenotype());
}





//=============================================================================
//
//	Class DecoderJSP_JobOrder
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main method  ---------------------------------------------------------
FuzzyFW::Solution * DecoderJSP_JobOrder::decode(FuzzyFW::Individual * indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) {

	int job;
	std::vector<int> solution, count;
	FuzzyFW::IndividualArrayInt * ind;

	// Check for the type of encoding
	if (dynamic_cast<EncoderJSP_JobOrder *>(svars->encoder) == NULL) {
		std::string errorMsg = "The individual is encoded with an ";
		errorMsg += "incompatible method";
		throw JSPException("Decoding", errorMsg);
	}

	// Check for the type of individual
	ind = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (ind == NULL) {
		std::string errorMsg = "Genotype type not valid";
		throw JSPException("Decoding", errorMsg);
	}

	ProblemJSP * fuzzyProb =
		dynamic_cast<ProblemJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw JSPException("Encoding", errorMsg);
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
