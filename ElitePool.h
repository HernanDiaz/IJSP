/*
 * ElitePool.h
 *
 *  Created on: Jun 10, 2026
 *      Author: hdiaz
 */
#pragma once

#include "Individual.h"
#include "PathRelinkIJSP.h"

namespace IJSP {

#define POOL_SIZE "pool.size"
#define POOL_MIN_DISTANCE "pool.min-distance"


//=============================================================================
//
//	Class ElitePool
//
//=============================================================================
/**
* Pool of elite individuals managed with a quality + diversity rule.
*
* Distances between elites are disjunctive-pair disagreements between their
* schedules (see PathRelinkIJSP::disagreement), expressed as a fraction of the
* total number of disjunctive pairs of the instance. A candidate that lies
* closer than the minimum distance to an existing elite can only replace that
* elite (if better); otherwise it enters by replacing the worst element.
*
* The pool owns its individuals: rejected candidates are deleted on insertion
* and remaining elites are deleted on clear().
*
* @author hdiaz
*/
class ElitePool {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	std::vector<FuzzyFW::Individual *> elites;

	unsigned int capacity;
	double minDistanceFrac;

	/**
	* Disjunctive pairs of the instance (lazily set on first insertion)
	*/
	double instancePairs;


	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	explicit ElitePool()
		: capacity(0), minDistanceFrac(0.0), instancePairs(0.0) { }

	~ElitePool() {
		this->clear();
	}

	void configure(const unsigned int poolCapacity, const double minDistance);

	void clear();


	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	unsigned int size() const {
		return (unsigned int)this->elites.size();
	}

	FuzzyFW::Individual * get(const unsigned int index) const {
		return this->elites[index];
	}

	int bestIndex() const;

	int worstIndex() const;


	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Quality + diversity insertion. The pool takes ownership of the
	* candidate (it is deleted when rejected).
	*
	* @return true if the candidate stayed in the pool
	*/
	bool tryInsert(FuzzyFW::Individual *candidate);

	/**
	* Insertion ignoring the diversity rule (seeding fallback): pushes while
	* below capacity, otherwise replaces the worst elite if better.
	*/
	void forceInsert(FuzzyFW::Individual *candidate);

	/**
	* Average pairwise distance between elites as a fraction of the total
	* disjunctive pairs (1.0 when fewer than 2 elites)
	*/
	double averageDistanceFrac() const;

	/**
	* Average fitness value of the pool
	*/
	double averageFitness() const;

	/**
	* Keeps the best half of the pool and deletes the rest
	*/
	void dropWorstHalf();


protected:
	double distanceFrac(const FuzzyFW::Individual *a,
		const FuzzyFW::Individual *b) const;

	const ScheduleIJSP * scheduleOf(const FuzzyFW::Individual *individual) const;
};

}
