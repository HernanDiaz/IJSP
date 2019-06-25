/*
* Neighbourhood.h
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/
#pragma once

#include "Neighbour.h"


namespace IJSP {

//=============================================================================
//
//	Class NeighbourIJSP_Arc
//
//=============================================================================
/**
* This class defines a specific type of neighbour for FJSP problems.
* It defines a neighbour as a reversal of a disjuntive arc in the graph
* associated to a solution. The class will keep the origin of the original arc
* as well as the destination
*
* @author hdiaz
*
*/
class NeighbourIJSP_Arc : public FuzzyFW::Neighbour {
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





	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor.
	*/
	explicit NeighbourIJSP_Arc()
		: Neighbour(), x(0), y(0) { }
		
	/*
	* Main constructor.
	*/
	explicit NeighbourIJSP_Arc(const int x, const int y)
		: Neighbour(), x(x), y(y) { }

	/**
	* Copy constructor
	*/
	NeighbourIJSP_Arc(const NeighbourIJSP_Arc &source);

	/**
	* Clone method for inheriting classes
	*/
	virtual Neighbour * clone() const {
		return new NeighbourIJSP_Arc(*this);
	}

	/**
	* Destructor. Nothing to do here
	*/
	virtual ~NeighbourIJSP_Arc() { }



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
};

}
