/*
* Fitness.h
*
*  Created on: June 7, 2017
*      Author: Juan Jose Palacios
*/
#ifndef OBJECTS_FITNESS_H_
#define OBJECTS_FITNESS_H_

#include "TFN.h"

namespace FJSP {

//=============================================================================
//
//	Abstract class Fitness
//
//=============================================================================
/**
* This class defines the methods that fitness values must implement.
*
* The idea of the class is to be able to compare different fitness without
* checking everytime the maximization/minimization conditions.
* Also to have a wrapper that can be used for any kind of fitness type
*
* @author Juan Jose Palacios
*
*/
class Fitness {
public:
	enum Type { INTEGER, DOUBLE, FUZZY };


	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
protected:
	/*
	* Indicates the optimisation strategy
	*/
	bool maximize;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	Fitness(bool maxim = true) : maximize(maxim) { }


	/*
	* Copy consrtuctor
	*/
	Fitness(const Fitness &source)
		: maximize(source.maximize) { }


	/*
	* Clone method for inherited instances
	*/
	virtual Fitness* clone() const = 0;



	//=========================================================================
	//		GET / SET METHODS
	//=========================================================================
	/*
	* Check if the fitness must be maximized
	*/
	virtual bool mustMaximize() const {
		return this->maximize;
	}


	/*
	* Gets the type of the Fitness to be identified
	*/
	virtual Fitness::Type getType() const = 0;

	/*
	* Converts the Fitness into a string
	*/
	virtual std::string toString() const = 0;


	/*
	* Converts the Fitness into a double value
	*/
	virtual double toDouble() const {
		throw new FJSPException("Fitness",
			"This fitness type cannot be converted to double");
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	virtual bool isBetterOrEqualTo(const Fitness * f)
		const = 0;
	virtual bool isBetterThan(const Fitness * f) const = 0;
	virtual bool isEqualTo(const Fitness * f) const = 0;
	virtual bool isWorseThan(const Fitness * f) const = 0;
	virtual bool isWorseOrEqualTo(const Fitness * f)
		const = 0;
};





//=============================================================================
//
//	Class FitnessDouble
//
//=============================================================================
/**
* Class were the fitness values are of type double
*
* @author Juan Jose Palacios
*
*/
class FitnessDouble : public Fitness {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	/*
	* Fitness value
	*/
	double value;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	FitnessDouble(bool maxim = true) : Fitness(maxim) { }

	/*
	* Main constructor
	*/
	FitnessDouble(const double d, bool maxim = true)
		: Fitness(maxim), value(d) { }

	/*
	* Copy constructor
	*/
	FitnessDouble(const FitnessDouble &fitness)
		: Fitness(fitness), value(fitness.value) { }


	/*
	* Clone method for inherited instances
	*/
	virtual Fitness* clone() const {
		return new FitnessDouble(*this);
	}


	//=========================================================================
	//		GET / SET METHODS
	//=========================================================================
public:
	/*
	* Gets the type of the Fitness to be identified
	*/
	virtual Fitness::Type getType() const {
		return Fitness::Type::DOUBLE;
	}

	/*
	* Gets the fitness value
	*/
	double getValue() const {
		return this->value;
	}

	/*
	* Sets the fitness value
	*/
	void setValue(const double source) {
		this->value = source;
	}

	/*
	* Converts the Fitness into a string
	*/
	virtual std::string toString() const {
		return valueToString(this->value);
	}

	/*
	* Converts the Fitness into a double value
	*/
	virtual double toDouble() const {
		return this->value;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	virtual bool isBetterOrEqualTo(const Fitness * f) const;
	virtual bool isBetterThan(const Fitness * f) const;
	virtual bool isEqualTo(const Fitness * f) const;
	virtual bool isWorseThan(const Fitness * f) const;
	virtual bool isWorseOrEqualTo(const Fitness * f) const;

private:
	/*
	* Auxiliary method: Casting of Fitness types to Double type
	* to make comparisons
	*/
	const FitnessDouble * convertType(const Fitness *f) const;
};





//=============================================================================
//
//	Class FitnessInteger
//
//=============================================================================
/**
* Class were the fitness values are of type integer
*
* @author Juan Jose Palacios
*
*/
class FitnessInteger : public Fitness {
	//=========================================================================
	//		FIELDS
	//=========================================================================
protected:
	/*
	* Fitness value
	*/
	int value;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	FitnessInteger(bool maxim = true) : Fitness(maxim) { }

