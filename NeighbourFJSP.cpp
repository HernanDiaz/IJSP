/*
* Neighbour.cpp
*
*  Created on: Oct 11, 2017
*      Author: jjpalacios
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
	x(source.x), y(source.y),
	newHeadX(source.newHeadX), newHeadY(source.newHeadY),
	newTailX(source.newTailX), newTailY(source.newTailY) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Set values  ----------------------------------------------------------
void NeighbourFJSP_Arc::setValues(const unsigned int x, const unsigned int y) {
	this->x = x;
	this->y = y;
	if (this->estimatedQuality != NULL)
		delete this->estimatedQuality;
	this->isEstimated = false;
	this->headsUpdated = false;
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



//-----  Head And Tail computation  -------------------------------------------
void NeighbourFJSP_Arc::updateHeadsTails(ScheduleFJSP *solution) {
	FuzzyFW::TFN tailX, tailY, headX, headY;
	int mpy, jpy, msy, jsy;
	int mpx, jpx, msx, jsx;
	int mac;
	FuzzyFW::TFN lower;

	mac = solution->taskInfo[x].task->machine;
	mpy = solution->taskInfo[y].mp;
	jpy = solution->taskInfo[y].task->jp;
	msy = solution->taskInfo[y].ms;
	msx = solution->taskInfo[x].ms;
	mpx = solution->taskInfo[x].mp;
	jpx = solution->taskInfo[x].task->jp;

	if (solution->lastTaskJob[solution->taskInfo[x].task->job] == x)
		jsx = -1;
	else
		jsx = solution->taskInfo[x].task->js;
	if (solution->lastTaskJob[solution->taskInfo[y].task->job] == y)
		jsy = -1;
	else
		jsy = solution->taskInfo[y].task->js;

	if (msx != y || mpy != x || solution->taskInfo[y].task->machine != mac)
		return;

	// New tail for task X
	solution->updateTails(FuzzyFW::TFN::Maximum::M_COMPONENT);
	if (jsx != -1 && msy != -1)
		tailX = maximum(solution->taskInfo[jsx].tail + solution->taskInfo[jsx].task->p,
			solution->taskInfo[msy].tail + solution->taskInfo[msy].task->p,
			FuzzyFW::TFN::M_COMPONENT);
	else if (jsx != -1)
		tailX = solution->taskInfo[jsx].tail + solution->taskInfo[jsx].task->p;
	else if (msy != -1)
		tailX = solution->taskInfo[msy].tail + solution->taskInfo[msy].task->p;
	else
		tailX = FuzzyFW::TFN(0, 0, 0);

	// New tail for task Y
	if (jsy != -1)
		tailY = maximum(solution->taskInfo[jsy].tail + solution->taskInfo[jsy].task->p,
			tailX + solution->taskInfo[x].task->p, FuzzyFW::TFN::M_COMPONENT);
	else
		tailY = tailX + solution->taskInfo[x].task->p;

	// New head for task Y
	if (mpx != -1 && jpy != -1)
		headY = maximum(solution->taskInfo[mpx].head + solution->taskInfo[mpx].task->p,
			solution->taskInfo[jpy].head + solution->taskInfo[jpy].task->p,
			FuzzyFW::TFN::M_COMPONENT);
	else if (mpx != -1)
		headY = solution->taskInfo[mpx].head + solution->taskInfo[mpx].task->p;
	else if (jpy != -1)
		headY = solution->taskInfo[jpy].head + solution->taskInfo[jpy].task->p;
	else headY = FuzzyFW::TFN(0, 0, 0);

	// New head for task X
	if (jpx != -1)
		headX = maximum(headY + solution->taskInfo[y].task->p,
			solution->taskInfo[jpx].head + solution->taskInfo[jpx].task->p,
			FuzzyFW::TFN::M_COMPONENT);
	else headX = headY + solution->taskInfo[y].task->p;

	lower = maximum(headX + solution->taskInfo[x].task->p + tailX,
		headY + solution->taskInfo[y].task->p + tailY,
		FuzzyFW::TFN::M_COMPONENT);

	newHeadX = headX;
	newTailX = tailX;
	newHeadY = headY;
	newTailY = tailY;

	this->headsUpdated = true;
}

}
