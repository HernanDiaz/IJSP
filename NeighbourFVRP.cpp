/*
* Neighbour.cpp
*
*  Created on: Nov 24, 2017
*      Author: jjpalacios
*/

#include "NeighbourFVRP.h"

namespace FVRP {

//=============================================================================
//
//	Class NeighbourFVRP_SeqSwap
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
NeighbourFVRP_SegSwap::NeighbourFVRP_SegSwap(const NeighbourFVRP_SegSwap &source)
	: Neighbour(source), vehicle1(source.vehicle1),
	initial1(source.initial1), final1(source.final1),
	vehicle2(source.vehicle2), initial2(source.initial2),
	final2(source.final2) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Set values  ----------------------------------------------------------
void NeighbourFVRP_SegSwap::setFirstSegment(const unsigned int v,
	const unsigned int init, const unsigned int end) {
	this->vehicle1 = v;
	this->initial1 = init;
	this->final1 = end;
	if (this->estimatedQuality != NULL) {
		delete this->estimatedQuality;
		this->estimatedQuality = NULL;
	}
	this->estimated = false;
}



//-----  Set values  ----------------------------------------------------------
void NeighbourFVRP_SegSwap::setSecondSegment(const unsigned int v,
	const unsigned int init, const unsigned int end) {
	this->vehicle2 = v;
	this->initial2 = init;
	this->final2 = end;
	if (this->estimatedQuality != NULL) {
		delete this->estimatedQuality;
		this->estimatedQuality = NULL;
	}
	this->estimated = false;
}



//-----  Equality  ------------------------------------------------------------
bool NeighbourFVRP_SegSwap::isEqualTo(const Neighbour *v) const {
	const NeighbourFVRP_SegSwap *neighbor =
		dynamic_cast<const NeighbourFVRP_SegSwap *>(v);

	// The neighbours are of different types
	if (neighbor == NULL)
		return false;

	if(this->vehicle1 == neighbor ->vehicle1
		&& this->initial1 == neighbor->initial1
		&& this->final1 == neighbor->final1
		&& this->vehicle2 == neighbor->vehicle2
		&& this->initial2 == neighbor->initial2
		&& this->final2 == neighbor->final2)
		return true;

	if (this->vehicle1 == neighbor->vehicle2
		&& this->initial1 == neighbor->initial2
		&& this->final1 == neighbor->final2
		&& this->vehicle2 == neighbor->vehicle1
		&& this->initial2 == neighbor->initial1
		&& this->final2 == neighbor->final1)
		return true;
	return false;
}



//-----  Opposite  ------------------------------------------------------------
bool NeighbourFVRP_SegSwap::isReverse(const Neighbour *v) const {
	// In tis neighbourhood, reverse and equality are the same
	return this->isEqualTo(v);
}

}
