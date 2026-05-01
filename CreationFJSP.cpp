/*
* Selection.cpp
*
*  Created on: Aug 2, 2017
*/

#include "CreationFJSP.h"

namespace FJSP {

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

	// Loads the SGS type to create
	std::string sgsType = parameters->getStringLower(this->sgsLabel);
	if (sgsType.length() == 0) {
		std::string errorMsg = "SGS not found. Please, specify a SGS to use";
		errorMsg += " during the evaluation of individuals";
		throw new FJSPException("Evaluation", errorMsg);
	}

	this->sgs = FJSPClassRegister::getSGSObject(sgsType);
	if (this->sgs == NULL) {
		std::string errorMsg = "The introduced SGS is not";
		errorMsg += " recognised: \'" + sgsType + "\'";
		throw new FJSPException("Evaluation", errorMsg);
	}
	this->sgs->setup(parameters);
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
	ProblemFJSP * fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Creation", errorMsg);
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

}
