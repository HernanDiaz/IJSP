/*
 * ABCPSOPareto.h
 *
 *  Created on: Jul 5, 2026
 *      Author: hdiaz
 */
#pragma once

#include "ArtificialBeeColonyPSO.h"
#include "ParetoArchive.h"
#include "ReplacementNSGA2.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class ABCPSOPareto
//
//=============================================================================
/**
* Pareto-archive variant of the ABC-PSO memetic algorithm for bi-objective
* search with lexicographic-pair fitness (e.g. ijsp.makespan-energy).
*
* The evolution engine is inherited unchanged; survival selection is meant
* to be the NSGA-II replacement (replacement = nsga2), to which this class
* attaches a bounded non-dominated archive. Every run starts with a clean
* archive, and at the end of the run the archive is dumped to
* <logFolder><signature>_Front.csv (one non-dominated solution per row,
* interval bounds of both objectives plus the printable solution), so
* fronts can be post-processed offline like the _Sols files.
*
* The per-individual fitness remains lexicographic, so the energy-aware
* local search (N2ME) and the statistics machinery work untouched:
* lexicographic exploitation + Pareto survival + archive.
*
* @author hdiaz
*/
class ABCPSOPareto : public ArtificialBeeColonyPSO {
protected:
	/*
	* Bounded non-dominated archive (owned; fed by the NSGA-II replacement)
	*/
	ParetoArchive archive;

public:
	ABCPSOPareto(ParameterDB *params = NULL)
		: ArtificialBeeColonyPSO(params), archive(100) { }

	virtual ~ABCPSOPareto() { }

	virtual void prepareToRun(ParameterDB *params);

	virtual std::pair<Solution *, Objective *> run(Problem *problem,
		std::string signature, std::string logFolder, int rngSeed);

	/**
	* The ABC engine performs its own trial-based survival and never calls
	* the replacement operator, so the archive is fed here instead: with
	* the tuned config (localsearch.target 1.0, period 1) every individual
	* passes through the local search each generation.
	*/
	virtual void applyLocalSearch(Population *population,
		const unsigned int individualIdx);
};

}
