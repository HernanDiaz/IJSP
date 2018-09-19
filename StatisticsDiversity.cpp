/*
* StatisticsDiversity.cpp
*
*  Created on: Oct 4, 2017
*      Author: jjpalacios
*/

#include "StatisticsDiversity.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class StatisticsHamming
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsHamming::StatisticsHamming(STAT_TYPE flag)
	: Statistics(flag) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsHamming::getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	IndividualArrayInt *indiv1, *indiv2;
	double best, worst, sum, sumSq;
	double distance;
	int count = 0;
	best = Infd;
	worst = -1.0;
	sum = sumSq = 0.0;

	if (population->size() <= 1)
		return 0.0;

	for (unsigned int i = 0; i < population->size(); i++) {
		indiv1 = dynamic_cast<IndividualArrayInt *>(population->getIndividual(i));
		if (indiv1 == NULL) {
			std::string errorMsg = "Hamming distance can be computed only on ";
			errorMsg += "arrays of integer values";
			throw new FuzzyFWException("Statistics Hamming", errorMsg);
		}

		for (unsigned int j = i + 1; j < population->size(); j++) {
			indiv2 = dynamic_cast<IndividualArrayInt *>(population->getIndividual(i));
			if (indiv2 == NULL) {
				std::string errorMsg = "Hamming distance can be computed only on ";
				errorMsg += "arrays of integer values";
				throw new FuzzyFWException("Statistics Hamming", errorMsg);
			}

			distance = indiv1->hammingDistance(indiv2);
			count++;

			if (distance < best)
				best = distance;
			if (distance > worst)
				worst = distance;
			sum += distance;
			sumSq += distance*distance;
		}
	}

	if (this->type == STAT_BEST)
		return best;
	else if (this->type == STAT_WORST)
		return worst;
	else if (this->type == STAT_AVG)
		return sum / count;
	else if (this->type == STAT_SDEV) {
		return sqrt((sumSq - ((sum * sum) / count))
			/ (count - 1));
	}
	return -1;
}





//=============================================================================
//
//	Class StatisticsKendall
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsKendall::StatisticsKendall(STAT_TYPE flag)
	: Statistics(flag) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsKendall::getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	IndividualArrayInt *indiv1, *indiv2;
	double best, worst, sum, sumSq;
	double distance;
	int count = 0;
	best = Infd;
	worst = -1.0;
	sum = sumSq = 0.0;

	if (population->size() <= 1)
		return 0.0;

	for (unsigned int i = 0; i < population->size(); i++) {
		indiv1 = dynamic_cast<IndividualArrayInt *>(population->getIndividual(i));
		if (indiv1 == NULL) {
			std::string errorMsg = "Hamming distance can be computed only on ";
			errorMsg += "arrays of integer values";
			throw new FuzzyFWException("Statistics Hamming", errorMsg);
		}

		for (unsigned int j = i + 1; j < population->size(); j++) {
			indiv2 = dynamic_cast<IndividualArrayInt *>(population->getIndividual(j));
			if (indiv2 == NULL) {
				std::string errorMsg = "Hamming distance can be computed only on ";
				errorMsg += "arrays of integer values";
				throw new FuzzyFWException("Statistics Hamming", errorMsg);
			}

			distance = indiv1->kendallTauDistance(indiv2);
			count++;

			if (distance < best)
				best = distance;
			if (distance > worst)
				worst = distance;
			sum += distance;
			sumSq += distance*distance;
		}
	}

	if (this->type == STAT_BEST)
		return best;
	else if (this->type == STAT_WORST)
		return worst;
	else if (this->type == STAT_AVG)
		return sum / count;
	else if (this->type == STAT_SDEV) {
		return sqrt((sumSq - ((sum * sum) / count))
			/ (count - 1));
	}
	return -1;
}





//=============================================================================
//
//	Class StatisticsNeri
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsNeri::StatisticsNeri(STAT_TYPE flag)
	: Statistics(flag) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsNeri::getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	double best, worst, avg;

	if (population->size() <= 1)
		return 0.0;
	
	best = population->getBest(svars)->getFitness()->toDouble();
	worst = population->getBest(svars, population->size() - 1)->getFitness()->toDouble();
	avg = population->getAverageFitness();

	if (best == worst)
		return 0.0;
	return 1 - (avg - best) / (worst - best);
}





//=============================================================================
//
//	Class StatisticsNeriBest
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsNeriBest::StatisticsNeriBest(STAT_TYPE flag)
	: Statistics(flag) { }



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsNeriBest::getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	double best, worst, avg, sol;

	if (population->size() <= 1)
		return 0.0;

	best = population->getBest(svars)->getFitness()->toDouble();
	avg = population->getAverageFitness();

	if (compareDouble(best, avg) == 0)
		return 0.0;
	if (compareDouble(best, 0.0) == 0)
		return 1.0;

	if (best < avg)
		sol = (avg - best) / best;
	else
		sol = (best - avg) / best;

	return std::min(1.0, sol);
}

}
