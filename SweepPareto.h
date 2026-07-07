/*
 * SweepPareto.h
 *
 *  Created on: Jul 7, 2026
 *      Author: hdiaz
 */
#pragma once

#include "ABCPSOPareto.h"
#include "EvaluationIJSP_MakespanEnergy.h"
#include "NeighbourhoodIJSP_N2ME.h"
#include "ScheduleIJSP.h"
#include "ProblemIJSP.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SweepPareto
//
//=============================================================================
/**
* Epsilon-sweep memetic front generator (see SWEEP_DESIGN.md).
* Phase A: the proven lexicographic ABC-PSO anchor (inherited engine).
* Phase B: a ladder of capped tabu searches — for each makespan allowance
* cap = (1+eps)*C_A the N2ME neighbourhood is put in goal mode (primary
* clamped to max(Cmax, cap)), so under-cap solutions tie on makespan and
* the NPE tie-break drives the search; each level is warm-started from the
* previous one and its best point is offered to the archive, dumped to
* _Front.csv at the end of the run.
*
* Requires evaluation ijsp.makespan-energy and neighbourhood
* ijsp.makespan-energy.n2me.
*
* @author hdiaz
*/
class SweepPareto : public ABCPSOPareto {
public:
	SweepPareto(ParameterDB *params = NULL) : ABCPSOPareto(params) { }
	virtual ~SweepPareto() { }

	virtual std::pair<Solution *, Objective *> run(Problem *problem,
		std::string signature, std::string logFolder, int rngSeed) {

		static const int PERMIL[] = { 5, 10, 15, 20, 30, 40, 50, 60, 80, 100 };
		static const int LEVELS = 10;
		static const int STALL_ROUNDS = 4;

		this->archive.clear();

		// ---- Phase A: anchor (skip the parent dump; we dump after B) ----
		std::pair<Solution *, Objective *> best =
			ArtificialBeeColonyPSO::run(problem, signature, logFolder,
				rngSeed);

		IJSP::NB_ParallelN2ME_MakespanEnergyIJSP *nb = dynamic_cast<
			IJSP::NB_ParallelN2ME_MakespanEnergyIJSP *>(this->neighbourhood);
		IJSP::EvaluationIJSP_MakespanEnergy *ev = dynamic_cast<
			IJSP::EvaluationIJSP_MakespanEnergy *>(this->evaluator);
		IJSP::ProblemIJSP *prob =
			dynamic_cast<IJSP::ProblemIJSP *>(problem);
		FitnessLexicographic *bestLex =
			dynamic_cast<FitnessLexicographic *>(best.second);
		if (nb == NULL || ev == NULL || prob == NULL || bestLex == NULL) {
			std::string errorMsg = "SweepPareto requires the N2ME ";
			errorMsg += "neighbourhood and the makespan-energy evaluation.";
			throw FuzzyFWException("SweepPareto", errorMsg);
		}
		Interval cstar = dynamic_cast<FitnessInterval *>(
			bestLex->getFitness(0))->getValue();

		// ---- Phase B: capped ladder, warm-started ----
		Solution *sigma = best.first->clone();
		for (int k = 0; k < LEVELS; k++) {
			Interval cap(cstar.a * (1000.0 + PERMIL[k]) / 1000.0,
				cstar.b * (1000.0 + PERMIL[k]) / 1000.0);
			nb->setGoalCap(cap);

			for (int r = 0; r < STALL_ROUNDS; r++) {
				// Clamped fitness of the current seed
				IJSP::ScheduleIJSP *sch =
					dynamic_cast<IJSP::ScheduleIJSP *>(sigma);
				FitnessLexicographic *cur = dynamic_cast<
					FitnessLexicographic *>(ev->evaluateSchedule(sch, prob));
				FitnessInterval *c0 = dynamic_cast<FitnessInterval *>(
					cur->getFitness(0));
				Interval mk = c0->getValue();
				c0->setValue(Interval(std::max(mk.a, cap.a),
					std::max(mk.b, cap.b)));

				FullSolution out = this->localSearch->apply(sigma, cur,
					this->sharedVariables);
				bool improved = out.second->isBetterThan(cur);
				delete cur;
				if (improved) {
					delete sigma;
					sigma = out.first;
					delete out.second;
				}
				else {
					delete out.first;
					delete out.second;
					break;
				}
			}
			// Archive the REAL (unclamped) point of this level
			IJSP::ScheduleIJSP *sch =
				dynamic_cast<IJSP::ScheduleIJSP *>(sigma);
			FitnessLexicographic *real = dynamic_cast<
				FitnessLexicographic *>(ev->evaluateSchedule(sch, prob));
			this->archive.offer(sigma->toString(), real);
			delete real;
		}
		nb->clearGoalCap();
		delete sigma;

		if (logFolder.length() > 0)
			this->archive.dump(logFolder + signature + "_Front.csv");
		return best;
	}
};

}
