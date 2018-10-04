#pragma once

#include "Neighbour.h"
#include "ParameterDB.h"

namespace FuzzyFW {

	// Creation parameters defined in this header file
#define FUZZYFW_TABU_MINSIZE "localsearch.tabu-size.min"
#define FUZZYFW_TABU_MAXSIZE "localsearch.tabu-size.max"



//=============================================================================
//
//	Class TabuList
//
//=============================================================================
/**
* This class defines a list of neighbours that are forbidden. It is a support
* class for Tabu Search algorithms.
*
* Requires two parameters, indicating the minimum and maximum size of the list.
* In the case of dynamic length lists, they fix the boundaries of the length.
*
* @author jjpalacios
*
*/
class TabuList {
	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* Minimum list size
	*/
	unsigned int minSize;
	std::string minSizeLabel;


	/*
	* Maximum list size
	*/
	unsigned int maxSize;
	std::string maxSizeLabel;


	/*
	* Current size of the list
	*/
	unsigned int listSize;


	/*
	* List of tabu neighbours
	*/
	std::list<Neighbour *> tabuList;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	explicit TabuList(ParameterDB *parameters = NULL);

	/**
	* Copy constructor
	*/
	TabuList(const TabuList &source);

	/**
	* Loads the needed parameters: No parameters needed
	*/
	virtual void setup(ParameterDB *parameters);


	/**
	* Clone method for inheriting classes
	*/
	virtual TabuList * clone() const {
		return new TabuList(*this);
	}


	/**
	* Destructor
	*/
	virtual ~TabuList();



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
	/*
	* Current list size
	*/
	virtual unsigned int size() const {
		return this->listSize;
	}


	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		std::string line;
		line = "Tabu-list min.Size:;";
		line += valueToString(this->minSize);
		setup.push_back(line);
		line = "Tabu-list max.Size:;";
		line += valueToString(this->maxSize);
		setup.push_back(line);
		return setup;
	}




	//=========================================================================
	//		METHODS
	//=========================================================================
	/*
	* Indicates if a neighbour is tabu or not.
	* Returns the number if neighbours checked until finding it, or 0 if
	* it is not in the list
	*/
	virtual unsigned int isTabu(Neighbour *neighbour) const;

	/*
	* Clear the list
	*/
	void clear();

	/*
	* Insert a new neighbour in the list
	* If the size is exceeded, the oldes neighbour is dumped
	*/
	void addNeighbour(Neighbour *neighbour);

	/*
	* Reduce the list size to the given value.
	* The discarded neighbours are the oldes ones
	*/
	void reduceSize(unsigned int newSize);

	/*
	* Reduce the list size to the given value.
	* The discarded neighbours are the oldes ones.
	* Forces the change of size under/above the limits
	*/
	void forceSize(unsigned int newSize);
};

}
