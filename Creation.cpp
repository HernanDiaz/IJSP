/*
* Selection.cpp
*
*  Created on: Aug 2, 2017
*      Author: jjpalacios
*/

#include "Creation.h"

namespace FJSP {

//=============================================================================
//
//	Abstract class Creation
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  createPopulation  ----------------------------------------------------
Population * Creation::createPopulation(const unsigned int popSize,
	const SharedVars *svars) const {
	Population * population = new Population();
	Individual *indiv;

	for (unsigned int i = 0; i < popSize; i++) {
		indiv = this->createIndividual(svars);
		indiv->id = i;
		population->addIndividual(indiv);
	}
	return population;
}





//=============================================================================
//
//	Class CreationRandomSchedule
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Setup method  --------------------------------------------------------
void CreationRandomSchedule::setup(ParameterDB *parameters) {
	Creation::setup(parameters);

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
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
Individual * CreationRandomSchedule::createIndividual(
	const SharedVars *svars) const {

	unsigned int nTasks;
	std::vector<int> permutation, count;
	int rand;
	Individual * indiv;

	// Convert the problem type
	FuzzyProblem * fuzzyProb =
		dynamic_cast<FuzzyProblem *>(svars->problem);
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
			for (int i = rand + 1; i < count.size(); i++)
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
