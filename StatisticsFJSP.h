#pragma once

/*
* StatisticsFJSP.h
*
*  Created on: Oct 4, 2017
*      Author: jjpalacios
*/
#pragma once


#include "Statistics.h"
#include "EvaluationFJSP.h"


namespace FJSP {

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
struct StatisticsMakespan : public FuzzyFW::Statistics {
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
	virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
		FuzzyFW::Population *population);


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
struct StatisticsAIavg : public FuzzyFW::Statistics {
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
	virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
		FuzzyFW::Population *population);


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
struct StatisticsAImin : public FuzzyFW::Statistics {
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
	virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
		FuzzyFW::Population *population);


	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->setStatName() + " AImin";
	}
};

}
