/*
* Neighbourhood.h
*
*  Created on: Oct 11, 2017
*      Author: jjpalacios
*/
#pragma once

#include "Neighbour.h"
#include "ScheduleFJSP.h"

namespace FJSP {

//=============================================================================
//
//	Class NeighbourFJSP_Arc
//
//=============================================================================
/**
* This class defines a specific type of neighbour for FJSP problems.
* It defines a neighbour as a reversal of a disjuntive arc in the graph
* associated to a solution. The class will keep the origin of the original arc
* as well as the destination
*
* @author jjpalacios
*
*/
class NeighbourFJSP_Arc : public FuzzyFW::Neighbour {
	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/*
	* Origin node of the arc
	*/
	unsigned int x;

	/*
	* Destination node of the arc
	*/
	unsigned int y;

	/*
	* Internal data structures
	* Certain estimators will compute values that are useful
	* later for the full evaluation
	*/
	FuzzyFW::TFN newHeadX;
	FuzzyFW::TFN newHeadY;
	FuzzyFW::TFN newTailX;
	FuzzyFW::TFN newTailY;

protected:
	/*
	* Indicates if the inner strcutres have been updated
	*/
	char headsUpdated;





	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor.
	*/
	explicit NeighbourFJSP_Arc()
		: Neighbour(), x(0), y(0), newHeadX(FuzzyFW::TFN(-1, -1, -1)),
		newHeadY(FuzzyFW::TFN(-1, -1, -1)), newTailX(FuzzyFW::TFN(-1, -1, -1)),
		newTailY(FuzzyFW::TFN(-1, -1, -1)), headsUpdated(false) { }
		
	/*
	* Main constructor.
	*/
	explicit NeighbourFJSP_Arc(const int x, const int y)
		: Neighbour(), x(x), y(y), newHeadX(FuzzyFW::TFN(-1, -1, -1)),
		newHeadY(FuzzyFW::TFN(-1, -1, -1)), newTailX(FuzzyFW::TFN(-1, -1, -1)),
		newTailY(FuzzyFW::TFN(-1, -1, -1)), headsUpdated(false) { }

	/**
	* Copy constructor
	*/
	NeighbourFJSP_Arc(const NeighbourFJSP_Arc &source);

	/**
	* Clone method for inheriting classes
	*/
	virtual Neighbour * clone() const {
		return new NeighbourFJSP_Arc(*this);
	}

	/**
	* Destructor. Nothing to do here
	*/
	virtual ~NeighbourFJSP_Arc() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Sets the nodes of the arc
	*/
	virtual void setValues(const unsigned int x, const unsigned int y);

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


	/*
	* Intermediate state before fully evaluating the solution.
	* It computes the new head and tail values for the nodes that are
	* implicated in the neighbour
	*/
	void updateHeadsTails(ScheduleFJSP *solution);
};

}