	/*
	* Main constructor
	*/
	FitnessInteger(const int i, bool maxim = true)
		: Fitness(maxim), value(i) { }


	/*
	* Copy constructor
	*/
	FitnessInteger(const FitnessInteger &fitness)
		: Fitness(fitness), value(fitness.value) { }


	/*
	* Clone method for inherited instances
	*/
	virtual Fitness* clone() const {
		return new FitnessInteger(*this);
	}



	//=========================================================================
	//		GET / SET METHODS
	//=========================================================================
public:
	/*
	* Gets the type of the Fitness to be identified
	*/
	virtual Fitness::Type getType() const {
		return Fitness::Type::INTEGER;
	}

	/*
	* Gets the fitness value
	*/
	int getValue() const {
		return this->value;
	}

	/*
	* Sets the fitness value
	*/
	void setValue(const int source) {
		this->value = source;
	}

	/*
	* Converts the Fitness into a string
	*/
	virtual std::string toString() const {
		return valueToString(this->value);
	}

	/*
	* Converts the Fitness into a double value
	*/
	virtual double toDouble() const {
		return (double)this->value;
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	virtual bool isBetterOrEqualTo(const Fitness * f) const;
	virtual bool isBetterThan(const Fitness * f) const;
	virtual bool isEqualTo(const Fitness * f) const;
	virtual bool isWorseThan(const Fitness * f) const;
	virtual bool isWorseOrEqualTo(const Fitness * f) const;

private:
	/*
	* Auxiliary method: Casting of Fitness types to Integer type
	* to make comparisons
	*/
	const FitnessInteger * convertType(const Fitness *f) const;
};





//=============================================================================
//
//	Class FitnessTFN
//
//=============================================================================
/**
* Class were the fitness values are of type TFN
*
* @author Juan Jose Palacios
*
*/
class FitnessTFN : public Fitness {
	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/*
	* Comparison strategy
	*/
	static TFN::Compare FitnessCompareStrategy;



protected:
	/*
	* Fitness value
	*/
	TFN value;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/*
	* Default constructor
	*/
	FitnessTFN(bool maxim = true) : Fitness(maxim) { }

	/*
	* Main constructor
	*/
	FitnessTFN(const TFN & tfn, bool maxim = true)
		: Fitness(maxim), value(tfn) { }

	/*
	* Copy constructor
	*/
	FitnessTFN(const FitnessTFN &fitness)
		: Fitness(fitness), value(fitness.value) { }


	/*
	* Clone method for inherited instances
	*/
	virtual Fitness* clone() const {
		return new FitnessTFN(*this);
	}



	//=========================================================================
	//		GET / SET METHODS
	//=========================================================================
public:
	/*
	* Gets the type of the Fitness to be identified
	*/
	virtual Fitness::Type getType() const {
		return Fitness::Type::FUZZY;
	}

	/*
	* Gets the fitness value
	*/
	TFN getValue() const {
		return this->value;
	}

	/*
	* Sets the fitness value
	*/
	void setValue(const TFN source) {
		this->value = source;
	}

	/*
	* Converts the Fitness into a string
	*/
	virtual std::string toString() const {
		return this->value.toString();
	}

	/*
	* Converts the Fitness into a double value
	*/
	virtual double toDouble() const {
		return this->value.expectedValue();
	}



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	virtual bool isBetterOrEqualTo(const Fitness * f) const;
	virtual bool isBetterThan(const Fitness * f) const;
	virtual bool isEqualTo(const Fitness * f) const;
	virtual bool isWorseThan(const Fitness * f) const;
	virtual bool isWorseOrEqualTo(const Fitness * f) const;

private:
	/*
	* Auxiliary method: Casting of Fitness types to TFN type
	* to make comparisons
	*/
	const FitnessTFN * convertType(const Fitness *f) const;
};

}

#endif  /* OBJECTS_FITNESS_H_ */
