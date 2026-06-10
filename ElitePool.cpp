/*
 * ElitePool.cpp
 *
 *  Created on: Jun 10, 2026
 *      Author: hdiaz
 */

#include "ElitePool.h"
#include "IJSPException.h"

namespace IJSP {

//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
void ElitePool::configure(const unsigned int poolCapacity,
	const double minDistance) {
	this->clear();
	this->capacity = poolCapacity;
	this->minDistanceFrac = minDistance;
}


void ElitePool::clear() {
	for (size_t i = 0; i < this->elites.size(); i++)
		delete this->elites[i];
	this->elites.clear();
	this->instancePairs = 0.0;
}



//=============================================================================
//		GET/SET METHODS
//=============================================================================
int ElitePool::bestIndex() const {
	int best = -1;
	for (size_t i = 0; i < this->elites.size(); i++)
		if (best < 0 || this->elites[i]->getFitness()
			->isBetterThan(this->elites[best]->getFitness()))
			best = (int)i;
	return best;
}


int ElitePool::worstIndex() const {
	int worst = -1;
	for (size_t i = 0; i < this->elites.size(); i++)
		if (worst < 0 || this->elites[worst]->getFitness()
			->isBetterThan(this->elites[i]->getFitness()))
			worst = (int)i;
	return worst;
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  tryInsert  -----------------------------------------------------------
bool ElitePool::tryInsert(FuzzyFW::Individual *candidate) {
	if (this->instancePairs <= 0.0)
		this->instancePairs =
			PathRelinkIJSP::totalPairs(this->scheduleOf(candidate));

	// Locate the nearest existing elite
	int nearest = -1;
	double nearestDistance = 1.0e300;
	for (size_t i = 0; i < this->elites.size(); i++) {
		double distance = this->distanceFrac(candidate, this->elites[i]);
		if (distance < nearestDistance) {
			nearestDistance = distance;
			nearest = (int)i;
		}
	}

	// Too close to an existing elite: it can only replace that elite
	if (nearest >= 0 && nearestDistance < this->minDistanceFrac) {
		if (candidate->getFitness()
			->isBetterThan(this->elites[nearest]->getFitness())) {
			delete this->elites[nearest];
			this->elites[nearest] = candidate;
			return true;
		}
		delete candidate;
		return false;
	}

	// Distant enough: enter while below capacity, else replace the worst
	if (this->size() < this->capacity) {
		this->elites.push_back(candidate);
		return true;
	}

	const int worst = this->worstIndex();
	if (worst >= 0 && candidate->getFitness()
		->isBetterThan(this->elites[worst]->getFitness())) {
		delete this->elites[worst];
		this->elites[worst] = candidate;
		return true;
	}

	delete candidate;
	return false;
}


//-----  forceInsert  ---------------------------------------------------------
void ElitePool::forceInsert(FuzzyFW::Individual *candidate) {
	if (this->instancePairs <= 0.0)
		this->instancePairs =
			PathRelinkIJSP::totalPairs(this->scheduleOf(candidate));

	if (this->size() < this->capacity) {
		this->elites.push_back(candidate);
		return;
	}

	const int worst = this->worstIndex();
	if (worst >= 0 && candidate->getFitness()
		->isBetterThan(this->elites[worst]->getFitness())) {
		delete this->elites[worst];
		this->elites[worst] = candidate;
		return;
	}

	delete candidate;
}


//-----  averageDistanceFrac  -------------------------------------------------
double ElitePool::averageDistanceFrac() const {
	if (this->elites.size() < 2)
		return 1.0;

	double total = 0.0;
	unsigned int pairs = 0;
	for (size_t i = 0; i + 1 < this->elites.size(); i++)
		for (size_t j = i + 1; j < this->elites.size(); j++) {
			total += this->distanceFrac(this->elites[i], this->elites[j]);
			pairs++;
		}
	return total / pairs;
}


//-----  averageFitness  ------------------------------------------------------
double ElitePool::averageFitness() const {
	if (this->elites.empty())
		return 0.0;

	double total = 0.0;
	for (size_t i = 0; i < this->elites.size(); i++)
		total += this->elites[i]->getFitness()->toDouble();
	return total / this->elites.size();
}


//-----  dropWorstHalf  -------------------------------------------------------
void ElitePool::dropWorstHalf() {
	const unsigned int keep = ((unsigned int)this->elites.size() + 1) / 2;
	std::vector<FuzzyFW::Individual *> kept;

	while (kept.size() < keep && !this->elites.empty()) {
		int best = this->bestIndex();
		kept.push_back(this->elites[best]);
		this->elites.erase(this->elites.begin() + best);
	}
	for (size_t i = 0; i < this->elites.size(); i++)
		delete this->elites[i];
	this->elites = kept;
}


//-----  distanceFrac  --------------------------------------------------------
double ElitePool::distanceFrac(const FuzzyFW::Individual *a,
	const FuzzyFW::Individual *b) const {
	return PathRelinkIJSP::disagreement(this->scheduleOf(a), this->scheduleOf(b))
		/ this->instancePairs;
}


//-----  scheduleOf  ----------------------------------------------------------
const ScheduleIJSP * ElitePool::scheduleOf(
	const FuzzyFW::Individual *individual) const {
	const ScheduleIJSP *schedule =
		dynamic_cast<const ScheduleIJSP *>(individual->getPhenotype());
	if (schedule == NULL) {
		std::string errorMsg = "The elite pool works only with IJSP schedules.";
		throw IJSPException("Elite Pool", errorMsg);
	}
	return schedule;
}

}
