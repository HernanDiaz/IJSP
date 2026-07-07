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
};

}
