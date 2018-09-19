/*
* Population.cpp
*
*  Created on: June 30, 2017
*      Author: Juan Jose Palacios
*/

#include "Population.h"

namespace FJSP {

//=============================================================================
//
//	Class Population
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
//=====  Copy constructor  ====================================================
Population::Population(const Population &source)
: sorted(source.sorted) {

	this->individual.resize(source.individual.size());
	this->order.resize(source.order.size());

	// Clone all the individuals from the source population
	for (unsigned int i = 0; i < source.size(); i++) {
		this->individual[i] = source.individual[i]->clone();
		this->order[i] = source.order[i];
	}
}


//=====  Destructor  ==========================================================
Population::~Population() {
	for(size_t i=0; i < this->individual.size(); i++)
		delete this->individual[i];
	this->individual.clear();
	this->order.clear();
}


//=====  Clear Method  ========================================================
void Population::clear() {
	for(size_t i=0; i < individual.size(); i++)
		delete this->individual[i];
	this->individual.clear();
	this->order.clear();
	this->sorted = true;
}



//=============================================================================
//		GET/SET METHODS
//=============================================================================
//=====  Get an individual  ===================================================
Individual * Population::getIndividual(const unsigned int index) const {
	if (index < 0 || index > this->individual.size())
		throw new FJSPException("Population",
			"Access to a non-existing individual");
	return this->individual[index];
}


//=====  Update an individual  ================================================
Individual * Population::replaceIndividual(const unsigned int index,
	Individual *newInd) {

	if (index < 0 || index > (int)this->individual.size())
		throw new FJSPException("Population",
			"Access to a non-existing individual");
	Individual *old = this->individual[index];
	this->individual[index] = newInd;
	this->sorted = false;
	return old;
}


//=====  Get Fitness  =========================================================
Fitness * Population::getFitness(const unsigned int index) const {
	if (index < 0 || index > (int)this->individual.size())
		throw new FJSPException("Population",
			"Access to a non-existing individual");
	return this->individual[index]->getFitness();
}


//=====  Set Fitness  =========================================================
void Population::setFitness(const unsigned int index, Fitness * fitness) {
	if (index < 0 || index > (int)this->individual.size())
		throw new FJSPException("Population",
			"Access to a non-existing individual");
	this->individual[index]->updateFitness(fitness);
}


//===== Get Best  =============================================================
Individual * Population::getBest(const SharedVars *svars, const unsigned int k) {
	if (k < 0 || k >= (int)this->individual.size()) {
		throw new FJSPException("Population",
			"Access to a non-existing individual");
	}
	this->sort(svars->rng);
	return this->individual[order[k]];
}


//===== Who is the Best  ======================================================
int Population::whoIsBest(const SharedVars *svars, const unsigned int k) {
	if (k < 0 || k >= (int)this->individual.size()) {
		throw new FJSPException("Population",
			"Access to a non-existing individual");
	}
	this->sort(svars->rng);
	return this->order[k];
}


//===== Get Average Fitness  ==================================================
double Population::getAverageFitness() const {
	double sum=0.0;
	TFN tfnValue;
	for(size_t i=0; i < this->individual.size(); i++) {
		if(!this->individual[i]->isEvaluated())
			return -1.0;

		sum += this->individual[i]->getFitness()->toDouble();
	}
	return sum / this->individual.size();
}



//=============================================================================
//		METHODS
//=============================================================================
//===== Add Individual  =======================================================
void Population::addIndividual(Individual *individual) {
	this->individual.push_back(individual);
	this->order.push_back(this->individual.size()-1);
	this->sorted = false;
}


//===== Rename  ===============================================================
void Population::rename() {
	for (size_t i = 0; i < this->individual.size(); i++)
		this->individual[i]->id = i;
}



//===== Sort  =================================================================
void Population::sort(Random *rng, const bool force) {
	if (this->sorted && !force)
		return;
	this->quickSort(0, (int)this->individual.size() - 1, rng);
	this->sorted = true;
}


//===== QuickSort  ==============================================================
void Population::quickSort(const int left, const int right, Random *rng) {
	int pivot, pos;

	if (left >= right)
		return;

	pivot = rng->getInteger(left, right);

	std::swap(order[pivot], order[right]);
	pos = left;
	for (int i = left; i < right; i++) {
		if(this->individual[order[i]]->getFitness()->isBetterThan(
			this->individual[order[right]]->getFitness())) {
			std::swap(order[i], order[pos]);
			pos++;
		}
		else if((this->individual[order[i]]->getFitness()->isEqualTo(
			this->individual[order[right]]->getFitness()))
			&& order[i] < order[right]) {
			std::swap(order[i], order[pos]);
			pos++;
		}
	}

	std::swap(order[pos], order[right]);
	this->quickSort(left, pos - 1, rng);
	this->quickSort(pos + 1, right, rng);
}

}
