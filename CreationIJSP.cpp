/*
 * CreationIJSP.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CreationIJSP_Base.h"

namespace IJSP {

//=============================================================================
//
//	Class CreationRandomSchedule
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Setup method  --------------------------------------------------------
void CreationRandomSchedule::setup(FuzzyFW::ParameterDB *parameters) {
	Creation::setup(parameters);
	this->randomRatio = parameters->getDouble(CREATION_RANDOM_RATIO, 0);

	std::string sgsType = parameters->getStringLower(this->sgsLabel);
	if (sgsType.length() == 0) {
		std::string errorMsg = "SGS not found. Please, specify a SGS to use";
		errorMsg += " during the evaluation of individuals";
		throw IJSPException("Evaluation", errorMsg);
	}
	this->sgs.reset(IJSPClassRegister::getSGSObject(sgsType));
	if (this->sgs == NULL) {
		std::string errorMsg = "The introduced SGS is not";
		errorMsg += " recognised: \'" + sgsType + "\'";
		throw IJSPException("Evaluation", errorMsg);
	}
	this->sgs->setup(parameters);
}

std::vector<std::string> CreationRandomSchedule::buildStrategyName(
	const std::string &name) const {
	std::vector<std::string> setup;
	std::vector<std::string> sgsName = this->sgs->getName();
	setup.push_back(name);
	setup.push_back(";RandomRatio:;" + valueToString(this->randomRatio));
	setup.push_back(";SGS:;" + sgsName[0]);
	for (size_t i = 1; i < sgsName.size(); i++)
		setup.push_back(";" + sgsName[i]);
	return setup;
}

bool CreationRandomSchedule::shouldUseRandom(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	if (this->randomRatio <= 0) return false;
	return svars->rng->getInteger(0, 100) < static_cast<int>(this->randomRatio * 100);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationRandomSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	std::vector<int> permutation, count;
	int rand;
	FuzzyFW::Individual * indiv;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw IJSPException("Creation", errorMsg);
	}

	// Find the first task of each job
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++)
		if (fuzzyProb->getNumberTasks(i) > 0)
			count.push_back(fuzzyProb->getTaskId(i, 0));

	// Build an array with repetitions
	while (count.size() > 0) {
		rand = svars->rng->getInteger(0, count.size() - 1);
		permutation.push_back(count[rand]);

		// Pass to the next task of the job
		count[rand] = (*fuzzyProb)[count[rand]]->js;
		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++)
				count[i - 1] = count[i];
			count.pop_back();
		}
	}

	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());

	return indiv;
}

} // namespace IJSP
