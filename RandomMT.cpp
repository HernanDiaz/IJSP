#include "RandomMT.h"

namespace FJSP {


	//==== Constructor  =======================================================
	RandomMT::RandomMT(int seed)
		: Random(seed) {
		if (seed < 0)
			this->seed = (unsigned int)time(NULL);
		else
			this->seed = seed;
		this->mt_rand = std::mt19937(this->seed);
	}


	//==== Method init  =======================================================
	void RandomMT::init(int seed) {
		this->mt_rand = std::mt19937(seed);
	}


	//==== Method getInteger  =================================================
	int RandomMT::getInteger() {
		std::uniform_int_distribution<int> distribution(0,
			std::numeric_limits<int>::max());
		return distribution(this->mt_rand);
	}


	//==== Method getInteger  =================================================
	int RandomMT::getInteger(int lower, int upper) {
		std::uniform_int_distribution<int> distribution(lower, upper);
		return distribution(this->mt_rand);
	}


	//==== Method getDouble  ==================================================
	double RandomMT::getDouble(int nDigits) {
		double lim = std::numeric_limits<int>::max();
		std::uniform_real_distribution<double> distribution(-lim, lim);
		double value =  distribution(this->mt_rand);
		
		int factor = 1;
		for (int i = 0; i < nDigits; i++)
			factor *= 10;
		value = std::floor(factor*value);
		value = value / factor;

		return value;
	}


	//==== Method getDouble  ==================================================
	double RandomMT::getDouble(double lower, double upper, int nDigits) {
		std::uniform_real_distribution<double> distribution(lower, upper);
		double value = distribution(this->mt_rand);

		int factor = 1;
		for (int i = 0; i < nDigits; i++)
			factor *= 10;
		value = std::floor(factor*value);
		value = value / factor;

		return value;
	}


	//==== Method getProbability  =============================================
	double RandomMT::getProbability() {
		std::uniform_real_distribution<double> distribution(0.0, 1.0);
		return distribution(this->mt_rand);
	}


	//==== Method getRandomVector  ============================================
	std::vector<int> RandomMT::getRandomVector(int lower, int upper) {
		std::vector<int> solution;
		int dif = upper - lower;
		if (dif < 0)
			return solution;

		for (int i = lower; i <= upper; i++) {
			solution.push_back(i);
		}
		shuffle(solution);
		return solution;
	}


	//==== Method shuffle  ====================================================
	template<class T>
	void RandomMT::shuffle(std::vector<T> & elements) {
		int ran;
		for (int i = 0; i < (int)elements.size(); i++) {
			ran = getInteger(0, (int)elements.size() - 1);
			std::swap(elements[i], elements[ran]);
		}
	}

}

