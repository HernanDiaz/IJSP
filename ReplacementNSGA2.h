/*
 * ReplacementNSGA2.h
 *
 *  Created on: Jul 5, 2026
 *      Author: hdiaz
 */
#pragma once

#include "Replacement.h"
#include "FitnessMO.h"
#include "ParetoArchive.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class ReplacementNSGA2
//
//=============================================================================
/**
* NSGA-II style survival selection for bi-objective search with
* lexicographic-pair fitness (two FitnessInterval components): the union of
* parents and offspring is sorted into non-domination fronts (dominance
* over the product of the two component orders) and survivors are chosen
* front by front, breaking the last front by crowding distance.
*
* Survivors are cloned into the offspring population (which the algorithm
* adopts) and both original populations' individuals are released,
* following the framework contract that apply() consumes oldPopulation.
*
* If an archive is attached (setArchive), every evaluated individual with
* an updated phenotype is offered to it before survival selection — the
* replacement is the single choke point that sees parents and offspring
* every generation.
*
* @author hdiaz
*/
class ReplacementNSGA2 : public Replacement {
protected:
	/*
	* Optional non-dominated archive fed each generation (not owned).
	* mutable: apply() is const in the framework interface.
	*/
	mutable ParetoArchive *archive;

	static bool dominates(const Individual *a, const Individual *b);

public:
	ReplacementNSGA2() : archive(NULL) { }
	ReplacementNSGA2(const ReplacementNSGA2 &source)
		: Replacement(source), archive(source.archive) { }
	virtual ~ReplacementNSGA2() { }

	virtual Replacement * clone() {
		return new ReplacementNSGA2(*this);
	}

	void setArchive(ParetoArchive *archive) { this->archive = archive; }

	virtual void apply(Population *oldPopulation, Population *newPopulation,
		const SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> info;
		info.push_back("Replacement:;NSGA-II (non-dominated sorting + crowding)");
		return info;
	}
};

}
