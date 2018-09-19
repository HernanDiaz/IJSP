/*
* TimeWindow.h
*
*  Created on: May 12, 2017
*      Author: Juan Jose Palacios
*/
#pragma once

#include "TFN.h"

namespace FuzzyFW {


//=============================================================================
//
//	Abstract class TimeWindow
//
//=============================================================================
/**
* This class provides the common elements for using time windows 
* and/or due dates
*
* The classes will consist on an array of elements containing the defining
* points of the fuzzy time wndow, depending on the kind we are using.
*
* It will be used to compute the satisfaction degree of the constraint as well.
* Notice that depending on the type of the time window, the satisfaction may
* be measured in different ways or under different terms
*
* @author Juan Jose Palacios
*
*/
class TimeWindow {
public:

	// Different types of Taime windows implemented
	enum Type { DEADLINE, CRISP, LINEAR, TRAPEZOID };


	//=========================================================================
	//		COMMON FIELDS
	//=========================================================================
public:
	/**
	* Initialization flag
	*/
	char loadedData;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Main constructor
	*/
	TimeWindow()
	: loadedData(false) { };	// Nothing to create. That was easy!

	/**
	* Copy constructor
	*/
	TimeWindow(const TimeWindow &tw)
		: loadedData(tw.loadedData) { };

	/**
	* Clone method to replicate inherited instances
	*/
	virtual TimeWindow * clone() const = 0;

	/**
	* Destructor
	*/
	virtual ~TimeWindow() { };	// Nothing to destroy here



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	* Get the type of the time window
	*/
	virtual Type getType() const = 0;

	/**
	* Get the name of the time window type
	*/
	virtual std::string getName() const = 0;



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Degree of satisfaction of the time window given a TFN. In this case, is
	* measured as the intersection between both values divided by the area
	* of the completion time.
	* Warning! Some Time Windows types may not have this option
	*/
	virtual double agreementIndex(const TFN c) const;

	/**
	* Degree of satisfaction of the time window given a crisp value. In this case,
	* is comes to the satisfaction degree given by the time window
	* function in the point x=c.
	* Warning! Some Time Windows types may not have this option
	*/
	virtual double agreementIndex(const double c) const;

	/**
	* Delay of a TFN with respect to the time window
	* Warning! Some Time Window types may not have this option
	* Takes a negative value in case of being out of the window
	*/
	virtual TFN delay(const TFN c) const;

	/**
	* Delay of a crisp value with respect to the time window
	* Warning! Some Time Window types may not have this option
	*/
	virtual double delay(const double c) const;



	//=========================================================================
	//		INPUT / OUTPUT
	//=========================================================================
public:
	/**
	* Overloads the input operator to read the time window from a
	* stream of data
	*/
	friend std::istream & operator >> (std::istream & is, TimeWindow * d);

	/**
	* Overloads the output operator to write the time window through a
	* stream of data
	*/
	friend std::ostream & operator << (std::ostream & os, TimeWindow * d);

	/**
	* Takes the time window value from a stream of data
	*/
	virtual std::istream & readFromStream(std::istream & is) = 0;

	/**
	* Prints the time window value through a stream of data
	*/
	virtual std::string toString() = 0;
};







//=============================================================================
//
//	Class TimeWindowDeadline
//
//=============================================================================
/**
* This class inherits from TimeWindow.
* It contains a crisp deadline, or strict deadline.
*
* @author Juan Jose Palacios
*
*/
class TimeWindowDeadline : public TimeWindow {

	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/**
	* time window value
	*/
	double timeLimit;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	TimeWindowDeadline();

	/**
	* Main constructor
	*/
	TimeWindowDeadline(const double twValue);

	/**
	* Copy constructor
	*/
	TimeWindowDeadline(const TimeWindowDeadline &tw);

	/**
	* Destructor
	*/
	virtual ~TimeWindowDeadline() { };	// Nothing to destroy here

	/**
	* Clone method to replicate inherited instances
	*/
	virtual TimeWindow * clone() const;



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	* Get the type of the time window
	*/
	virtual Type getType() const {
		return Type::CRISP;
	};

	/**
	* Get the name of the time window type
	*/
	virtual std::string getName() const {
		return "Crisp Deadline";
	};



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* In this case, the agreement index is the poportion of the area of
	* the TFN that is below the time window
	*/
	virtual double agreementIndex(const TFN c) const;

