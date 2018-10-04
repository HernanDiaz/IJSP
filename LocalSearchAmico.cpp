/*
* LocalSearchAmico.cpp
*
*  Created on: Sep 27, 2018
*      Author: jjpalacios
*/

#include "LocalSearchAmico.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class LS_Tabu_Amico
//
//=============================================================================
//=============================================================================
//		CONTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
LS_Tabu_Amico::LS_Tabu_Amico(ParameterDB *parameters)
	: TcycleLabel(FUZZYFW_LS_AMICO_CYCLE), Tcycle(1), cycleControlList(),
	LS_Tabu(parameters) { }


//-----  Copy constructor  ----------------------------------------------------
LS_Tabu_Amico::LS_Tabu_Amico(const LS_Tabu_Amico &source)
	: TcycleLabel(source.TcycleLabel), Tcycle(source.Tcycle),
	cycleControlList(),	LS_Tabu(source) { 
	
	std::pair<Neighbour *, Fitness *> witness;
	for (size_t i = 0; i < source.cycleControlList.size(); i++) {
		witness.first = source.cycleControlList[i].first->clone();
		witness.second = source.cycleControlList[i].second->clone();
		this->cycleControlList.push_back(witness);
	}
}


//-----  Setup method  --------------------------------------------------------
void LS_Tabu_Amico::setup(ParameterDB *parameters) {
	// Loads the maximum number of iterations
	this->Tcycle = parameters->getInteger(this->TcycleLabel, 1);
	
	LS_Tabu::setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Apply method  --------------------------------------------------------
FullSolution LS_Tabu_Amico::apply(const Solution *solution,
	const Fitness *fitness, const SharedVars *svars) {

	int index, nNeighbours, bestNeighbor;
	unsigned int isTabu;
	this->evaluations = 0;
	this->neighbours = 0;
	this->iterations = 0;
	this->badIterations = 0;

	FullSolution current, bestSolution;
	Fitness *estimation, *realValue, *best;
	std::vector<int> randomArray;
	std::pair<Neighbour *, Fitness *> witness;

	this->neighbourhood->setInitialSolution(solution->clone(),
		fitness->clone(), svars);
	current = this->neighbourhood->getCurrentSolution();
	bestSolution.first = solution->clone();
	bestSolution.second = fitness->clone();

	bool improves = true;
	this->cycleCount = 0;
	this->tabuList->clear();
	while (!this->stoppingCriteria()) {
		improves = false;
		bestNeighbor = -1;
		best = NULL;

		nNeighbours =
			this->neighbourhood->findNewNeighbours(svars);
		this->neighbours += nNeighbours;
		this->neighbourhood->sortByEstimation(svars);

		index = 0;
		while (index < nNeighbours) {
			estimation = this->neighbourhood->getEstimation(index, svars);
			isTabu = this->tabuList->isTabu(this->neighbourhood->getNeighbour(index));

			if (!this->estimationFilter || best == NULL || estimation->isBetterThan(best)) {
				if (this->estimationGuided) {
					if ((best == NULL || estimation->isBetterThan(best))
						&& (!isTabu || estimation->isBetterThan(bestSolution.second))) {
						best = estimation;
						bestNeighbor = index;
					}
				}
				else {
					realValue = this->neighbourhood->evaluateNeighbour(index, svars, true);
					this->evaluations++;
					if ((best == NULL || realValue->isBetterThan(best))
						&& (!isTabu || realValue->isBetterThan(bestSolution.second))) {
						best = realValue;
						bestNeighbor = index;
					}
				}
				index++;
			}
			else
				index = nNeighbours;
		}

		if (bestNeighbor >= 0) {
			this->tabuList->addNeighbour(this->neighbourhood->getNeighbour(bestNeighbor));

			if (this->isRepeatedMove(this->neighbourhood->getNeighbour(bestNeighbor),
				this->neighbourhood->getNeighbour(bestNeighbor)->getEvaluatedFitness()))
				cycleCount++;
			else {
				cycleCount = 0;
				witness.first = this->neighbourhood->getNeighbour(bestNeighbor)->clone();
				witness.first->setEvaluation(NULL, NULL);
				witness.second = this->neighbourhood->getNeighbour(bestNeighbor)->getEvaluatedFitness()->clone();
				this->cycleControlList.push_back(witness);
			}

			if (this->neighbourhood->getNeighbour(bestNeighbor)->getEvaluatedFitness()->isBetterThan(current.second))
				this->tabuList->reduceSize(this->tabuList->size()-1);
			this->neighbourhood->acceptNeighbour(bestNeighbor, svars);
			current = this->neighbourhood->getCurrentSolution();
			this->iterations++;
			if (this->estimationGuided)
				this->evaluations++;

			if (current.second->isBetterThan(bestSolution.second)) {
				delete bestSolution.first;
				delete bestSolution.second;
				bestSolution.first = current.first->clone();
				bestSolution.second = current.second->clone();
				improves = true;
				this->badIterations = 0;
				this->tabuList->forceSize(1);
			}
			else
				this->badIterations++;
		}
		// No neihgbours: Dead end
		else
			this->badIterations = this->maxBadIterations;
	}

	return bestSolution;
}



//-----  Method for reset the control list  -----------------------------------
void LS_Tabu_Amico::resetCycleControl() {
	for (unsigned int i = 0; i < this->cycleControlList.size(); i++) {
		delete this->cycleControlList[i].first;
		delete this->cycleControlList[i].second;
	}
	this->cycleControlList.clear();
}



//-----  Method is a repeated move  -------------------------------------------
bool LS_Tabu_Amico::isRepeatedMove(Neighbour *neighbour, Fitness *fitness) {
	for (unsigned int i = 0; i < this->cycleControlList.size(); i++) {
		if (fitness->isEqualTo(this->cycleControlList[i].second)) {
			if (neighbour->isEqualTo(this->cycleControlList[i].first)) {
				return true;
			}
		}
	}
	return false;
}



//-----  Stopping criteria  ---------------------------------------------------
bool LS_Tabu_Amico::stoppingCriteria() {
	if (this->cycleCount >= this->Tcycle)
		return true;
	return LS_Tabu::stoppingCriteria();
}


}
