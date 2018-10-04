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
	: estimated(source.estimated), evaluated(source.evaluated) {
	if (source.estimatedQuality != NULL)
		estimatedQuality = source.estimatedQuality->clone();
	else
		estimatedQuality = NULL;
	
	if (source.solution.first != NULL)
		solution.first = source.solution.first->clone();
	else
		solution.first = NULL;

	if (source.solution.second != NULL)
		solution.second = source.solution.second->clone();
	else
		solution.second = NULL;
}



//-----  Update the quality estimation  ---------------------------------------
void Neighbour::setEstimatedQuality(Fitness *estimation) {
	if (this->estimatedQuality != NULL) {
		delete this->estimatedQuality;
		this->estimatedQuality = NULL;
	}
	estimatedQuality = estimation;
	this->estimated = true;
}



//-----  Update the fitness of the individual  --------------------------------
void Neighbour::setEvaluation(Solution *sol, Fitness *fitness) {
	if (this->solution.first != NULL)
		delete this->solution.first;
	this->solution.first = sol;

	if (this->solution.second != NULL)
		delete this->solution.second;
	this->solution.second = fitness;

	if (sol != NULL)
		this->evaluated = true;
	else
		this->estimated = false;
}
}
