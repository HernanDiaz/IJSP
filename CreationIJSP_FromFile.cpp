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
#include "ScheduleIJSP.h"

// Local disjunctive-disagreement distance (inlined from the path-relinking
// branch's PathRelinkIJSP::disagreement, to avoid pulling the IPR-TS
// machinery): number of same-machine task pairs whose relative order
// differs between the two schedules.
namespace {
double localDisagreement(const IJSP::ScheduleIJSP *a,
	const IJSP::ScheduleIJSP *b) {
	size_t nTasks = a->taskInfo.size();
	std::vector<int> posA(nTasks, 0), posB(nTasks, 0);
	for (size_t k = 0; k < a->lastTaskMachine.size(); k++) {
		int p = 0;
		for (int t = a->lastTaskMachine[k]; t != -1; t = a->taskInfo[t].mp)
			posA[t] = p++;
		p = 0;
		for (int t = b->lastTaskMachine[k]; t != -1; t = b->taskInfo[t].mp)
			posB[t] = p++;
	}
	double d = 0;
	for (size_t k = 0; k < a->lastTaskMachine.size(); k++) {
		std::vector<int> tasks;
		for (int t = a->lastTaskMachine[k]; t != -1; t = a->taskInfo[t].mp)
			tasks.push_back(t);
		for (size_t i = 0; i < tasks.size(); i++)
			for (size_t j = i + 1; j < tasks.size(); j++) {
				bool ordA = posA[tasks[i]] < posA[tasks[j]];
				bool ordB = posB[tasks[i]] < posB[tasks[j]];
				if (ordA != ordB)
					d += 1.0;
			}
	}
	return d;
}
}

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
	std::string sel = parameters->getStringLower(CREATION_SEED_SELECTION);
	this->useMaxMin = (sel == "maxmin");

	this->solutions.clear();
	this->loaded = false;
	this->maxminOrder.clear();
	this->nextPick = 0;
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

	int idx;
	if (this->useMaxMin) {
		// Hand out the stored solutions in greedy maximum-diversity order
		if (this->maxminOrder.empty())
			this->buildMaxMinOrder(svars);
		idx = this->maxminOrder[this->nextPick % this->maxminOrder.size()];
		this->nextPick++;
	}
	else {
		// Rank-biased pick: minimum of two uniform draws over the
		// quality-sorted list makes better solutions linearly more likely
		const int draw1 = svars->rng->getInteger(0, this->solutions.size() - 1);
		const int draw2 = svars->rng->getInteger(0, this->solutions.size() - 1);
		idx = (draw1 < draw2 ? draw1 : draw2);
	}
	std::vector<int> order = this->solutions[idx].second;

	this->sgs->buildSchedule(svars, order);
	FuzzyFW::Individual *indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());
	return indiv;
}


//-----  buildMaxMinOrder  ----------------------------------------------------
void CreationFromFileSchedule::buildMaxMinOrder(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	const int k = (int)this->solutions.size();

	// Decode every stored solution once, keeping its schedule for distances
	std::vector<ScheduleIJSP *> sched(k, NULL);
	for (int i = 0; i < k; i++) {
		std::vector<int> order = this->solutions[i].second;
		this->sgs->buildSchedule(svars, order);
		sched[i] = dynamic_cast<ScheduleIJSP *>(this->sgs->getSchedule()->clone());
	}

	// Greedy maximum-diversity (p-dispersion): start from the best-quality
	// solution (index 0, the list is sorted), then repeatedly add the one
	// farthest in disjunctive distance from those already chosen.
	std::vector<char> chosen(k, 0);
	std::vector<double> minDist(k, 1.0e300);
	this->maxminOrder.clear();
	this->maxminOrder.push_back(0);
	chosen[0] = 1;
	for (int j = 0; j < k; j++)
		if (!chosen[j])
			minDist[j] = localDisagreement(sched[j], sched[0]);

	for (int step = 1; step < k; step++) {
		int best = -1;
		double bestD = -1.0;
		for (int j = 0; j < k; j++)
			if (!chosen[j] && minDist[j] > bestD) {
				bestD = minDist[j];
				best = j;
			}
		if (best < 0)
			break;
		this->maxminOrder.push_back(best);
		chosen[best] = 1;
		for (int j = 0; j < k; j++)
			if (!chosen[j]) {
				double d = localDisagreement(sched[j], sched[best]);
				if (d < minDist[j])
					minDist[j] = d;
			}
	}

	for (int i = 0; i < k; i++)
		delete sched[i];
	this->nextPick = 0;
}


//-----  getName  -------------------------------------------------------------
std::vector<std::string> CreationFromFileSchedule::getName() const {
	std::vector<std::string> name;
	std::vector<std::string> sgsName = this->sgs->getName();
	name.push_back("Stored solutions (" + this->solutionsDir + ")");
	if (this->useMaxMin)
		name.push_back(";Selection:;max-min diversity (greedy)");
	else
		name.push_back(";Selection:;rank-biased (min of 2 draws)");
	for (size_t i = 0; i < sgsName.size(); i++)
		name.push_back(";SGS:;" + sgsName[i]);
	name.push_back(";Random ratio:;" + valueToString(this->randomRatio));
	return name;
}

}
