#pragma once
/*
* LocalSearch.h
*
*  Created on: Oct 6, 2017
*      Author: jjpalacios
*/
#ifndef LOCAL_SEARCH_H_
#define LOCAL_SEARCH_H_

#include "Population.h"
#include "SchedulingClassRegister.h"
#include "Encoder.h"

namespace FJSP {

/*
* 	-frequency : Indicates when the hill climbing must be applied.This
* 			parameter can take three values : initial, final and period.
* 			If set to initial, hill climbing will be applied only to the
* 			initial population.If it is final, only to the last population.
* 			If it is period, it will be applied every "period-freq" iterations
* 	-period - freq : If the previous option is "period", we must specify the
* 			number of iterations between different hill - climbing
* 	-target : best, worst, percentage value.Indicates to which solutions in
* 			the population apply the hill climbing
*/
#define LOCAL_SEARCH_FREQ "localsearch.frequency"
#define LOCAL_SEARCH_PERIOD "localsearch.period"
#define LOCAL_SEARCH_TARGET "localsearch.target"

#define LS_FREQ_INITIAL "initial"
#define LS_FREQ_FINAL "final"
#define LS_FREQ_PERIOD "period"
#define LS_FREQ_STUCK "stuck"

#define LS_TARGET_BEST "target"
#define LS_TARGET_WORST "worst"



	//=============================================================================
	//
	//	Abstract class LocalSearch
	//
	//=============================================================================
	/**
	* This class provides the general framework that every type of local search
	* must follow.
	* It basically consist on 
	*
	* @author jjpalacios
	*
	*/
	class LocalSearch {
		//=========================================================================
		//		CONSTRUCTORS / INITIALIZERS
		//=========================================================================
	public:
		/**
		* Default constructor
		*/
		explicit Creation(ParameterDB *parameters = NULL) {
			if (parameters != NULL)
				this->setup(parameters);
		}





#endif /* LOCAL_SEARCH_H_ */
