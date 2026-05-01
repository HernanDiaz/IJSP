/*
* Neighbour.cpp
*
*  Created on: Oct 11, 2017
*/

#include "NeighbourFJSP.h"

namespace FJSP {

//=============================================================================
//
//	Class NeighbourFJSP_Arc
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NeighbourFJSP_Arc::NeighbourFJSP_Arc(const NeighbourFJSP_Arc &source)
	: Neighbour(source),
	x(source.x), y(source.y) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Set values  ----------------------------------------------------------
void NeighbourFJSP_Arc::setValues(const unsigned int x, const unsigned int y) {
	this->x = x;
	this->y = y;
	if (this->estimatedQuality != NULL) {
		delete this->estimatedQuality;
		this->estimatedQuality = NULL;
	}
	if (this->evaluated) {
		delete this->solution.first;
		delete this->solution.second;
		this->solution.first = NULL;
		this->solution.second = NULL;
	}
	this->evaluated = false;
	this->estimated = false;
}



//-----  Equality  ------------------------------------------------------------
bool NeighbourFJSP_Arc::isEqualTo(const Neighbour *v) const {
	const NeighbourFJSP_Arc *arc =
		dynamic_cast<const NeighbourFJSP_Arc *>(v);

	// The neighbours are of different types
	if (arc == NULL)
		return false;

	if (this->x == arc->x && this->y == arc->y)
		return true;
	return false;
}



//-----  Opposite  ------------------------------------------------------------
bool NeighbourFJSP_Arc::isReverse(const Neighbour *v) const {
	const NeighbourFJSP_Arc *arc =
		dynamic_cast<const NeighbourFJSP_Arc *>(v);

	// The neighbours are of different types
	if (arc == NULL)
		return false;

	if (this->x == arc->y && this->y == arc->x)
		return true;
	return false;
}


}
