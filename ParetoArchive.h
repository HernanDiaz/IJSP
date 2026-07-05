/*
 * ParetoArchive.h
 *
 *  Created on: Jul 5, 2026
 *      Author: hdiaz
 */
#pragma once

#include "Fitness.h"
#include "FitnessInterval.h"
#include "FitnessMO.h"
#include "Individual.h"
#include <string>
#include <vector>

namespace FuzzyFW {

//=============================================================================
//
//	Class ParetoArchive
//
//=============================================================================
/**
* Bounded archive of non-dominated solutions for bi-objective search with
* lexicographic-pair fitness (two FitnessInterval components, each totally
* ordered by the globally configured interval comparison, e.g. LEX2).
*
* Dominance over the product order: a dominates b iff a is better-or-equal
* in both components and strictly better in at least one. When the archive
* exceeds its capacity, the entry with the smallest crowding distance
* (computed on the interval midpoints of both objectives) is discarded;
* extreme points are always preserved.
*
* The archive stores the GENOTYPE string of each solution (as printed in
* the _Sols files) so fronts can be re-decoded and post-processed offline.
*
* @author hdiaz
*/
class ParetoArchive {
public:
	struct Entry {
		std::string genotype;
		double c_lo, c_hi;	// makespan interval
		double e_lo, e_hi;	// NPE interval
	};

protected:
	std::vector<Entry> entries;
	size_t capacity;

	/*
	* Total-order comparison per component, delegated to the Fitness
	* objects; strict and better-or-equal variants
	*/
	static bool dominates(const Entry &a, const Entry &b);

	/*
	* Crowding distance of every entry on the (midpoint, midpoint) plane
	*/
	std::vector<double> crowding() const;

public:
	ParetoArchive(const size_t capacity = 100) : capacity(capacity) { }

	void clear() { this->entries.clear(); }

	size_t size() const { return this->entries.size(); }

	const std::vector<Entry> & getEntries() const { return this->entries; }

	/**
	* Offers a solution to the archive. Returns true if it was accepted
	* (i.e. it is not dominated by any archived entry).
	*
	* @param genotype Printable genotype of the solution
	* @param fitness Lexicographic [makespan, NPE] fitness
	*/
	bool offer(const std::string &genotype,
		const FitnessLexicographic *fitness);

	/**
	* Dumps the archive to a CSV file (one row per non-dominated solution)
	*/
	void dump(const std::string &path) const;
};

}
