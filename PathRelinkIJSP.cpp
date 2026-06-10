/*
 * PathRelinkIJSP.cpp
 *
 *  Created on: Jun 10, 2026
 *      Author: hdiaz
 */

#include "PathRelinkIJSP.h"
#include "LocalSearchClassRegister.h"
#include "IJSPException.h"

namespace IJSP {

//=============================================================================
//		SETUP
//=============================================================================
void PathRelinkIJSP::setup(FuzzyFW::ParameterDB *parameters) {
	std::string name = parameters->getStringLower(PR_NEIGHBOURHOOD);
	if (name.length() < 1)
		name = PR_NEIGHBOURHOOD_DEFAULT;

	delete this->neighbourhood;
	this->neighbourhood =
		FuzzyFW::LocalSearchClassRegister::getNeighbourhoodObject(name);
	if (this->neighbourhood == NULL) {
		std::string errorMsg = "Invalid path relinking neighbourhood: " + name;
		throw IJSPException("Path Relinking", errorMsg);
	}
	this->neighbourhood->setup(parameters);

	this->maxSteps = parameters->getInteger(PR_MAX_STEPS, -1);
}



//=============================================================================
//		GET/SET METHODS
//=============================================================================
std::vector<std::string> PathRelinkIJSP::getName() {
	std::vector<std::string> setup, nb;
	setup.push_back("Path Relinking (extreme critical arcs)");
	nb = this->neighbourhood->getName();
	for (size_t i = 0; i < nb.size(); i++)
		setup.push_back(";" + nb[i]);
	if (this->maxSteps >= 0)
		setup.push_back(";Max. Steps:;" + valueToString(this->maxSteps));
	return setup;
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  machinePositions  ----------------------------------------------------
std::vector<int> PathRelinkIJSP::machinePositions(const ScheduleIJSP *schedule) {
	const size_t nTasks = schedule->taskInfo.size();
	std::vector<int> positions(nTasks, 0);

	for (size_t t = 0; t < nTasks; t++) {
		if (schedule->taskInfo[t].mp != -1)
			continue;	// not the head of a machine sequence
		int current = (int)t;
		int position = 0;
		while (current != -1) {
			positions[current] = position++;
			current = schedule->taskInfo[current].ms;
		}
	}
	return positions;
}


//-----  disagreement  --------------------------------------------------------
double PathRelinkIJSP::disagreement(const ScheduleIJSP *a,
	const ScheduleIJSP *b) {

	const std::vector<int> posB = machinePositions(b);
	const size_t nTasks = a->taskInfo.size();
	std::vector<int> sequence;
	double count = 0.0;

	// Walk each machine sequence of `a`, map it through `b`'s positions and
	// count inversions (machine sequences are short: O(len^2) is enough)
	for (size_t t = 0; t < nTasks; t++) {
		if (a->taskInfo[t].mp != -1)
			continue;
		sequence.clear();
		int current = (int)t;
		while (current != -1) {
			sequence.push_back(posB[current]);
			current = a->taskInfo[current].ms;
		}
		for (size_t i = 0; i + 1 < sequence.size(); i++)
			for (size_t j = i + 1; j < sequence.size(); j++)
				if (sequence[i] > sequence[j])
					count += 1.0;
	}
	return count;
}


//-----  totalPairs  ----------------------------------------------------------
double PathRelinkIJSP::totalPairs(const ScheduleIJSP *schedule) {
	const size_t nTasks = schedule->taskInfo.size();
	double total = 0.0;

	for (size_t t = 0; t < nTasks; t++) {
		if (schedule->taskInfo[t].mp != -1)
			continue;
		double length = 0.0;
		int current = (int)t;
		while (current != -1) {
			length += 1.0;
			current = schedule->taskInfo[current].ms;
		}
		total += length * (length - 1.0) / 2.0;
	}
	return total;
}


//-----  relink  --------------------------------------------------------------
FuzzyFW::FullSolution PathRelinkIJSP::relink(const FuzzyFW::Solution *from,
	const FuzzyFW::Fitness *fromFitness, const ScheduleIJSP *guide,
	const FuzzyFW::SharedVars *svars) {

	FuzzyFW::FullSolution best, current;
	this->steps = 0;
	this->evaluations = 0;

	const std::vector<int> guidePos = machinePositions(guide);

	// The neighbourhood owns the clones it receives
	this->neighbourhood->setInitialSolution(from->clone(), fromFitness->clone(),
		svars);
	best.first = from->clone();
	best.second = fromFitness->clone();

	while (this->maxSteps < 0 || (int)this->steps < this->maxSteps) {
		const unsigned int nNeighbours =
			this->neighbourhood->findNewNeighbours(svars);
		if (nNeighbours == 0)
			break;
		this->neighbourhood->sortByEstimation(svars);

		// Best-estimated feasible arc that disagrees with the guide
		int chosen = -1;
		for (unsigned int i = 0; i < nNeighbours && chosen < 0; i++) {
			NeighbourIJSP_Arc *arc = dynamic_cast<NeighbourIJSP_Arc *>(
				this->neighbourhood->getNeighbour(i));
			if (arc == NULL || arc->tipo != 0)
				continue;	// the walk only takes single-arc reversals
			if (guidePos[arc->y] >= guidePos[arc->x])
				continue;	// already agrees with the guide
			if (this->neighbourhood->evaluateNeighbour(i, svars, false) == NULL)
				continue;	// infeasible (cannot happen on extreme critical arcs)
			this->evaluations++;
			chosen = (int)i;
		}
		if (chosen < 0)
			break;	// no extreme-critical disagreement left: walk is over

		this->neighbourhood->acceptNeighbour(chosen, svars);
		this->steps++;

		current = this->neighbourhood->getCurrentSolution();
		if (current.second->isBetterThan(best.second)) {
			delete best.first;
			delete best.second;
			best.first = current.first->clone();
			best.second = current.second->clone();
		}
	}

	return best;
}

}
