/*
 * EncoderJSP.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "EncoderJSP.h"

namespace JSP {

//=============================================================================
//
//	Class EncoderJSP_Order
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Encoding method  -----------------------------------------------------
void EncoderJSP_Order::encode(FuzzyFW::Solution *solution,
	FuzzyFW::Individual *indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {

	ScheduleJSP *schedule;
	FuzzyFW::IndividualArrayInt * intIndiv;
	
	schedule = dynamic_cast<ScheduleJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval job shop problems.";
		throw JSPException("Encoding", errorMsg);
	}

	intIndiv = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (intIndiv == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "integer array individuals.";
		throw JSPException("Encoding", errorMsg);
	}

	intIndiv->updateGenotype(schedule->getTaskOrder(svars->rng));
}



//-----  Encoding method  -----------------------------------------------------
FuzzyFW::Individual * EncoderJSP_Order::encode(FuzzyFW::Solution *solution,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {

	ScheduleJSP *schedule;
	std::vector<unsigned int> taskOrder;
	std::vector<int> genotype;

	schedule = dynamic_cast<ScheduleJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval job shop problems.";
		throw JSPException("Encoding", errorMsg);
	}

	return new FuzzyFW::IndividualArrayInt(schedule->getTaskOrder(svars->rng));
}




//=============================================================================
//
//	Class EncoderJSP_JobOrder
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  Encoding method  -----------------------------------------------------
void EncoderJSP_JobOrder::encode(FuzzyFW::Solution *solution,
	FuzzyFW::Individual *indiv,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {

	ScheduleJSP *schedule;
	FuzzyFW::IndividualArrayInt * intIndiv;
	ProblemJSP * fuzzyProb;
	std::vector<int> genotype;

	schedule = dynamic_cast<ScheduleJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval job shop problems.";
		throw JSPException("Encoding", errorMsg);
	}

	intIndiv = dynamic_cast<FuzzyFW::IndividualArrayInt *>(indiv);
	if (intIndiv == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "integer array individuals.";
		throw JSPException("Encoding", errorMsg);
	}

	fuzzyProb =
		dynamic_cast<ProblemJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw JSPException("Encoding", errorMsg);
	}

	genotype = schedule->getTaskOrder(svars->rng);
	for (size_t i = 0; i < genotype.size(); i++)
		genotype[i] = (*fuzzyProb)[genotype[i]]->job;
	
	intIndiv->updateGenotype(genotype);
}



//-----  Encoding method  -----------------------------------------------------
FuzzyFW::Individual * EncoderJSP_JobOrder::encode(
	FuzzyFW::Solution *solution,
	const FuzzyFW::SharedVarsEvolutionary * const svars) const {


	ScheduleJSP *schedule;
	std::vector<int> taskOrder;
	std::vector<int> genotype;

	schedule = dynamic_cast<ScheduleJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval job shop problems.";
		throw JSPException("Encoding", errorMsg);
	}

	ProblemJSP * fuzzyProb =
		dynamic_cast<ProblemJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw JSPException("Encoding", errorMsg);
	}

	taskOrder = schedule->getTaskOrder(svars->rng);
	genotype.resize(taskOrder.size());
	for (size_t i = 0; i < taskOrder.size(); i++)
		genotype[i] = (*fuzzyProb)[taskOrder[i]]->job;

	return new FuzzyFW::IndividualArrayInt(genotype);
}

}
