/*
* TabuList.cpp
*
*  Created on: Dec 21, 2017
*/

#include "TabuList.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class TabuList
//
//=============================================================================
//=============================================================================
//		CONTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
TabuList::TabuList(ParameterDB *parameters)
	: minSizeLabel(FUZZYFW_TABU_MINSIZE),
	maxSizeLabel(FUZZYFW_TABU_MAXSIZE),
	minSize(0), maxSize(Infi), listSize(0)
{
	if (parameters != NULL)
		this->setup(parameters);
}



//-----  Copy constructor  ----------------------------------------------------
TabuList::TabuList(const TabuList &source)
	: minSizeLabel(source.minSizeLabel), minSize(source.minSize),
	maxSizeLabel(source.maxSizeLabel), maxSize(source.maxSize),
	listSize(source.listSize)
{
	if (source.tabuList.size() > 0) {
		std::list<Neighbour *>::iterator iter;
		for (iter = tabuList.begin(); iter != tabuList.end(); iter++)
			this->tabuList.push_back((*iter)->clone());
	}
}



//-----  Setup method  --------------------------------------------------------
void TabuList::setup(ParameterDB *parameters) {
	// Loads the maximum size of the list
	this->maxSize = parameters->getInteger(this->maxSizeLabel, -1);
	if (this->maxSize < 0) {
		std::cout << "Warning: Tabu list max size not found. Taking ";
		std::cout << " unlimited size by default" << std::endl;
			this->maxSize = Infi;
	}

	// Loads the minimum size of the list
	this->minSize = parameters->getInteger(this->minSizeLabel, -1);
	if (this->minSize < 0) {
		std::cout << "Warning: Tabu list min size not found. Taking ";
		std::cout << " 0 by default" << std::endl;
		this->minSize = 0;
	}
}


//-----  Destructor  ----------------------------------------------------------
TabuList::~TabuList() {
	while (tabuList.size() > 0) {
		delete *(this->tabuList.begin());
		this->tabuList.pop_front();
	}
	this->tabuList.clear();
}





//=============================================================================
//		METHODS
//=============================================================================
//-----  Method is Tabu  ------------------------------------------------------
unsigned int TabuList::isTabu(Neighbour *neighbour) const {
	std::list<Neighbour *>::const_iterator iter;
	unsigned int count = 1;

	iter = this->tabuList.begin();
	while (iter != this->tabuList.end()) {
		if ((*iter)->isReverse(neighbour))
			return count;
		iter++;
	}
	return 0;
}


//-----  Clear method  --------------------------------------------------------
void TabuList::clear() {
	this->listSize = 0;
	while (tabuList.size() > 0) {
		delete *(this->tabuList.begin());
		this->tabuList.pop_front();
	}
	this->tabuList.clear();
}


//-----  Add a neighbour to the list  -----------------------------------------
void TabuList::addNeighbour(Neighbour *neighbour) {
	this->tabuList.push_front(neighbour->clone());

	if (this->listSize >= this->maxSize) {
		delete *(--this->tabuList.end());
		tabuList.pop_back();
	}
	else
		this->listSize++;
}


//-----  Reduce the list size  ------------------------------------------------
void TabuList::reduceSize(unsigned int newSize) {
	while (newSize < this->listSize && this->listSize > this->minSize) {
		delete *(--tabuList.end());
		tabuList.pop_back();
		this->listSize--;
	}
}


//-----  Forces the list size  ------------------------------------------------
void TabuList::forceSize(unsigned int newSize) {
	while (newSize < this->listSize) {
		delete *(--tabuList.end());
		tabuList.pop_back();
		this->listSize--;
	}
}

}