	/**
	* In this case the agreement index does not make sense, being 1.0 if
	* the deadline is met, and 0.0 otherwise
	*/
	virtual double agreementIndex(const double c) const;

	/**
	* Absolute delay of a TFN with respect to the time window (difference)
	*/
	virtual TFN delay(const TFN c) const;

	/**
	* Absolute delay of a TFN with respect to the time window (difference)
	*/
	virtual double delay(const double c) const;



	//=========================================================================
	//		INPUT / OUTPUT
	//=========================================================================
public:
	/**
	* Takes the time window value from a stream of data
	*/
	virtual std::istream & readFromStream(std::istream & is);

	/**
	* Prints the time window value through a stream of data
	*/
	virtual std::string toString();
};





//=============================================================================
//
//	Class TimeWindowCrisp
//
//=============================================================================
/**
* This class inherits from TimeWindow.
* It contains a crisp time window
*
* @author Juan Jose Palacios
*
*/
class TimeWindowCrisp : public TimeWindow {

	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/**
	* time window values
	*/
	double earlyTime;
	double lateTime;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	TimeWindowCrisp();

	/**
	* Main constructor
	*/
	TimeWindowCrisp(const double early, const double late);

	/**
	* Copy constructor
	*/
	TimeWindowCrisp(const TimeWindowCrisp &tw);

	/**
	* Destructor
	*/
	virtual ~TimeWindowCrisp() { };	// Nothing to destroy here

	/**
	* Clone method to replicate inherited instances
	*/
	virtual TimeWindow * clone() const;



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	* Get the type of the time window
	*/
	virtual Type getType() const {
		return Type::CRISP;
	};

	/**
	* Get the name of the time window type
	*/
	virtual std::string getName() const {
		return "Crisp";
	};



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* In this case, the agreement index is the poportion of the area of
	* the TFN that is below the time window
	*/
	virtual double agreementIndex(const TFN c) const;

	/**
	* In this case the agreement index does not make sense, being 1.0 if
	* the deadline is met, and 0.0 otherwise
	*/
	virtual double agreementIndex(const double c) const;

	/**
	* Absolute delay of a TFN with respect to the time window (difference)
	*/
	virtual TFN delay(const TFN c) const;

	/**
	* Absolute delay of a TFN with respect to the time window (difference)
	*/
	virtual double delay(const double c) const;



	//=========================================================================
	//		INPUT / OUTPUT
	//=========================================================================
public:
	/**
	* Takes the time window value from a stream of data
	*/
	virtual std::istream & readFromStream(std::istream & is);

	/**
	* Prints the time window value through a stream of data
	*/
	virtual std::string toString();
};





//=============================================================================
//
//	Class TimeWindowLinear
//
//=============================================================================
/**
* This class inherits from TimeWindow.
* It contains a flexible time window defined by a linear satisfaction function.
* Typically, the function will take value 1 until a value d1 is reached, and
* decrease linearly until a value d2, from which the satisfaction level
* becomes 0.
*
* @author Juan Jose Palacios
*
*/
class TimeWindowLinear : public TimeWindow {

	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/**
	* Satisfaction function defining points
	*/
	double d1;
	double d2;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	TimeWindowLinear();

	/**
	* Main constructor
	*/
	TimeWindowLinear(const double dd1, const double dd2);

	/**
	* Copy constructor
	*/
	TimeWindowLinear(const TimeWindowLinear &tw);

	/**
	* Clone method to replicate inherited instances
	*/
	virtual TimeWindow * clone() const;


	/**
	* Destructor
	*/
	virtual ~TimeWindowLinear() { };	// Nothing to destroy here

	
	
	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	* Get the type of the time window
	*/
	virtual Type getType() const {
		return Type::LINEAR;
	};

	/**
	* Get the name of the time window type
	*/
	virtual std::string getName() const {
		return "Linear";
	};


	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Degree of satisfaction of the time window given a TFN.
	*/
	virtual double agreementIndex(const TFN c) const;

	/**
	* Degree of satisfaction of the time window given a crisp value.
	*/
	virtual double agreementIndex(const double c) const;

	/**
	* delay functions do not apply to this kind of time window
	*/
	//virtual TFN delay(const TFN c);
	//virtual double delay(const double c);



	//=========================================================================
	//		INPUT / OUTPUT
	//=========================================================================
public:
	/**
	* Takes the time window value from a stream of data
	*/
	virtual std::istream & readFromStream(std::istream & is);

	/**
	* Prints the time window value through a stream of data
	*/
	virtual std::string toString();
};

}
