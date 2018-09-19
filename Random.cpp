#include "Random.h"

namespace FuzzyFW {


	//==== Constructor  ===========================================================
	Random::Random(int seed) {
		if (seed < 0)
			this->seed = (int)time(NULL);
		else
			this->seed = seed;
		srand(this->seed);
	}


	//==== Method init  ===========================================================
	void Random::init(int seed) {
		srand(seed);
	}


	//==== Method getInteger  ======================================================
	int Random::getInteger() {
		return rand();
	}


	//==== Method getInteger  ======================================================
	int Random::getInteger(int lower, int upper) {
		int dif = upper - lower;
		if (dif < 0) {
			std::swap(lower, upper);
			dif = -dif;
		}
		else if (dif == 0)
			return lower;

		return lower + (int)floor((dif + 1) * ((double)rand() / (RAND_MAX + 1.0)));
	}


	//==== Method getReal  ========================================================
	double Random::getDouble(int nDigits) {
		float r, factor = 1;
		for (int i = 0; i < nDigits; i++)
			factor *= 10;
		r = rand() / factor;
		return r;
	}


	//==== Method getReal  ========================================================
	double Random::getDouble(double lower, double upper, int nDigits) {
		double diff, residual, factor = 1;
		int maxValue;

		diff = upper - lower;
		if (diff < 0) {
			std::swap(lower, upper);
			diff = -diff;
		}

		for (int i = 0; i < nDigits; i++)
			factor *= 10;

		maxValue = (int)floor(factor*diff);
		residual = diff - maxValue / factor;

		return lower + (residual / 2) + (getInteger(0, maxValue) / factor);
	}


	//==== Method getProbability  =================================================
	double Random::getProbability() {
		return (double)rand() / (RAND_MAX + 1.0);
	}


	//==== Method getRandomVector  ================================================
	std::vector<int> Random::getRandomVector(int lower, int upper) {
		std::vector<int> solution;
		int dif = upper - lower;
		if (dif < 0) {
			std::swap(lower, upper);
			dif = -dif;
		}

		for (int i = lower; i <= upper; i++) {
			solution.push_back(i);
		}
		shuffle(solution);
		return solution;
	}

}

