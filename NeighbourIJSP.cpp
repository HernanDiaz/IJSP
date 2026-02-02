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
	x(source.x), y(source.y), z(source.z),tipo(source.tipo) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Set values  ----------------------------------------------------------
void NeighbourIJSP_Arc::setValues(const unsigned int x, const unsigned int y, const unsigned int z, const unsigned int tipo) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->tipo = tipo;
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
	if ((arc == NULL)||(arc->tipo!=tipo))
		return false;

	if (this->x == arc->x && this->y == arc->y && this->z == arc->z)
		return true;
	return false;
}



//-----  Opposite  ------------------------------------------------------------
bool NeighbourIJSP_Arc::isReverse(const Neighbour *v) const {
	const NeighbourIJSP_Arc *arc =
		dynamic_cast<const NeighbourIJSP_Arc *>(v);

	// The neighbours are of different types
	if ((arc == NULL) || (arc->tipo != tipo))
		return false;
	/*type of the arc
		 0->b->a->
		 1->b  c->a->
		 2->f->d  e->
		 3->c->b->a->
		*/
	if (tipo == 0 && this->x == arc->y && this->y == arc->x)
		return true;
	if (tipo == 1 && this->x == arc->y && this->y == arc->z && this->z == arc->x)
		return true;
	if (tipo == 2 && this->x == arc->z && this->y == arc->x && this->z == arc->y)
		return true;
	if (tipo == 3 && this->x == arc->z && this->y == arc->y &&this->z == arc->x)
		return true;
	return false;
}


}
