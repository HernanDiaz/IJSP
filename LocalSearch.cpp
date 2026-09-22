/*
* LocalSearch.cpp
*
*  Created on: Oct 11, 2017
*/

#include "LocalSearch.h"
#include "NeighbourhoodJSP_Base.h"
#include "NeighbourJSP.h"
#include <cstdlib>
#include <time.h>

namespace FuzzyFW {

//=============================================================================
//
//	Abstract class LocalSearch
//
//=============================================================================
//=============================================================================
//		CONTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
LocalSearch::LocalSearch(ParameterDB *parameters)
	: iterationLabel(FUZZYFW_LOCAL_SEARCH_ITER),
	evaluationLabel(FUZZYFW_LOCAL_SEARCH_EVAL),
	evaluations(0), timeLabel(FUZZYFW_LOCAL_SEARCH_TIME), maxTime(0.0),
	neighbours(0), iterations(0), neighbourhood(NULL),
	guideLabel(FUZZYFW_LOCAL_SEARCH_DRIVE), estimationGuided(false),
	filterLabel(FUZZYFW_LOCAL_SEARCH_FILTER), estimationFilter(false),
	tailsLabel(FUZZYFW_LOCAL_SEARCH_TAILS), fullTails(false),
	deadEndLabel(FUZZYFW_LOCAL_SEARCH_DEADEND), deadEndEscape(false)
	{
	if (parameters != NULL)
		this->setup(parameters);
}


//-----  Copy constructor  ----------------------------------------------------
LocalSearch::LocalSearch(const LocalSearch &source)
	: iterationLabel(source.iterationLabel), iterations(source.iterations),
	evaluationLabel(source.evaluationLabel), evaluations(source.evaluations),
	maxIterations(source.maxIterations), maxEvaluations(source.maxEvaluations),
	timeLabel(source.timeLabel), maxTime(source.maxTime),
	guideLabel(source.guideLabel), estimationGuided(source.estimationGuided),
	filterLabel(source.filterLabel), estimationFilter(source.estimationFilter),
	tailsLabel(source.tailsLabel), fullTails(source.fullTails),
	deadEndLabel(source.deadEndLabel), deadEndEscape(source.deadEndEscape),
	neighbours(source.neighbours) {

	if (source.neighbourhood != NULL)
		neighbourhood = source.neighbourhood->clone();
	else neighbourhood = NULL;
}



//-----  Setup method  --------------------------------------------------------
void LocalSearch::setup(ParameterDB *parameters) {
	// Loads the maximum number of iterations
	this->maxIterations = parameters->getInteger(this->iterationLabel, -1);
	this->maxEvaluations = parameters->getInteger(this->evaluationLabel, -1);
	// Loads the maximumtime run
	this->maxTime = parameters->getDouble(this->timeLabel, -1.0);

	// Loads the main drive
	this->estimationGuided = parameters->getBoolean(this->guideLabel, false);

	// Loads the filter mechanism
	this->estimationFilter = parameters->getBoolean(this->filterLabel, false);

	// How the tails feeding the heads&tails estimate are maintained.
	//   "incremental" (default, unchanged): the backward sweep of
	//       acceptNeighbour, which only continues while a tail changes
	//   "full"       : recomputed from scratch before every estimation
	// Measured 2026-09-22 on ta29: with the incremental tails the estimate
	// exceeds the true value of the neighbour in 882,481 of 1,362,270
	// evaluations, 64.8 %, so it is not a lower bound, the neighbourhood is
	// sorted wrongly and the estimation filter prunes away moves that are
	// actually better. Recomputed from scratch, the violations are 0 of
	// 1,352,160 and the throughput cost is under 1 %.
	this->fullTails =
		(parameters->getStringLower(this->tailsLabel).compare("full") == 0);
	this->deadEndEscape =
		(parameters->getStringLower(this->deadEndLabel).compare("escape") == 0);
}


//-----  Stopping criteria  ---------------------------------------------------
bool LocalSearch::stoppingCriteria() {
	if (this->maxEvaluations >= 0 &&
		this->evaluations >= this->maxEvaluations)
		return true;
	if (this->maxIterations >= 0 &&
		this->iterations >= this->maxIterations)
		return true;

	double currentRuntime = this->runtime / (double)CLOCKS_PER_SEC;
	if (this->maxTime > 0 &&
		compareDouble(currentRuntime, this->maxTime) >= 0)
		return true;

	return false;
}





//=============================================================================
//
//	Class LS_HillClimbing
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
FullSolution LS_HillClimbing::apply(const Solution *solution,
	const Fitness *fitness, const SharedVars *svars) {

	int next, index, nNeighbours;
	this->evaluations = 0;
	this->neighbours = 0;
	this->iterations = 0;
	clock_t _startClock = clock();
	this->runtime = 0;

	FullSolution current;
	Fitness *estimation, *realValue;
	std::vector<int> randomArray;

	this->neighbourhood->setInitialSolution(solution->clone(),
		fitness->clone(), svars);
	current = this->neighbourhood->getCurrentSolution();
	//std::ofstream outfile;
	//outfile.open("LocalSearchdebugN1.txt", std::ios_base::app); // append instead of overwrite
	//outfile << "HillClimbing for solution " << solution->toString() << std::endl;
	//outfile.close();
	bool improves = true;
	while (improves && !this->stoppingCriteria()) {
		this->runtime = clock() - _startClock;
		improves = false;
		nNeighbours =
			this->neighbourhood->findNewNeighbours(svars);
		this->neighbours += nNeighbours;
		randomArray = svars->rng->getRandomVector(0, nNeighbours - 1);

		while (!improves && nNeighbours > 0) {
			// Pick a random number
			next = svars->rng->getInteger(0, nNeighbours - 1);
			index = randomArray[next];

			// Estimate the quality of a neighbour
			estimation = this->neighbourhood->getEstimation(index, svars);
			
			if (!this->estimationFilter || estimation->isBetterThan(current.second)) {
				if (this->estimationGuided) {
					if (estimation->isBetterThan(current.second)) {
						improves = true;
						this->neighbourhood->acceptNeighbour(index, svars);
						current = this->neighbourhood->getCurrentSolution();
						this->evaluations++;
						this->iterations++;
					}
				}
				else {
					realValue = this->neighbourhood->evaluateNeighbour(index, svars, true);

					this->evaluations++;
					if (realValue!= NULL && realValue->isBetterThan(current.second)) {
						improves = true;
						this->neighbourhood->acceptNeighbour(index, svars);
						current = this->neighbourhood->getCurrentSolution();
						this->iterations++;
					}
				}
			}
			if (!improves) {
				//this->neighbourhood->discardNeighbour(next);
				//std::swap(randomArray[next], randomArray[randomArray.size() - 1]); BUG
				std::swap(randomArray[next], randomArray[nNeighbours -1]);
				nNeighbours--;
			}
		}
	}

	current.first = current.first->clone();
	current.second = current.second->clone();
	return current;
}





//=============================================================================
//
//	Class LS_GradientDescent
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
FullSolution LS_GradientDescent::apply(const Solution *solution,
	const Fitness *fitness, const SharedVars *svars) {

	int index, nNeighbours, bestNeighbor;
	this->evaluations = 0;
	this->neighbours = 0;
	this->iterations = 0;
	clock_t _startClock = clock();
	this->runtime = 0;

	FullSolution current;
	Fitness *estimation, *realValue, *best;
	std::vector<int> randomArray;

	this->neighbourhood->setInitialSolution(solution->clone(),
		fitness->clone(), svars);
	current = this->neighbourhood->getCurrentSolution();

	bool improves = true;
	while (improves && !this->stoppingCriteria()) {
		this->runtime = clock() - _startClock;
		improves = false;
		bestNeighbor = -1;
		best = current.second;

		nNeighbours =
			this->neighbourhood->findNewNeighbours(svars);
		this->neighbours += nNeighbours;
		this->neighbourhood->sortByEstimation(svars);

		index = 0;
		while (index < nNeighbours) {
			estimation = this->neighbourhood->getEstimation(index, svars);

			if (!this->estimationFilter || estimation->isBetterThan(best)) {
				if (this->estimationGuided) {
					if (estimation->isBetterThan(best)) {
						best = estimation;
						bestNeighbor = index;
					}
				}
				else {
					realValue = this->neighbourhood->evaluateNeighbour(index, svars, true);
					this->evaluations++;
					if (realValue!= NULL && realValue->isBetterThan(best)) {
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
			improves = true;
			this->neighbourhood->acceptNeighbour(bestNeighbor, svars);
			current = this->neighbourhood->getCurrentSolution();
			this->iterations++;
			if (this->estimationGuided)
				this->evaluations++;
		}
	}

	current.first = current.first->clone();
	current.second = current.second->clone();
	return current;
}





//=============================================================================
//
//	Class LS_Tabu
//
//=============================================================================
//=============================================================================
//		CONTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
LS_Tabu::LS_Tabu(ParameterDB *parameters)
	: LocalSearch(parameters), maxBadIterations(0),
	badIterationsLabel(FUZZYFW_LOCAL_SEARCH_TABUITER),
	badIterations(0)
{
	tabuList = new TabuList(parameters);
}


//-----  Copy constructor  ----------------------------------------------------
LS_Tabu::LS_Tabu(const LS_Tabu &source)
	: LocalSearch(source), maxBadIterations(source.maxBadIterations),
	badIterationsLabel(badIterationsLabel),
	badIterations(source.badIterations)
{
	tabuList = source.tabuList->clone();
}



//-----  Setup method  --------------------------------------------------------
void LS_Tabu::setup(ParameterDB *parameters) {
	LocalSearch::setup(parameters);
	this->tieBreakLabel = FUZZYFW_LOCAL_SEARCH_TIEBREAK;
	this->tieBreakFrequency =
		(parameters->getStringLower(this->tieBreakLabel).compare("frequency") == 0);
	this->arcUses.clear();
	this->arcUsesTasks = 0;
	// Configure the tabu list with the parameters from the setup file
	this->tabuList->setup(parameters);
	// Loads the maximum number of iterations without improvement
	this->maxBadIterations = parameters->getInteger(this->badIterationsLabel, -1);

	if (this->maxBadIterations < 0) {
		std::string errorMsg = "Number of iterations without improvement not";
		errorMsg += " specified";
		throw FuzzyFWException("Tabu Search",errorMsg);
	}
}


//-----  Stopping criteria  ---------------------------------------------------
bool LS_Tabu::stoppingCriteria() {
	if (LocalSearch::stoppingCriteria())
		return true;
	if (this->badIterations >= this->maxBadIterations)
		return true;

	return false;
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Apply method  --------------------------------------------------------
unsigned long LS_Tabu::diagTieSum = 0;
unsigned long LS_Tabu::diagIters = 0;
unsigned long LS_Tabu::diagScanned = 0;
unsigned long LS_Tabu::diagBoundBreaks = 0;
unsigned long LS_Tabu::diagTieMax = 0;
unsigned long LS_Tabu::deepCalls = 0;
unsigned long LS_Tabu::deepIters = 0;
unsigned long LS_Tabu::deepDeadEnd = 0;
unsigned long LS_Tabu::deepBadStop = 0;
unsigned long LS_Tabu::deepTimeStop = 0;
unsigned long LS_Tabu::deepEscapes = 0;

FullSolution LS_Tabu::apply(const Solution *solution,
	const Fitness *fitness, const SharedVars *svars) {

	int index, nNeighbours, bestNeighbor;
	FullSolution current, bestSolution;
	Fitness *estimation, *realValue, *best;
	std::vector<int> randomArray;
	Neighbour *lastNeighbour = NULL;

	unsigned int isTabu, isLastNeighbour;
	this->evaluations = 0;
	this->neighbours = 0;
	this->iterations = 0;
	this->badIterations = 0;
	clock_t _startClock = clock();
	this->runtime = 0;
	// Wall-clock safety limit for LS (guards against clock() issues or infinite inner loops)
	struct timespec _lsWallStart;
	clock_gettime(CLOCK_MONOTONIC, &_lsWallStart);
	double _lsWallLimit = (this->maxTime > 0) ? (this->maxTime * 4.0 + 5.0) : 15.0;

	// Set the initial solution of the nieghbourhood
	this->neighbourhood->setInitialSolution(solution->clone(),
		fitness->clone(), svars);
	current = this->neighbourhood->getCurrentSolution();
	bestSolution.first = solution->clone();
	bestSolution.second = fitness->clone();

	bool improves = true;
	this->tabuList->clear();
	if (this->tieBreakFrequency) {
		JSP::NB_ParallelBase_MakespanJSP *nb0 =
			dynamic_cast<JSP::NB_ParallelBase_MakespanJSP *>(this->neighbourhood);
		if (nb0 != NULL) {
			unsigned int n = (unsigned int)nb0->getScheduledTasksCount();
			if (n > 0 && this->arcUsesTasks != n) {
				this->arcUsesTasks = n;
				this->arcUses.assign((size_t)n * n, 0u);
			}
		}
	}

	while (!this->stoppingCriteria()) {
		this->runtime = clock() - _startClock;
		// Wall-clock safety: escape if LS has been running too long (guards against clock() issues)
		{
			struct timespec _lsWallNow;
			clock_gettime(CLOCK_MONOTONIC, &_lsWallNow);
			double _lsElapsed = (_lsWallNow.tv_sec - _lsWallStart.tv_sec)
				+ (_lsWallNow.tv_nsec - _lsWallStart.tv_nsec) * 1e-9;
			if (_lsElapsed >= _lsWallLimit) {
				fprintf(stderr, "[LS_SAFETY] Wall-clock limit %.1fs hit after %d iters (cpu=%.3fs)\n",
					_lsWallLimit, this->iterations,
					(clock() - _startClock) / (double)CLOCKS_PER_SEC);
				fflush(stderr);
				break;
			}
		}
		improves = false;
		bestNeighbor = -1;
		best = NULL;

		nNeighbours =
			this->neighbourhood->findNewNeighbours(svars);
		this->neighbours += nNeighbours;
		// DIAGNOSTIC: with N2_FULL_TAILS set, rebuild every tail from scratch
		// before the estimations are computed, so that the estimator works on
		// exact data instead of on whatever the incremental maintenance left.
		{
			const bool fullTails = this->fullTails;
			if (fullTails) {
				JSP::NB_ParallelBase_MakespanJSP *nb =
					dynamic_cast<JSP::NB_ParallelBase_MakespanJSP *>(this->neighbourhood);
				if (nb != NULL) nb->recomputeAllTails();
			}
		}
		this->neighbourhood->sortByEstimation(svars);

		index = 0;
		int escapeNeighbor = -1;          // best ignoring the tabu status
		Fitness *escapeValue = NULL;
		unsigned long diagTies = 0;               // eligible neighbours at the best value
		double diagBestSeen = 0.0;
		bool diagHaveBest = false;
		while (index < nNeighbours) {
			estimation = this->neighbourhood->getEstimation(index, svars);
			isTabu = this->tabuList->isTabu(this->neighbourhood->getNeighbour(index));
			if(lastNeighbour != NULL)
				isLastNeighbour = lastNeighbour->isReverse(this->neighbourhood->getNeighbour(index));

			// With directed tie-breaking the sweep must not stop at an
			// estimate merely EQUAL to the best real value, or the tied moves
			// stay invisible. Sound because the cell also sets
			// localsearch.tails = full, which makes the estimate a true lower
			// bound (I-004: 0 violations of 1.35 million).
			bool passFilter;
			if (!this->estimationFilter || best == NULL)
				passFilter = true;
			else if (this->tieBreakFrequency)
				passFilter = !estimation->isWorseThan(best);
			else
				passFilter = estimation->isBetterThan(best);
			if (passFilter) {
				if (this->estimationGuided) {
					if ((best == NULL || estimation->isBetterThan(best))
						&& (!isTabu || estimation->isBetterThan(bestSolution.second))) {
						best = estimation;
						bestNeighbor = index;
					}
				}
				else {
					realValue = this->neighbourhood->evaluateNeighbour(index, svars, false);
					this->evaluations++;
					// DIAGNOSTIC: is the estimate a lower bound, and how many
					// eligible neighbours tie at the best value?
					if (realValue != NULL) {
						LS_Tabu::diagScanned++;
						if (estimation != NULL
							&& estimation->isWorseThan(realValue))
							LS_Tabu::diagBoundBreaks++;
						bool eligible =
							(!isTabu || realValue->isBetterThan(bestSolution.second))
							&& (lastNeighbour == NULL || !isLastNeighbour);
						if (eligible) {
							double v = realValue->toDouble();
							if (!diagHaveBest || v < diagBestSeen) {
								diagBestSeen = v; diagHaveBest = true; diagTies = 1;
							}
							else if (v == diagBestSeen)
								diagTies++;
						}
					}
					if (this->deadEndEscape && realValue != NULL
						&& (escapeValue == NULL
							|| realValue->isBetterThan(escapeValue))) {
						escapeValue = realValue;
						escapeNeighbor = index;
					}
					bool eligibleMove = (realValue != NULL)
						&& (!isTabu || realValue->isBetterThan(bestSolution.second))
						&& (lastNeighbour == NULL || !isLastNeighbour);
					if (eligibleMove && best != NULL && this->tieBreakFrequency
						&& realValue->isEqualTo(best) && bestNeighbor >= 0) {
						// A tie at the best value: keep the less used arc.
						const JSP::NeighbourJSP_Arc *cand = dynamic_cast<const JSP::NeighbourJSP_Arc *>(
							this->neighbourhood->getNeighbour(index));
						const JSP::NeighbourJSP_Arc *held = dynamic_cast<const JSP::NeighbourJSP_Arc *>(
							this->neighbourhood->getNeighbour(bestNeighbor));
						if (cand != NULL && held != NULL && this->arcUsesTasks > 0) {
							unsigned int ci = cand->x * this->arcUsesTasks + cand->y;
							unsigned int hi = held->x * this->arcUsesTasks + held->y;
							if (ci < this->arcUses.size() && hi < this->arcUses.size()
								&& this->arcUses[ci] < this->arcUses[hi]) {
								best = realValue;
								bestNeighbor = index;
							}
						}
					}
					else if (eligibleMove
						&& (best == NULL || realValue->isBetterThan(best))) {
						best = realValue;
						bestNeighbor = index;
					}
				}
				index++;
			}
			else
				index = nNeighbours;
		}

		if (diagHaveBest) {
			LS_Tabu::diagIters++;
			LS_Tabu::diagTieSum += diagTies;
			if (diagTies > LS_Tabu::diagTieMax) LS_Tabu::diagTieMax = diagTies;
		}
		if (bestNeighbor >= 0) {
			if (lastNeighbour != NULL)
				delete lastNeighbour;
			lastNeighbour = this->neighbourhood->getNeighbour(bestNeighbor)->clone();

			if (this->tieBreakFrequency && this->arcUsesTasks > 0) {
				const JSP::NeighbourJSP_Arc *taken = dynamic_cast<const JSP::NeighbourJSP_Arc *>(
					this->neighbourhood->getNeighbour(bestNeighbor));
				if (taken != NULL) {
					unsigned int ti = taken->x * this->arcUsesTasks + taken->y;
					if (ti < this->arcUses.size()) this->arcUses[ti]++;
				}
			}
			this->tabuList->addNeighbour(this->neighbourhood->getNeighbour(bestNeighbor));
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
			}
			else
				this->badIterations++;
		}
		// No admissible neighbour. Every move is tabu without aspiring, or is
		// the reverse of the last one. Measured 2026-09-22: this is how EVERY
		// deep call ends, after 75 to 92 moves, so it and not
		// localsearch.bad-iterations is what caps the depth of this search.
		else if (this->deadEndEscape && escapeNeighbor >= 0) {
			// The classical way out: take the best move anyway and carry on.
			if (this->maxBadIterations > 100) LS_Tabu::deepEscapes++;
			if (lastNeighbour != NULL) delete lastNeighbour;
			lastNeighbour =
				this->neighbourhood->getNeighbour(escapeNeighbor)->clone();
			this->tabuList->addNeighbour(
				this->neighbourhood->getNeighbour(escapeNeighbor));
			this->neighbourhood->acceptNeighbour(escapeNeighbor, svars);
			current = this->neighbourhood->getCurrentSolution();
			this->iterations++;
			if (current.second->isBetterThan(bestSolution.second)) {
				delete bestSolution.first;
				delete bestSolution.second;
				bestSolution.first = current.first->clone();
				bestSolution.second = current.second->clone();
				improves = true;
				this->badIterations = 0;
			}
			else
				this->badIterations++;
		}
		else {
			if (this->maxBadIterations > 100) LS_Tabu::deepDeadEnd++;
			this->badIterations = this->maxBadIterations;
		}
	}

	if (this->maxBadIterations > 100) {
		LS_Tabu::deepCalls++;
		LS_Tabu::deepIters += this->iterations;
		double elapsed = this->runtime / (double)CLOCKS_PER_SEC;
		if (this->maxTime > 0 && elapsed >= this->maxTime * 0.9)
			LS_Tabu::deepTimeStop++;
		else if (this->badIterations >= this->maxBadIterations)
			LS_Tabu::deepBadStop++;
	}
	if(lastNeighbour != NULL)
		delete lastNeighbour;
	return bestSolution;
}

}

