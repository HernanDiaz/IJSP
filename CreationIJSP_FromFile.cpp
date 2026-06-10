/*
 * CreationIJSP_FromFile.cpp
 *
 *  Created on: Jun 10, 2026
 *      Author: hdiaz
 */

#include <algorithm>
#include <fstream>
#include <sstream>
#include "CreationIJSP_FromFile.h"
#include "IJSPException.h"

namespace IJSP {

//=============================================================================
//		SETUP
//=============================================================================
void CreationFromFileSchedule::setup(FuzzyFW::ParameterDB *parameters) {
	CreationRandomSchedule::setup(parameters);

	this->solutionsDir = parameters->getString(CREATION_SOLUTIONS_DIR);
	if (this->solutionsDir.length() < 1) {
		std::string errorMsg = "Missing parameter '";
		errorMsg += CREATION_SOLUTIONS_DIR;
		errorMsg += "' for the solutions-file creation strategy.";
		throw IJSPException("Creation", errorMsg);
	}
	this->solutions.clear();
	this->loaded = false;
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  loadSolutions  -------------------------------------------------------
void CreationFromFileSchedule::loadSolutions(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	this->loaded = true;
	std::string path = this->solutionsDir + "/"
		+ svars->problem->getName() + "_Sols.csv";

	std::ifstream input(path.c_str());
	if (!input.is_open()) {
		std::string errorMsg = "Cannot open stored solutions file: " + path;
		throw IJSPException("Creation", errorMsg);
	}

	std::string line;
	while (std::getline(input, line)) {
		// Expected: "run;<task ids separated by spaces>;(lo, up)"
		size_t first = line.find(';');
		if (first == std::string::npos)
			continue;
		size_t second = line.find(';', first + 1);
		if (second == std::string::npos)
			continue;

		std::istringstream tasks(line.substr(first + 1, second - first - 1));
		std::vector<int> order;
		int task;
		while (tasks >> task)
			order.push_back(task);
		if (order.empty())
			continue;	// header or malformed line (allows concatenated files)

		// Stored objective midpoint, used only to rank the solutions
		double lower = 0.0, upper = 0.0;
		size_t paren = line.find('(', second);
		if (paren != std::string::npos) {
			std::istringstream objective(line.substr(paren + 1));
			char comma;
			objective >> lower >> comma >> upper;
		}
		this->solutions.push_back(
			std::make_pair((lower + upper) / 2.0, order));
	}

	if (this->solutions.empty()) {
		std::string errorMsg = "No solutions parsed from file: " + path;
		throw IJSPException("Creation", errorMsg);
	}

	std::sort(this->solutions.begin(), this->solutions.end());
}


//-----  createIndividual  ----------------------------------------------------
FuzzyFW::Individual * CreationFromFileSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	if (!this->loaded)
		this->loadSolutions(svars);

	// Keep injecting fresh random material with the configured ratio
	if (this->shouldUseRandom(svars))
		return CreationRandomSchedule::createIndividual(svars);

	// Rank-biased pick: minimum of two uniform draws over the
	// quality-sorted list makes better solutions linearly more likely
	const int draw1 = svars->rng->getInteger(0, this->solutions.size() - 1);
	const int draw2 = svars->rng->getInteger(0, this->solutions.size() - 1);
	std::vector<int> order =
		this->solutions[draw1 < draw2 ? draw1 : draw2].second;

	this->sgs->buildSchedule(svars, order);
	FuzzyFW::Individual *indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());
	return indiv;
}


//-----  getName  -------------------------------------------------------------
std::vector<std::string> CreationFromFileSchedule::getName() const {
	std::vector<std::string> name;
	std::vector<std::string> sgsName = this->sgs->getName();
	name.push_back("Stored solutions (" + this->solutionsDir + ")");
	name.push_back(";Sampling:;rank-biased (min of 2 draws)");
	for (size_t i = 0; i < sgsName.size(); i++)
		name.push_back(";SGS:;" + sgsName[i]);
	name.push_back(";Random ratio:;" + valueToString(this->randomRatio));
	return name;
}

}
