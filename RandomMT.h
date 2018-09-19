/*
* RandomMT.h
*
*  Created on: Jul 23, 2015
*      Author: Juan Jose Palacios
*/

#ifndef UTILS_RANDOM_MT_H_
#define UTILS_RANDOM_MT_H_

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <time.h>
#include <vector>
#include <random>

#include <iostream>
#include "Random.h"

namespace FJSP {

	/**
	* Class to generate random numbers
	*
	* It uses the Standard Random numbers generator
	*
	* @author jjpalacios
	*
	*/
	class RandomMT : public Random {
	public:

		//! Default constructor
		RandomMT(int seed = -1);

		//! Initialize random seed
		void init(int seed);

		//! Get an integer value in the interval [0, RAND_MAX]
		int getInteger();

		//! Get an integer value in the interval [lower, upper]
		int getInteger(int lower, int upper);

		//! Get a real value in the interval [0, RAND_MAX]
		//! with nDecimals decimal digits
		double getDouble(int nDecimals = 3);

		//! Get a real value in the interval [lower, upper]
		//! with nDecimals decimal digits
		double getDouble(double lower, double upper, int nDecimals = 5);

		//! Get a real value in the interval [0, 1)
		double getProbability();

		//! Get an array of random integer values in the interval [lower, upper]
		std::vector<int> getRandomVector(int lower, int upper);

		//! Given an array, it shuffles it by random
		template<class T>
		void shuffle(std::vector<T> & elements);

	protected:
		std::mt19937 mt_rand;
	};

}

#endif /* UTILS_RANDOM_MT_H_ */
