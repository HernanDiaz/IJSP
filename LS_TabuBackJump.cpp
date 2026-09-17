/*
* LS_TabuBackJump.cpp
*
*  Created on: Sep 17, 2026
*      Author: Hernan Diaz Rodriguez
*/
#include "LS_TabuBackJump.h"

namespace FuzzyFW {

//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
LS_TabuBackJump::LS_TabuBackJump(ParameterDB *parameters)
	: LS_Tabu(parameters), backJumpSizeLabel(FUZZYFW_BACKJUMP_SIZE),
	maxBackJumpPoints(5), maxJumpsLabel(FUZZYFW_BACKJUMP_MAXJUMPS), maxJumps(3)
{
}


//-----  Copy constructor  ----------------------------------------------------
LS_TabuBackJump::LS_TabuBackJump(const LS_TabuBackJump &source)
	: LS_Tabu(source), backJumpSizeLabel(source.backJumpSizeLabel),
	maxBackJumpPoints(source.maxBackJumpPoints),
	maxJumpsLabel(source.maxJumpsLabel), maxJumps(source.maxJumps)
{
	// The stack holds the state of a run in progress, so a copy starts empty.
}


//-----  Setup method  --------------------------------------------------------
void LS_TabuBackJump::setup(ParameterDB *parameters) {
	int size;

	LS_Tabu::setup(parameters);

	size = parameters->getInteger(this->backJumpSizeLabel, -1);
	if (size <= 0) {
		std::cout << "Warning: Back-jump stack size not found. Taking 5";
		std::cout << " by default" << std::endl;
		size = 5;
	}
	this->maxBackJumpPoints = (unsigned int)size;

	size = parameters->getInteger(this->maxJumpsLabel, -1);
	if (size < 0) {
		std::cout << "Warning: Max. back jumps per call not found. Taking 3";
		std::cout << " by default" << std::endl;
		size = 3;
	}
	this->maxJumps = (unsigned int)size;
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Clear the stack  -----------------------------------------------------
void LS_TabuBackJump::clearBackJumpStack() {
	while (!this->backJumpStack.empty()) {
		delete this->backJumpStack.back().solution;
		delete this->backJumpStack.back().fitness;
		delete this->backJumpStack.back().move;
		this->backJumpStack.pop_back();
	}
}


//-----  Remember a decision point  -------------------------------------------
void LS_TabuBackJump::pushBackJumpPoint(const Solution *solution,
	const Fitness *fitness, const Neighbour *move) {
	BackJumpPoint point;

	// Drop the oldest point: the most recent ones sit closest to the region
	// the search is currently working on.
	if (this->backJumpStack.size() >= this->maxBackJumpPoints) {
		delete this->backJumpStack.front().solution;
		delete this->backJumpStack.front().fitness;
		delete this->backJumpStack.front().move;
		this->backJumpStack.pop_front();
	}

	point.solution = solution->clone();
	point.fitness = fitness->clone();
	point.move = move->clone();
	this->backJumpStack.push_back(point);
}


//-----  Apply method  --------------------------------------------------------
FullSolution LS_TabuBackJump::apply(const Solution *solution,
	const Fitness *fitness, const SharedVars *svars) {

	int index, nNeighbours, bestNeighbor;
	FullSolution current, bestSolution;
	Fitness *estimation, *realValue, *best;
	Neighbour *lastNeighbour = NULL;
	Solution *startSolution;
	Fitness *startFitness;
	Neighbour *forbidden = NULL;
	unsigned int isTabu, isLastNeighbour;

	this->evaluations = 0;
	this->neighbours = 0;
	this->iterations = 0;
	this->badIterations = 0;
	clock_t _startClock = clock();
	this->runtime = 0;

	// Wall-clock safety limit, as in LS_Tabu: guards against clock() issues and
	// against an inner loop that never reaches its stopping condition.
	struct timespec _lsWallStart;
	clock_gettime(CLOCK_MONOTONIC, &_lsWallStart);
	double _lsWallLimit = (this->maxTime > 0) ? (this->maxTime * 4.0 + 5.0) : 15.0;
	bool outOfTime = false;
	unsigned int jumps = 0;

	this->clearBackJumpStack();
	bestSolution.first = solution->clone();
	bestSolution.second = fitness->clone();
	startSolution = solution->clone();
	startFitness = fitness->clone();

	// ---- Outer loop: one pass per back jump --------------------------------
	while (true) {
		this->neighbourhood->setInitialSolution(startSolution, startFitness,
			svars);
		current = this->neighbourhood->getCurrentSolution();

		// The forbidden move is the one taken from this point last time, so
		// seeding the list with it sends the search down another branch.
		this->tabuList->clear();
		if (forbidden != NULL) {
			this->tabuList->addNeighbour(forbidden);
			delete forbidden;
			forbidden = NULL;
		}
		this->badIterations = 0;

		// ---- Inner loop: ordinary tabu search ------------------------------
		while (!this->stoppingCriteria()) {
			this->runtime = clock() - _startClock;
			{
				struct timespec _lsWallNow;
				clock_gettime(CLOCK_MONOTONIC, &_lsWallNow);
				double _lsElapsed = (_lsWallNow.tv_sec - _lsWallStart.tv_sec)
					+ (_lsWallNow.tv_nsec - _lsWallStart.tv_nsec) * 1e-9;
				if (_lsElapsed >= _lsWallLimit) {
					fprintf(stderr, "[LS_SAFETY] Wall-clock limit %.1fs hit "
						"after %d iters (cpu=%.3fs)\n", _lsWallLimit,
						this->iterations,
						(clock() - _startClock) / (double)CLOCKS_PER_SEC);
					fflush(stderr);
					outOfTime = true;
					break;
				}
			}

			bestNeighbor = -1;
			best = NULL;

			nNeighbours = this->neighbourhood->findNewNeighbours(svars);
			this->neighbours += nNeighbours;
			this->neighbourhood->sortByEstimation(svars);

			index = 0;
			while (index < nNeighbours) {
				estimation = this->neighbourhood->getEstimation(index, svars);
				isTabu = this->tabuList->isTabu(
					this->neighbourhood->getNeighbour(index));
				if (lastNeighbour != NULL)
					isLastNeighbour = lastNeighbour->isReverse(
						this->neighbourhood->getNeighbour(index));

				if (!this->estimationFilter || best == NULL
					|| estimation->isBetterThan(best)) {
					if (this->estimationGuided) {
						if ((best == NULL || estimation->isBetterThan(best))
							&& (!isTabu
								|| estimation->isBetterThan(bestSolution.second))) {
							best = estimation;
							bestNeighbor = index;
						}
					}
					else {
						realValue = this->neighbourhood->evaluateNeighbour(index,
							svars, false);
						this->evaluations++;
						if (realValue != NULL
							&& (best == NULL || realValue->isBetterThan(best))
							&& (!isTabu
								|| realValue->isBetterThan(bestSolution.second))
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

			if (bestNeighbor >= 0) {
				Neighbour *chosen = this->neighbourhood->getNeighbour(bestNeighbor);

				// The search is about to leave a solution that is as good as
				// the best one known, by a move that does not improve it. That
				// is a branching decision worth coming back to.
				if (!current.second->isBetterThan(bestSolution.second)
					&& !bestSolution.second->isBetterThan(current.second)
					&& !best->isBetterThan(current.second))
					this->pushBackJumpPoint(current.first, current.second,
						chosen);

				if (lastNeighbour != NULL)
					delete lastNeighbour;
				lastNeighbour = chosen->clone();

				this->tabuList->addNeighbour(chosen);
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
					this->badIterations = 0;
				}
				else
					this->badIterations++;
			}
			// No neighbours: dead end
			else
				this->badIterations = this->maxBadIterations;
		}

		// ---- Jump back, or give up -----------------------------------------
		// LocalSearch::stoppingCriteria() covers the time, iteration and
		// evaluation budget shared by the whole call, so an exhausted budget
		// ends the search rather than starting another pass.
		if (outOfTime || LocalSearch::stoppingCriteria()
			|| this->backJumpStack.empty() || jumps >= this->maxJumps)
			break;
		jumps++;

		BackJumpPoint point = this->backJumpStack.back();
		this->backJumpStack.pop_back();
		startSolution = point.solution;
		startFitness = point.fitness;
		forbidden = point.move;

		if (lastNeighbour != NULL) {
			delete lastNeighbour;
			lastNeighbour = NULL;
		}
	}

	if (lastNeighbour != NULL)
		delete lastNeighbour;
	if (forbidden != NULL)
		delete forbidden;
	this->clearBackJumpStack();

	return bestSolution;
}

}
