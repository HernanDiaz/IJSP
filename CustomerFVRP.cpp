/*
* FuzzyTask.cpp
*
*  Created on: May 12, 2017
*      Author: Juan Jose Palacios
*/

#include "CustomerFVRP.h"

namespace FVRP {

//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
CustomerFVRP::CustomerFVRP() :
	id(0), timeWindow(NULL), demand(FuzzyFW::TFN(0, 0, 0)),
	serviceTime(FuzzyFW::TFN(0, 0, 0)), x(0), y(0)
{ }
	
//====  Copy constructor  =====================================================
CustomerFVRP::CustomerFVRP(const CustomerFVRP & source)
	: id(source.id), demand(source.demand), serviceTime(source.serviceTime)
	, x(source.x), y(source.y)
{ 
	timeWindow = source.timeWindow->clone();
}



//=============================================================================
//		OPERATORS
//=============================================================================
//====  Assignment overload  ==================================================
CustomerFVRP & CustomerFVRP::operator = (const CustomerFVRP &src) {
	this->id = src.id;
	this->demand = src.demand;
	this->serviceTime = src.serviceTime;
	this->x = src.x;
	this->y = src.y;

	timeWindow = src.timeWindow->clone();

	return *this;
}

}
