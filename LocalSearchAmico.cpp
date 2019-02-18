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
	LambdaLabel(FUZZYFW_LS_AMICO_LAMBDA), Lambda(Infi),
	minALabel(FUZZYFW_LS_AMICO_MINA), minA(1),
	minBLabel(FUZZYFW_LS_AMICO_MINB), minB(1),
	maxALabel(FUZZYFW_LS_AMICO_MAXA), maxA(Infi),
	maxBLabel(FUZZYFW_LS_AMICO_MAXB), maxB(Infi),
	LS_Tabu(parameters) { }


//-----  Copy constructor  ----------------------------------------------------
LS_Tabu_Amico::LS_Tabu_Amico(const LS_Tabu_Amico &source)
	: TcycleLabel(source.TcycleLabel), Tcycle(source.Tcycle),
	cycleControlList(),	
	LambdaLabel(source.LambdaLabel), Lambda(source.Lambda),
	minALabel(source.minALabel), minA(source.minA),
	minBLabel(source.minBLabel), minB(source.minB),
	maxALabel(source.maxALabel), maxA(source.maxA),
	maxBLabel(source.maxBLabel), maxB(source.maxB),
	LS_Tabu(source) {
	
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
	
	// Loads the Lambda parameter
	this->Lambda = parameters->getInteger(this->LambdaLabel, Infi);

	// Loads the boundaries for the tabu list sizes
	this->minA = parameters->getInteger(this->minALabel, 1);
	this->minB = parameters->getInteger(this->minBLabel, 1);
	this->maxA = parameters->getInteger(this->maxALabel, Infi);
	this->maxB = parameters->getInteger(this->maxBLabel, Infi);

	LS_Tabu::setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Apply method  --------------------------------------------------------
FullSolution LS_Tabu_Amico::apply(const Solution *solution,
	const Fitness *fitness, const SharedVars *svars) {

	int index, nNeighbours, bestNeighbor;
	FullSolution current, bestSolution;
	Fitness *estimation, *realValue, *best;
	std::vector<int> randomArray;
	Neighbour *bestFoundNeighbor, *lastNeighbour = NULL;
	std::pair<Neighbour *, Fitness *> witness;

	unsigned int isTabu, isLastNeighbour, lambdaCount;
	this->evaluations = 0;
	this->neighbours = 0;
	this->iterations = 0;
	this->badIterations = 0;
	this->cycleCount = 0;
	lambdaCount = 0;

	// Set the initial solution of the neighbourhood
	this->neighbourhood->setInitialSolution(solution->clone(),
		fitness->clone(), svars);
	current = this->neighbourhood->getCurrentSolution();
	bestSolution.first = solution->clone();
	bestSolution.second = fitness->clone();

	// Set the tabu list
	this->updateTabuListBounds(svars);
	this->tabuList->clear();

	bool improves = true;
	while (!this->stoppingCriteria()) {
		improves = false;
		bestNeighbor = -1;
		best = NULL;

		// Update the min/max tabu list sizes
		if (lambdaCount + 1 >= this->Lambda) {
			this->updateTabuListBounds(svars);
			lambdaCount = 0;
		}
		else
			lambdaCount++;

		// Find the neighbours of the current solution
		nNeighbours =
			this->neighbourhood->findNewNeighbours(svars);
		this->neighbours += nNeighbours;
		this->neighbourhood->sortByEstimation(svars);

		// Iterate over the neighbours looking for the best
		index = 0;
		while (index < nNeighbours) {
			estimation = this->neighbourhood->getEstimation(index, svars);
			isTabu = this->tabuList->isTabu(this->neighbourhood->getNeighbour(index));
			if (lastNeighbour != NULL)
				isLastNeighbour = lastNeighbour->isReverse(this->neighbourhood->getNeighbour(index));

			if (!this->estimationFilter || best == NULL || estimation->isBetterThan(best)) {
				if (this->estimationGuided) {
					// Pick the neighbour with the best fitness estimation
					if ((best == NULL || estimation->isBetterThan(best))
						&& (!isTabu || estimation->isBetterThan(bestSolution.second))) {
						best = estimation;
						bestNeighbor = index;
					}
				}
				else {
					// Pick the neighbour with the best real fitness
					realValue = this->neighbourhood->evaluateNeighbour(index, svars, true);
					this->evaluations++;
					if ((best == NULL || realValue->isBetterThan(best))
						&& (!isTabu || realValue->isBetterThan(bestSolution.second))
						&& (lastNeighbour == NULL || !isLastNeighbour)) {
						best = realValue;
						bestNeighbor = index;
					}
				}
				index++;
			}
			else
				index = nNeighbours;
		}

		// If it has found a neighbour to move to...
		if (bestNeighbor >= 0) {
			if (lastNeighbour != NULL)
				delete lastNeighbour;
			bestFoundNeighbor = this->neighbourhood->getNeighbour(bestNeighbor);
			lastNeighbour = bestFoundNeighbor->clone();

			this->tabuList->addNeighbour(bestFoundNeighbor);

			// Check if the solution has been visited in the past
			if (isRepeatedMove(bestFoundNeighbor, bestFoundNeighbor->getEvaluatedFitness()))
				cycleCount++;
			else {
				cycleCount = 0;
				witness.first = bestFoundNeighbor->clone();
				witness.first->setEvaluation(NULL, NULL);
				witness.second = bestFoundNeighbor->getEvaluatedFitness()->clone();
				this->cycleControlList.push_back(witness);
			}

			if (bestFoundNeighbor->getEvaluatedFitness()->isBetterThan(current.second))
				this->tabuList->reduceSize(this->tabuList->size() - 1);

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

	// Free memory
	if (lastNeighbour != NULL)
		delete lastNeighbour;
	resetCycleControl();

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



//-----  Method for updating the tabu list size boundaries  -------------------
void LS_Tabu_Amico::updateTabuListBounds(const SharedVars *svars) {
	unsigned int rand;
	rand = svars->rng->getInteger(minA, minB);
	this->tabuList->setMinSize(rand);
	rand = svars->rng->getInteger(maxA, maxB);
	this->tabuList->setMaxSize(rand);
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
