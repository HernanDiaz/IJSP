/*
* Random.h
*
*  Created on: Jul 23, 2015
*      Author: Juan Jose Palacios
*/

#ifndef UTILS_RANDOM_H_
#define UTILS_RANDOM_H_

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <time.h>
#include <vector>

#include <iostream>

namespace FJSP {

	/**
	* Class to generate random numbers
	*
	* It uses the Standard Random numbers generator
	*
	* @author jjpalacios
	*
	*/
	class Random {
	public:
		//! Seed for the RNG
		unsigned int seed;


		//! Default constructor
		Random(int seed = -1);

		//! Initialize random seed
		virtual void init(int seed);

		//! Get an integer value in the interval [0, RAND_MAX]
		virtual int getInteger();

		//! Get an integer value in the interval [lower, upper]
		virtual int getInteger(int lower, int upper);

		//! Get a real value in the interval [0, RAND_MAX]
		//! with nDecimals decimal digits
		virtual double getDouble(int nDecimals = 3);

		//! Get a real value in the interval [lower, upper]
		//! with nDecimals decimal digits
		virtual double getDouble(double lower, double upper, int nDecimals = 5);

		//! Get a real value in the interval [0, 1]
		virtual double getProbability();

		//! Get an array of random integer values in the interval [lower, upper]
		virtual std::vector<int> getRandomVector(int lower, int upper);

		//! Given an array, it shuffles it by random
		template<typename T>
		void shuffle(std::vector<T> & elements) {
			int ran;
			for (int i = 0; i < (int)elements.size(); i++) {
				ran = this->getInteger(0, (int)elements.size() - 1);
				std::swap(elements[i], elements[ran]);
			}
		}
	};

}

#endif /* UTILS_RANDOM_H_ */
