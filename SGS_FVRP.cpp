/*
* SGS_FJSP.cpp
*
*  Created on: June 1, 2017
*/

#include "SGS_FVRP.h"

namespace FVRP {

//=============================================================================
//
//	Abstract class SGS_FVRP
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//=====  Default constructor  =================================================
SGS_FVRP::SGS_FVRP(const FuzzyFW::ParameterDB *params)
	: routePlan(NULL), isCreated(false) {
	if (params != NULL)
		this->setup(params);
}


//=====  Copy constructor  ====================================================
SGS_FVRP::SGS_FVRP(const SGS_FVRP &source) {
	this->routePlan = new RouteFVRP(*source.routePlan);
	this->isCreated = source.isCreated;
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Reset  ===============================================================
void SGS_FVRP::reset() {
	if (this->isCreated)
		this->routePlan->reset();
}

}
