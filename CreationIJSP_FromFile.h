/*
 * CreationIJSP_FromFile.h
 *
 *  Created on: Jun 10, 2026
 *      Author: hdiaz
 */
#pragma once

#include "CreationIJSP_Base.h"

namespace IJSP {

#define CREATION_SOLUTIONS_DIR "creation.solutions-dir"
#define CREATION_SEED_SELECTION "creation.seed-selection"  // rankbias | maxmin


//=============================================================================
//
//	Class CreationFromFileSchedule
//
//=============================================================================
/**
* Creation strategy that warm-starts from previously stored solutions.
*
* Reads the task orders written by the framework's solutions writer
* (<instance>_Sols.csv lines: "run;<task ids separated by spaces>;(lo, up)")
* from `creation.solutions-dir`/<problem name>_Sols.csv. Header or malformed
* lines are skipped, so several solution files can simply be concatenated to
* mix sources (e.g. N2 + N8 runs).
*
* Solutions are kept sorted by their stored objective (interval midpoint).
* Two selection strategies (`creation.seed-selection`):
*   - `rankbias` (default): each call samples one with a rank bias (the index
*     is the minimum of two uniform draws, so better solutions are linearly
*     more likely);
*   - `maxmin`: the solutions are decoded once and handed out in a greedy
*     maximum-diversity order (start from the best, then repeatedly the one
*     farthest, in disjunctive distance, from those already seeded), so the
*     pool that path relinking works on is as spread out as possible.
* The chosen solution is rebuilt through the configured SGS. With probability
* `creation.randomratio` (base-class mix machinery) a pure random individual
* is returned instead, so pool re-seeding keeps injecting fresh diversity
* once the stored material is absorbed.
*
* @author hdiaz
*/
class CreationFromFileSchedule : public CreationRandomSchedule {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	std::string solutionsDir;

	/**
	* Stored task orders with their stored objective midpoint, sorted by
	* quality; lazily loaded on the first createIndividual call (the problem
	* name is only known then)
	*/
	mutable std::vector< std::pair<double, std::vector<int> > > solutions;
	mutable bool loaded;

	/**
	* maxmin selection: greedy maximum-diversity order of the stored
	* solutions (indices) and the next one to hand out; built lazily
	*/
	bool useMaxMin;
	mutable std::vector<int> maxminOrder;
	mutable size_t nextPick;


	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	explicit CreationFromFileSchedule(FuzzyFW::ParameterDB *parameters = NULL)
		: CreationRandomSchedule(parameters), loaded(false),
		useMaxMin(false), nextPick(0) { }

	CreationFromFileSchedule(const CreationFromFileSchedule &source)
		: CreationRandomSchedule(source), solutionsDir(source.solutionsDir),
		solutions(source.solutions), loaded(source.loaded),
		useMaxMin(source.useMaxMin), nextPick(0) { }

	virtual void setup(FuzzyFW::ParameterDB *parameters);

	virtual Creation * clone() const {
		return new CreationFromFileSchedule(*this);
	}


	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	virtual FuzzyFW::Individual * createIndividual(
		const FuzzyFW::SharedVarsEvolutionary *svars) const;

	virtual std::vector<std::string> getName() const;

protected:
	void loadSolutions(const FuzzyFW::SharedVarsEvolutionary *svars) const;

	/**
	* Builds the greedy maximum-diversity order over the stored solutions
	* (decodes each through the SGS once and uses the disjunctive distance)
	*/
	void buildMaxMinOrder(const FuzzyFW::SharedVarsEvolutionary *svars) const;
};

}
