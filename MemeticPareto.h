/*
 * MemeticPareto.h
 *
 *  Created on: Jul 7, 2026
 *      Author: hdiaz
 */
#pragma once

#include "MemeticAlgorithm.h"
#include "ParetoArchive.h"
#include "ReplacementNSGA2.h"
#include "FitnessMO.h"
#include "EvaluationIJSP_MakespanEnergy.h"
#include "NeighbourhoodIJSP_N2ME.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class MemeticPareto
//
//=============================================================================
/**
* Pareto variant of the memetic algorithm with TRUE NSGA-II survival: the
* generational engine calls the replacement operator every generation, so
* with replacement = nsga2 the population itself spreads along the front
* (unlike the ABC engine, whose trial-based survival stays lexicographic).
* The attached bounded archive is fed by the NSGA-II replacement and dumped
* to <logFolder><signature>_Front.csv after every run.
*
* Requires the lexicographic (makespan, energy) evaluation, and a
* lexicographic-aware local search neighbourhood (N2ME) if local search is
* enabled.
*
* @author hdiaz
*/
class MemeticPareto : public MemeticAlgorithm {
protected:
	ParetoArchive archive;

public:
	MemeticPareto(ParameterDB *params = NULL)
		: MemeticAlgorithm(params), archive(100) { }

	virtual ~MemeticPareto() { }

	virtual void prepareToRun(ParameterDB *params) {
		MemeticAlgorithm::prepareToRun(params);
		ReplacementNSGA2 *nsga2 =
			dynamic_cast<ReplacementNSGA2 *>(this->replacement);
		if (nsga2 == NULL) {
			std::string errorMsg = "The MEMETIC-Pareto algorithm requires ";
			errorMsg += "the NSGA-II replacement (replacement = nsga2).";
			throw FuzzyFWException("MemeticPareto", errorMsg);
		}
		nsga2->setArchive(&this->archive);
	}

	virtual std::pair<Solution *, Objective *> run(Problem *problem,
		std::string signature, std::string logFolder, int rngSeed) {
		this->archive.clear();
		std::pair<Solution *, Objective *> best =
			MemeticAlgorithm::run(problem, signature, logFolder, rngSeed);
		if (logFolder.length() > 0)
			this->archive.dump(logFolder + signature + "_Front.csv");
		return best;
	}

	/**
	* Lexicographic boundary for the local search (same pattern as the
	* ABC-PSO engine): a makespan-only neighbourhood receives the primary
	* component and the full fitness is rebuilt from the optimised
	* schedule; a lexicographic-aware neighbourhood (N2ME) receives the
	* full fitness. The improved individual is offered to the archive.
	*/
	virtual void applyLocalSearch(Population *population,
		const unsigned int individualIdx) {

		Individual *target = population->getIndividual(individualIdx);
		FitnessLexicographic *lexFitness =
			dynamic_cast<FitnessLexicographic *>(target->getFitness());
		bool lexAwareNB = dynamic_cast<
			IJSP::NB_ParallelN2ME_MakespanEnergyIJSP *>(this->neighbourhood)
			!= NULL;
		Fitness *lsFitness = (lexFitness != NULL && !lexAwareNB) ?
			lexFitness->getFitness(0) : target->getFitness();

		FullSolution optimised = this->localSearch->apply(
			target->getPhenotype(), lsFitness, this->sharedVariables);

		this->evaluationsLS += this->localSearch->getEvaluations();
		this->neighboursLS += this->localSearch->getNeighbours();
		this->iterationsLS += this->localSearch->getIterations();
		this->callsLS++;

		if (lexFitness != NULL && !lexAwareNB) {
			IJSP::EvaluationIJSP_MakespanEnergy *lexEvaluator =
				dynamic_cast<IJSP::EvaluationIJSP_MakespanEnergy *>(
					this->evaluator);
			IJSP::ScheduleIJSP *lsSchedule =
				dynamic_cast<IJSP::ScheduleIJSP *>(optimised.first);
			IJSP::ProblemIJSP *lexProblem =
				dynamic_cast<IJSP::ProblemIJSP *>(
					this->sharedVariables->problem);
			if (lexEvaluator == NULL || lsSchedule == NULL
				|| lexProblem == NULL) {
				std::string errorMsg = "Lexicographic fitness requires the ";
				errorMsg += "ijsp.makespan-energy evaluation.";
				throw FuzzyFWException("MemeticPareto", errorMsg);
			}
			delete optimised.second;
			optimised.second = lexEvaluator->evaluateSchedule(lsSchedule,
				lexProblem);
		}

		if (this->lsLamarckism)
			this->sharedVariables->encoder->encode(optimised.first,
				target, this->sharedVariables);
		target->updatePhenotype(optimised.first);
		target->updateFitness(optimised.second);
		population->setSorted(false);

		FitnessLexicographic *fin =
			dynamic_cast<FitnessLexicographic *>(target->getFitness());
		if (fin == NULL || !target->isPhenotypeUpdated())
			return;
		// Offer REAL (unclamped) values under a goal cap
		IJSP::EvaluationIJSP_MakespanEnergy *ev2 = dynamic_cast<
			IJSP::EvaluationIJSP_MakespanEnergy *>(this->evaluator);
		IJSP::ScheduleIJSP *sch2 = dynamic_cast<IJSP::ScheduleIJSP *>(
			target->getPhenotype());
		IJSP::ProblemIJSP *prob2 = dynamic_cast<IJSP::ProblemIJSP *>(
			this->sharedVariables->problem);
		if (ev2 != NULL && sch2 != NULL && prob2 != NULL) {
			FitnessLexicographic *real = dynamic_cast<
				FitnessLexicographic *>(ev2->evaluateSchedule(sch2,
					prob2, true));
			this->archive.offer(sch2->toString(), real);
			delete real;
		}
		else
			this->archive.offer(target->getPhenotype()->toString(), fin);
	}
};

}
