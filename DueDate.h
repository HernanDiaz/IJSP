/*
* DueDate.h
*
*  Created on: May 12, 2017
*      Author: Juan Jose Palacios
*/
#ifndef PROBLEM_DUEDATE_H_
#define PROBLEM_DUEDATE_H_

#include "TFN.h"

namespace FJSP {


//=============================================================================
//
//	Abstract class DueDate
//
//=============================================================================
/**
* This class provides the common elements for using Due-Dates
*
*
* The classes will consist on an array of elements containing the defining
* points of the due-date, depending on the kind we are using.
*
* It will be used to compute the satisfaction degree of the due-date as well.
* Notice that depending on the type of the due-date, the satisfaction may
* be measured in different ways or under different terms
*
* @author Juan Jose Palacios
*
*/
class DueDate {
public:

	// Different types of DueDate implemented
	enum Type { CRISP, LINEAR, TRAPEZOID };


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
	DueDate() 
	: loadedData(false) { };	// Nothing to create. That was easy!

	/**
	* Copy constructor
	*/
	DueDate(const DueDate &duedate)
		: loadedData(duedate.loadedData) { };

	/**
	* Clone method to replicate inherited instances
	*/
	virtual DueDate * clone() const = 0;

	/**
	* Destructor
	*/
	virtual ~DueDate() { };	// Nothing to destroy here



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	* Get the type of the due-date
	*/
	virtual Type getType() const = 0;

	/**
	* Get the name of the due-date type
	*/
	virtual std::string getName() const = 0;



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Degree of satisfaction of due-date given a TFN. In this case, is
	* measured as the intersection between both values divided by the area
	* of the completion time.
	* Warning! Some Due-date types may not have this option
	*/
	virtual double agreementIndex(const TFN c) const;

	/**
	* Degree of satisfaction of due-date given a crisp value. In this case,
	* is comes to the satisfaction degree given by the due date
	* function in the point x=c.
	* Warning! Some Due-date types may not have this option
	*/
	virtual double agreementIndex(const double c) const;

	/**
	* Delay of a TFN with respect to the due-date
	* Warning! Some Due-date types may not have this option
	*/
	virtual TFN delay(const TFN c) const;

	/**
	* Delay of a crisp value with respect to the due-date
	* Warning! Some Due-date types may not have this option
	*/
	virtual double delay(const double c) const;



	//=========================================================================
	//		INPUT / OUTPUT
	//=========================================================================
public:
	/**
	* Overloads the input operator to read the due-date from a
	* stream of data
	*/
	friend std::istream & operator >> (std::istream & is, DueDate * d);

	/**
	* Overloads the output operator to write the due-date through a
	* stream of data
	*/
	friend std::ostream & operator << (std::ostream & os, DueDate * d);

	/**
	* Takes the due-date value from a stream of data
	*/
	virtual std::istream & readFromStream(std::istream & is) = 0;

	/**
	* Prints the due-date value through a stream of data
	*/
	virtual std::string toString() = 0;
};







//=============================================================================
//
//	Class DueDateCrisp
//
//=============================================================================
/**
* This class inherits from DueDate.
* It contains a crisp due-date, or strict due-date. Those are the due-dates
* that are defined by a crisp value. They represent a strict deadline
*
* @author Juan Jose Palacios
*
*/
class DueDateCrisp : public DueDate {

	//=========================================================================
	//		FIELDS
	//=========================================================================
public:
	/**
	* Due-date value
	*/
	double duedate;



	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	/**
	* Default constructor
	*/
	DueDateCrisp();

	/**
	* Main constructor
	*/
	DueDateCrisp(const double duedateValue);

	/**
	* Copy constructor
	*/
	DueDateCrisp(const DueDateCrisp &duedate);

	/**
	* Destructor
	*/
	virtual ~DueDateCrisp() { };	// Nothing to destroy here

	/**
	* Clone method to replicate inherited instances
	*/
	virtual DueDate * clone() const;



	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	* Get the type of the due-date
	*/
	virtual Type getType() const {
		return Type::CRISP;
	};

	/**
	* Get the name of the due-date type
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
	* the TFN that is below the due-date
	*/
	virtual double agreementIndex(const TFN c) const;

	/**
	* In this case the agreement index does not make sense, being 1.0 if
	* the deadline is met, and 0.0 otherwise
	*/
	virtual double agreementIndex(const double c) const;

	/**
	* Absolute delay of a TFN with respect to the due-date (difference)
	*/
	virtual TFN delay(const TFN c) const;

	/**
	* Absolute delay of a TFN with respect to the due-date (difference)
	*/
	virtual double delay(const double c) const;



	//=========================================================================
	//		INPUT / OUTPUT
	//=========================================================================
public:
	/**
	* Takes the due-date value from a stream of data
	*/
	virtual std::istream & readFromStream(std::istream & is);

	/**
	* Prints the due-date value through a stream of data
	*/
	virtual std::string toString();
};







//=============================================================================
//
//	Class DueDateLinear
//
//=============================================================================
/**
* This class inherits from DueDate.
* It contains a flexible due-date defined by a linear satisfaction function.
* Typically, the function will take value 1 until a value d1 is reached, and
* decrease linearly until a value d2, from which the satisfaction level
* becomes 0.
*
* @author Juan Jose Palacios
*
*/
class DueDateLinear : public DueDate {

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
	DueDateLinear();

	/**
	* Main constructor
	*/
	DueDateLinear(const double dd1, const double dd2);

	/**
	* Copy constructor
	*/
	DueDateLinear(const DueDateLinear &duedate);

	/**
	* Clone method to replicate inherited instances
	*/
	virtual DueDate * clone() const;


	/**
	* Destructor
	*/
	virtual ~DueDateLinear() { };	// Nothing to destroy here

	
	
	//=========================================================================
	//		GET/SET METHODS
	//=========================================================================
public:
	/**
	* Get the type of the due-date
	*/
	virtual Type getType() const {
		return Type::LINEAR;
	};

	/**
	* Get the name of the due-date type
	*/
	virtual std::string getName() const {
		return "Linear";
	};


	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	/**
	* Degree of satisfaction of the due-date given a TFN.
	*/
	virtual double agreementIndex(const TFN c) const;

	/**
	* Degree of satisfaction of the due-date given a crisp value.
	*/
	virtual double agreementIndex(const double c) const;

	/**
	* delay functions do not apply to this kind of due-date
	*/
	//virtual TFN delay(const TFN c);
	//virtual double delay(const double c);



	//=========================================================================
	//		INPUT / OUTPUT
	//=========================================================================
public:
	/**
	* Takes the due-date value from a stream of data
	*/
	virtual std::istream & readFromStream(std::istream & is);

	/**
	* Prints the due-date value through a stream of data
	*/
	virtual std::string toString();
};

}

#endif /* PROBLEM_DUEDATE_H_ */
