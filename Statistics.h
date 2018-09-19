#pragma once

/*
* Statistics.h
*
*  Created on: Oct 4, 2017
*      Author: jjpalacios
*/
#ifndef SRC_STATISTICS_H_
#define SRC_STATISTICS_H_


#include "Evaluation.h"


namespace FJSP {

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
	virtual double getValue(const SharedVars * svars, Population *population) = 0;

	/*
	* Get the statistic as a string
	*/
	virtual std::string setStatName() const {
		switch (this->type) {
		case STAT_BEST:
			return "Best";
			break;
		case STAT_WORST:
			return "Worst";
				break;
		case STAT_AVG:
			return "Avg";
			break;
		case STAT_SDEV:
			return "Std.Dev.";
			break;
		}
		return "";
	}

	/*
	* Get the value from the population
	*/
	virtual std::string getName() const = 0;

};





//=============================================================================
//
//	Class StatisticsMakespan
//
//=============================================================================
/**
* This class gives the makespan from the population
*
* @author jjpalacios
*
*/
struct StatisticsMakespan : public Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	StatisticsMakespan(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsMakespan(StatisticsMakespan & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsMakespan(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsMakespan() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Get the value from the population
	*/
	virtual double getValue(const SharedVars * svars, Population *population);


	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->setStatName() + " Cmax";
	}
};





//=============================================================================
//
//	Class StatisticsAIavg
//
//=============================================================================
/**
* This class gives the average AI from the population
*
* @author jjpalacios
*
*/
struct StatisticsAIavg : public Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	StatisticsAIavg(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsAIavg(StatisticsAIavg & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsAIavg(*this);
	}


	/*
	* Destructor
	*/
	~StatisticsAIavg() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Get the value from the population
	*/
	virtual double getValue(const SharedVars * svars, Population *population);


	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->setStatName() + " AIavg";
	}
};





//=============================================================================
//
//	Class StatisticsAImin
//
//=============================================================================
/**
* This class gives the minimum AI from the population
*
* @author jjpalacios
*
*/
struct StatisticsAImin : public Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
	*	001 : best
	*	010 : worst
	*	100 : average
	*/
	StatisticsAImin(STAT_TYPE flag = STAT_BEST);


	/*
	* Copy constructot
	*/
	StatisticsAImin(StatisticsAImin & source)
		: Statistics(source) { }


	/*
	* Clone method
	*/
	virtual Statistics * clone() {
		return new StatisticsAImin(*this);
	}



	/*
	* Destructor
	*/
	~StatisticsAImin() { }



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/*
	* Get the value from the population
	*/
	virtual double getValue(const SharedVars * svars, Population *population);


	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->setStatName() + " AImin";
	}
};




}
#endif /* SRC_STATISTICS_H_ */

