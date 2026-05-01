/*
* NeighbourhoodFVRP.h
*
*  Created on: Nov 24, 2017
*/
#pragma once

#include "Neighbour.h"
#include "RouteFVRP.h"

namespace FVRP {

//=============================================================================
//
//	Class NeighbourFVRP_SegSwap
//
//=============================================================================
/**
* This class defines a specific type of neighbour for FVRP problems.
* It defines a neighbour as a swap of two disjoint sequences of visits that
* can or not be in the same route.
*
*
*/
class NeighbourFVRP_SegSwap : public FuzzyFW::Neighbour {
	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/*
	* Vehicle of the first sequence
	*/
	unsigned int vehicle1;

	/*
	* Starting node of the first sequence
	*/
	unsigned int initial1;

	/*
	* Length of the first sequence
	*/
	unsigned int final1;

	/*
	* Vehicle of the first sequence
	*/
	unsigned int vehicle2;

	/*
	* Starting node of the first sequence
	*/
	unsigned int initial2;

	/*
	* Length of the first sequence
	*/
	unsigned int final2;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor.
	*/
	explicit NeighbourFVRP_SegSwap()
		: Neighbour(), vehicle1(-1), initial1(-1), final1(-1)
		, vehicle2(-1), initial2(-1), final2(-1) { }
		
	/*
	* Main constructor.
	*/
	explicit NeighbourFVRP_SegSwap(const int v1, const int n1, const int l1,
		const int v2, const int n2, const int l2)
		: Neighbour(), vehicle1(v1), initial1(n1), final1(l1)
		, vehicle2(v2), initial2(n2), final2(l2) { }

	/**
	* Copy constructor
	*/
	NeighbourFVRP_SegSwap(const NeighbourFVRP_SegSwap &source);

	/**
	* Clone method for inheriting classes
	*/
	virtual Neighbour * clone() const {
		return new NeighbourFVRP_SegSwap(*this);
	}

	/**
	* Destructor. Nothing to do here
	*/
	virtual ~NeighbourFVRP_SegSwap() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Sets the first sequence to swap
	*/
	virtual void setFirstSegment(const unsigned int v, const unsigned int init,
		const unsigned int end);

	/*
	* Sets the second sequence to swap
	*/
	virtual void setSecondSegment(const unsigned int v, const unsigned int init,
		const unsigned int end);

	/*
	* Comparison methods: Equality
	* Indicates if two neighbours are the same
	*/
	virtual bool isEqualTo(const Neighbour *v) const;

	/*
	* Comparison methods: Reverse
	* Indicates if two neighbours are exactly the opposite of each other.
	* That is, if we move to this neighbour, we end up in neighbour v.
	*/
	virtual bool isReverse(const Neighbour *v) const;
};

}
