/*
 * Decoder.cpp
 *
 *  Created on: Sep 14, 2017
 *      Author: jjpalacios
 */

#include "Encoder.h"

namespace FJSP {

//=============================================================================
//
//	Class EncoderFJSP_Order
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Encoding method  -----------------------------------------------------
void EncoderFJSP_Order::encode(Solution *solution, Individual *indiv,
	const SharedVars * const svars) const {

	FuzzySchedule *schedule;
	IndividualArrayInt * intIndiv;
	
	schedule = dynamic_cast<FuzzySchedule *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	intIndiv = dynamic_cast<IndividualArrayInt *>(indiv);
	if (intIndiv == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "integer array individuals.";
		throw new FJSPException("Encoding", errorMsg);
	}

	intIndiv->setGenotype(schedule->getTaskOrder());
}



//-----  Encoding method  -----------------------------------------------------
Individual * EncoderFJSP_Order::encode(Solution *solution,
	const SharedVars * const svars) const {

	FuzzySchedule *schedule;
	std::vector<unsigned int> taskOrder;
	std::vector<int> genotype;

	schedule = dynamic_cast<FuzzySchedule *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	return new IndividualArrayInt(schedule->getTaskOrder());
}




//=============================================================================
//
//	Class EncoderFJSP_JobOrder
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Encoding method  -----------------------------------------------------
void EncoderFJSP_JobOrder::encode(Solution *solution, Individual *indiv,
	const SharedVars * const svars) const {

	FuzzySchedule *schedule;
	IndividualArrayInt * intIndiv;
	FuzzyProblem * fuzzyProb;
	std::vector<int> genotype;

	schedule = dynamic_cast<FuzzySchedule *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	intIndiv = dynamic_cast<IndividualArrayInt *>(indiv);
	if (intIndiv == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "integer array individuals.";
		throw new FJSPException("Encoding", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	genotype = schedule->getTaskOrder();
	for (size_t i = 0; i < genotype.size(); i++)
		genotype[i] = (*fuzzyProb)[genotype[i]]->job;
	
	intIndiv->setGenotype(genotype);
}



//-----  Encoding method  -----------------------------------------------------
Individual * EncoderFJSP_JobOrder::encode(Solution *solution,
	const SharedVars * const svars) const {


	FuzzySchedule *schedule;
	std::vector<int> taskOrder;
	std::vector<int> genotype;

	schedule = dynamic_cast<FuzzySchedule *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy job shop problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	FuzzyProblem * fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Encoding", errorMsg);
	}

	taskOrder = schedule->getTaskOrder();
	genotype.resize(taskOrder.size());
	for (size_t i = 0; i < taskOrder.size(); i++)
		genotype[i] = (*fuzzyProb)[taskOrder[i]]->job;

	return new IndividualArrayInt(genotype);
}

}
