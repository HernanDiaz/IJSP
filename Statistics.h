#pragma once

/*
* Statistics.h
*
*  Created on: Oct 4, 2017
*      Author: jjpalacios
*/
#pragma once


#include "Evaluation.h"


namespace FuzzyFW {

//=============================================================================
//
//	Abstract Class Statistics
//
//=============================================================================
/**
* This class allows to get values for statistical purposes.
*
* @author jjpalacios
*
*/
class Statistics {
public:
	enum STAT_TYPE { STAT_BEST, STAT_WORST, STAT_AVG, STAT_SDEV };


	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	/*
	* Keeps the type of statistical value to show
	*/
	STAT_TYPE type;

	/*
	* Evaluation function for the statistical value
	*/
	Evaluation *evaluation;

public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	Statistics(STAT_TYPE flag = STAT_BEST)
		: type(flag) { }


	/*
	* Copy constructot
	*/
	Statistics(Statistics & source)
		: type(source.type) {
		this->evaluation = source.evaluation->clone();
	}


	/*
	* Clone method
	*/
	virtual Statistics * clone() = 0;


	/*
	* Destructor
	*/
	~Statistics() {
		if (evaluation != NULL)
			delete evaluation;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Set the statistic to compute
	*/
	virtual void setStat(STAT_TYPE statType) {
		this->type = statType;
	}


	/*
	* Get the value from the population
	*/
	virtual double getValue(const SharedVarsEvolutionary * svars,
		Population *population) = 0;

	/*
	* Get the value from the population
	*/
	virtual double getValue(const SharedVarsEvolutionary * svars,
		Individual *individual) = 0;

	/*
	* Get the statistic as a string
	*/
	virtual std::string getStatName() const {
		switch (this->type) {
		case STAT_BEST:
			return "Best";
		case STAT_WORST:
			return "Worst";
		case STAT_AVG:
			return "Avg";
		case STAT_SDEV:
			return "Std.Dev.";
		}
		return "";
	}

	/*
	* Get the value from the population
	*/
	virtual std::string getName() const = 0;

};

}
