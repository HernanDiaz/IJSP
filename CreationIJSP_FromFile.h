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
* Solutions are kept sorted by their stored objective (interval midpoint) and
* each call samples one with a rank bias — the index is the minimum of two
* uniform draws, so better-stored solutions are linearly more likely — then
* rebuilds it through the configured SGS. With probability
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


	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	explicit CreationFromFileSchedule(FuzzyFW::ParameterDB *parameters = NULL)
		: CreationRandomSchedule(parameters), loaded(false) { }

	CreationFromFileSchedule(const CreationFromFileSchedule &source)
		: CreationRandomSchedule(source), solutionsDir(source.solutionsDir),
		solutions(source.solutions), loaded(source.loaded) { }

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
};

}
