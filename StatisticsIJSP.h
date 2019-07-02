#pragma once

/*
* StatisticsIJSP.h
*
*  Created on: June 25, 2019
*      Author: Hernan Diaz
*/
#pragma once


#include "Statistics.h"
#include "EvaluationIJSP.h"


namespace IJSP {

//=============================================================================
//
//	Class StatisticsMakespan
//
//=============================================================================
/**
* This class gives the makespan from the population
*
* @author hdiaz
*
*/
struct StatisticsMakespan : public FuzzyFW::Statistics {
public:
	/*
	* Main constructor
	* The flag indicates the metric to use:
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
	* Get the value from an individual
	*/
	virtual double getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
		FuzzyFW::Individual *individual) {
		return this->evaluation->evaluate(svars, individual)->toDouble();
	}


	/*
	* Get the value from the population
	*/
	virtual std::string getName() const {
		return this->getStatName() + " Cmax";
	}
};
}
