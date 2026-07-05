/*
 * NeighbourhoodIJSP_N2ME.cpp
 *
 *  Created on: Jul 5, 2026
 *      Author: hdiaz
 */

#include "NeighbourhoodIJSP_N2ME.h"
#include "EvaluationIJSP_Energy.h"

namespace IJSP {

//-----  Set the initial solution (requires lexicographic fitness)  ----------
void NB_ParallelN2ME_MakespanEnergyIJSP::setInitialSolution(
	FuzzyFW::Solution *solution, FuzzyFW::Fitness *fitness,
	const FuzzyFW::SharedVars *svars) {

	FuzzyFW::FitnessLexicographic *lex =
		dynamic_cast<FuzzyFW::FitnessLexicographic *>(fitness);
	if (lex == NULL) {
		std::string errorMsg = "The N2ME neighbourhood requires a ";
		errorMsg += "lexicographic (makespan, energy) fitness. Use the ";
		errorMsg += "ijsp.makespan-energy evaluation.";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	this->problem = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (this->problem == NULL) {
		std::string errorMsg = "This neighbourhood works only with ";
		errorMsg += "Interval problems.";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	if (this->currentLexFitness != NULL)
		delete this->currentLexFitness;
	this->currentLexFitness = lex;

	// The base class keeps the makespan component (pruning machinery)
	NB_ParallelN2_MakespanIJSP::setInitialSolution(solution,
		lex->getFitness(0)->clone(), svars);
}


//-----  Lexicographic wrapper of the makespan estimation  --------------------
FuzzyFW::Fitness *NB_ParallelN2ME_MakespanEnergyIJSP::getEstimation(
	const unsigned int idx, const FuzzyFW::SharedVars *svars) {

	FuzzyFW::Fitness *base =
		NB_ParallelN2_MakespanIJSP::getEstimation(idx, svars);

	if (idx >= this->lexEstimations.size())
		this->lexEstimations.resize(idx + 1);

	FuzzyFW::FitnessLexicographic *wrapped =
		new FuzzyFW::FitnessLexicographic();
	wrapped->addFitness(base->clone());
	// Optimistic energy: an estimation must never discard an energy move
	wrapped->addFitness(new FuzzyFW::FitnessInterval(
		FuzzyFW::Interval(0, 0), false));
	this->lexEstimations[idx].reset(wrapped);
	return wrapped;
}


//-----  Find neighbours: N2 arcs + energy-critical arcs  ---------------------
unsigned int NB_ParallelN2ME_MakespanEnergyIJSP::findNewNeighbours(
	const FuzzyFW::SharedVars *svars) {

	unsigned int taskId, nTasks;
	FuzzyFW::Interval currentMakespan;
	ScheduledTaskInfo task, mp, jp;
	ScheduledTaskInfo mpmp, ms;
	std::queue<int> taskQueue;
	std::vector<char> added;
	std::vector<char> visited;

	currentMakespan = this->currentFitness->getValue();
	nTasks = this->schedule->getScheduledTasks();
	added.resize(nTasks, false);

	if (nTasks < 2)
		return 1;

	this->numNeighbours = 0;

	// Helper to append the reversal of the machine arc (x, y), deduplicated
	// by its left endpoint (each task has a single machine successor)
	auto addArc = [&](int x, int y) {
		if (added[x])
			return;
		if (this->numNeighbours < this->neighbours.size()
			&& this->neighbours[this->numNeighbours] != nullptr)
			this->neighbours[this->numNeighbours]->setValues(x, y);
		else
			this->neighbours.push_back(
				std::make_unique<NeighbourIJSP_Arc>(x, y));
		this->numNeighbours++;
		added[x] = true;
	};

	for (short int comp = 1; comp <= 2; comp++) {

		//---  Pass 1: N2 arcs (boundary arcs of extreme critical blocks)  ----
		visited.assign(nTasks, false);
		for (size_t i = 0; i < this->schedule->lastTaskMachine.size(); i++) {
			if (this->schedule->getCTMachine(i).EqualComponent(currentMakespan, comp)) {
				visited[this->schedule->lastTaskMachine[i]] = true;
				taskQueue.push(this->schedule->lastTaskMachine[i]);
			}
		}
		while (taskQueue.size() > 0) {
			taskId = taskQueue.front();
			taskQueue.pop();
			task = this->schedule->taskInfo[taskId];
			if (task.mp != -1 && task.mp != task.task->jp) {
				mp = this->schedule->taskInfo[task.mp];
				if ((mp.head + mp.task->p).EqualComponent(task.head, comp)) {
					if (!visited[task.mp]) {
						taskQueue.push(task.mp);
						visited[task.mp] = true;
					}
					if (!added[task.mp]) {
						if (mp.mp != -1)
							mpmp = this->schedule->taskInfo[mp.mp];
						if (task.ms != -1)
							ms = this->schedule->taskInfo[task.ms];
						// N2 boundary-arc condition
						if (mp.mp == -1 || task.ms == -1
							|| !(mpmp.head + mpmp.task->p).EqualComponent(mp.head, comp)
							|| (!(task.head + task.task->p).EqualComponent(ms.head, comp)
								|| visited[task.ms] == false)) {
							addArc(task.mp, taskId);
						}
					}
				}
			}
			if (task.task->jp != -1) {
				jp = this->schedule->taskInfo[task.task->jp];
				if ((jp.head + jp.task->p).EqualComponent(task.head, comp)) {
					if (!visited[task.task->jp]) {
						taskQueue.push(task.task->jp);
						visited[task.task->jp] = true;
					}
				}
			}
		}

		//---  Pass 2: energy-critical arcs E(sigma)  -------------------------
		// Seeds: last task of every machine, plus right endpoints of
		// positive idle gaps (component comp). Walk backwards over tight
		// arcs; every tight MACHINE arc on a sustaining chain is a candidate
		// (no boundary filter: feasibility only needs tightness).
		visited.assign(nTasks, false);
		for (size_t i = 0; i < this->schedule->lastTaskMachine.size(); i++) {
			int last = this->schedule->lastTaskMachine[i];
			if (last < 0)
				continue;
			if (!visited[last]) {
				visited[last] = true;
				taskQueue.push(last);
			}
			// Gap successors on this machine
			int t = last;
			while (this->schedule->taskInfo[t].mp != -1) {
				int pred = this->schedule->taskInfo[t].mp;
				const ScheduledTaskInfo &ti = this->schedule->taskInfo[t];
				const ScheduledTaskInfo &pi = this->schedule->taskInfo[pred];
				double gap = (comp == 1)
					? ti.head.a - (pi.head.a + pi.task->p.a)
					: ti.head.b - (pi.head.b + pi.task->p.b);
				if (gap > 0 && !visited[t]) {
					visited[t] = true;
					taskQueue.push(t);
				}
				t = pred;
			}
		}
		while (taskQueue.size() > 0) {
			taskId = taskQueue.front();
			taskQueue.pop();
			task = this->schedule->taskInfo[taskId];
			if (task.mp != -1 && task.mp != task.task->jp) {
				mp = this->schedule->taskInfo[task.mp];
				if ((mp.head + mp.task->p).EqualComponent(task.head, comp)) {
					// Tight machine arc on a sustaining chain: candidate
					addArc(task.mp, taskId);
					if (!visited[task.mp]) {
						taskQueue.push(task.mp);
						visited[task.mp] = true;
					}
				}
			}
			if (task.task->jp != -1) {
				jp = this->schedule->taskInfo[task.task->jp];
				if ((jp.head + jp.task->p).EqualComponent(task.head, comp)) {
					if (!visited[task.task->jp]) {
						taskQueue.push(task.task->jp);
						visited[task.task->jp] = true;
					}
				}
			}
		}
	}
	return this->numNeighbours;
}


//-----  Fully evaluate the neighbour (lexicographic fitness)  ----------------
FuzzyFW::Fitness *NB_ParallelN2ME_MakespanEnergyIJSP::evaluateNeighbour(
	const unsigned int idx, const FuzzyFW::SharedVars *svars,
	const bool improvement) {

	// Base evaluation builds the new schedule and its makespan fitness
	// (with the inherited pruning against the makespan component)
	FuzzyFW::Fitness *base =
		NB_ParallelN2_MakespanIJSP::evaluateNeighbour(idx, svars, improvement);
	if (base == NULL)
		return NULL;

	ScheduleIJSP *newSolution = dynamic_cast<ScheduleIJSP *>(
		this->neighbours[idx]->getEvaluation());
	FuzzyFW::FitnessInterval *makespan =
		dynamic_cast<FuzzyFW::FitnessInterval *>(base);
	if (newSolution == NULL || makespan == NULL) {
		std::string errorMsg = "N2ME: unexpected evaluation types from the ";
		errorMsg += "base neighbourhood.";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	FuzzyFW::FitnessLexicographic *lex = new FuzzyFW::FitnessLexicographic();
	lex->addFitness(makespan->clone());
	lex->addFitness(new FuzzyFW::FitnessInterval(
		EvaluationIJSP_Energy::computeNPE(newSolution, this->problem), false));

	// Replace the stored evaluated fitness by the lexicographic one
	this->neighbours[idx]->setEvaluation(newSolution->clone(), lex);
	return this->neighbours[idx]->getEvaluatedFitness();
}


//-----  Accept the neighbour  ------------------------------------------------
void NB_ParallelN2ME_MakespanEnergyIJSP::acceptNeighbour(
	const unsigned int idx, const FuzzyFW::SharedVars *svars) {

	if (idx > this->numNeighbours || this->neighbours[idx] == nullptr) {
		std::string errorMsg = "Trying to access a non-existing neighbour";
		throw IJSPException("Neighbourhood", errorMsg);
	}
	if (!this->neighbours[idx]->isEvaluated())
		this->evaluateNeighbour(idx, svars, false);

	FuzzyFW::FitnessLexicographic *lex =
		dynamic_cast<FuzzyFW::FitnessLexicographic *>(
			this->neighbours[idx]->getEvaluatedFitness());
	if (lex == NULL) {
		std::string errorMsg = "N2ME: accepted neighbour does not carry a ";
		errorMsg += "lexicographic fitness.";
		throw IJSPException("Neighbourhood", errorMsg);
	}

	// Update both fitness views: the full lexicographic one, and the
	// makespan component in the base field (pruning machinery). The base
	// acceptNeighbour cannot be reused: it would cast the stored
	// lexicographic fitness to FitnessInterval (NULL + leak per accept),
	// so its schedule/tails update is replicated below.
	if (this->currentLexFitness != NULL)
		delete this->currentLexFitness;
	this->currentLexFitness =
		dynamic_cast<FuzzyFW::FitnessLexicographic *>(lex->clone());
	if (this->currentFitness != nullptr)
		delete this->currentFitness;
	this->currentFitness = dynamic_cast<FuzzyFW::FitnessInterval *>(
		this->currentLexFitness->getFitness(0)->clone());

	if (this->schedule != nullptr)
		delete this->schedule;
	this->schedule = dynamic_cast<ScheduleIJSP *>(
		this->neighbours[idx]->getEvaluation()->clone());

	// Tails update (verbatim from the N2 acceptNeighbour)
	int job;
	int z, msz, jsz;
	FuzzyFW::Interval newTail;
	std::queue<int> taskQueue;
	NeighbourIJSP_Arc *arc = this->neighbours[idx].get();
	std::vector<int> tailsUpdated;
	tailsUpdated.resize(this->schedule->getScheduledTasks(), 0);

	taskQueue.push(arc->x);
	tailsUpdated[arc->x] = 1;
	taskQueue.push(arc->y);
	tailsUpdated[arc->y] = 1;

	while (!taskQueue.empty()) {
		z = taskQueue.front();
		taskQueue.pop();
		msz = this->schedule->taskInfo[z].ms;
		job = this->schedule->taskInfo[z].task->job;
		if (this->schedule->lastTaskJob[job] == z)
			jsz = -1;
		else jsz = this->schedule->taskInfo[z].task->js;

		if (jsz != -1 && msz != -1)
			newTail = maximum(this->schedule->taskInfo[msz].task->p + this->tails[msz],
				this->schedule->taskInfo[jsz].task->p + this->tails[jsz],
				FuzzyFW::Interval::M_COMPONENT);
		else if (msz != -1)
			newTail = this->schedule->taskInfo[msz].task->p + this->tails[msz];
		else if (jsz != -1)
			newTail = this->schedule->taskInfo[jsz].task->p + this->tails[jsz];
		if ((msz != -1 || jsz != -1) &&
			!(this->tails[z].isEqualTo(newTail, FuzzyFW::Interval::C_COMPONENT))) {
			this->tails[z] = newTail;
			if (this->schedule->taskInfo[z].mp != -1 && !tailsUpdated[this->schedule->taskInfo[z].mp]) {
				taskQueue.push(this->schedule->taskInfo[z].mp);
				tailsUpdated[this->schedule->taskInfo[z].mp]++;
			}
			if (this->schedule->taskInfo[z].task->jp != -1 && !tailsUpdated[this->schedule->taskInfo[z].task->jp]) {
				taskQueue.push(this->schedule->taskInfo[z].task->jp);
				tailsUpdated[this->schedule->taskInfo[z].task->jp]++;
			}
		}
	}
}

} // namespace IJSP
