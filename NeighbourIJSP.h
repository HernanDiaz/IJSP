/*
* Neighbourhood.h
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/
#pragma once

#include "Neighbour.h"
#include "ScheduleIJSP.h"

namespace IJSP {

//=============================================================================
//
//	Class NeighbourIJSP_Arc
//
//=============================================================================
/**
* This class defines a specific type of neighbour for IJSP problems.
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

	/*
	* Destination node of the arc
	*/
	unsigned int z;

	/*
	* type of the arc
	* 0 a b pasa a  ->b->a->   solo un arco
	* 1 ->b  c->a-> mpmpmp == -1 o no bloque critico
	* 2 ->f->d  e-> ms == -1 o no bloque critico
	* 3 ->c->b->a-> en los dos
	*/
	unsigned int tipo;


	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor.
	*/
	explicit NeighbourIJSP_Arc()
		: Neighbour(), x(0), y(0), z(0), tipo(0)  { }
		
	/*
	* Main constructor.
	*/
	explicit NeighbourIJSP_Arc(const int x, const int y, const int z=0, const int tipo=0)
		: Neighbour(), x(x), y(y), z(z), tipo(tipo) { }

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
	virtual void setValues(const unsigned int x, const unsigned int y, const unsigned int z = 0, const unsigned int tipo = 0);
		
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
