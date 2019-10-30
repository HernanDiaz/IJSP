/*
* Neighbour.cpp
*
*  Created on: Oct 11, 2019
*      Author: hdiaz
*/

#include "NeighbourIJSP.h"

namespace IJSP {

//=============================================================================
//
//	Class NeighbourIJSP_Arc
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NeighbourIJSP_Arc::NeighbourIJSP_Arc(const NeighbourIJSP_Arc &source)
	: Neighbour(source),
	x(source.x), y(source.y) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Set values  ----------------------------------------------------------
void NeighbourIJSP_Arc::setValues(const unsigned int x, const unsigned int y) {
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
bool NeighbourIJSP_Arc::isEqualTo(const Neighbour *v) const {
	const NeighbourIJSP_Arc *arc =
		dynamic_cast<const NeighbourIJSP_Arc *>(v);

	// The neighbours are of different types
	if (arc == NULL)
		return false;

	if (this->x == arc->x && this->y == arc->y)
		return true;
	return false;
}



//-----  Opposite  ------------------------------------------------------------
bool NeighbourIJSP_Arc::isReverse(const Neighbour *v) const {
	const NeighbourIJSP_Arc *arc =
		dynamic_cast<const NeighbourIJSP_Arc *>(v);

	// The neighbours are of different types
	if (arc == NULL)
		return false;

	if (this->x == arc->y && this->y == arc->x)
		return true;
	return false;
}


}
