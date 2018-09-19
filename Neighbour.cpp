/*
* Neighbour.cpp
*
*  Created on: Oct 11, 2017
*      Author: jjpalacios
*/

#include "Neighbour.h"

namespace FuzzyFW {

//=============================================================================
//
//	Abstract class Neighbour
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Copy constructor  ----------------------------------------------------
Neighbour::Neighbour(const Neighbour &source)
	: isEstimated(source.isEstimated) {
	if (source.estimatedQuality != NULL)
		estimatedQuality = source.estimatedQuality->clone();
	else
		estimatedQuality = NULL;
}



//-----  Update the quality estimation  ---------------------------------------
void Neighbour::setEstimatedQuality(Fitness *estimation) {
	if (this->estimatedQuality != NULL)
		delete this->estimatedQuality;
	estimatedQuality = estimation;
	this->isEstimated = true;
}

}
