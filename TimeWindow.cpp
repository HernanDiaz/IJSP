/*
* DueDate.cpp
*
*  Created on: May 12, 2017
*      Author: Juan Jose Palacios
*/

#include "TimeWindow.h"

namespace FuzzyFW {

//=============================================================================
//
//	Abstract class TimeWindow
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Agreement index (TFN)  ===============================================
double TimeWindow::agreementIndex(const TFN c)  const {
	std::string errorMsg;
	errorMsg = "Agreement index cannot be computed for this type of time window";
	throw FuzzyFWException("Time Window", errorMsg);
	return -1.0;
}


//=====  Agreement index (crips value)  =======================================
double TimeWindow::agreementIndex(const double c)  const {
	std::string errorMsg;
	errorMsg = "Agreement index cannot be computed for this type of time window";
	throw FuzzyFWException("Time Window", errorMsg);
	return -1.0;
}


//=====  Delay (TFN)  =========================================================
TFN TimeWindow::delay(const TFN c)  const {
	std::string errorMsg;
	errorMsg = "Tardiness cannot be computed for this type of timw window";
	throw FuzzyFWException("Time Window", errorMsg);
	return TFN(-1, -1, -1);
}


//=====  Delay (crisp value)  =================================================
double TimeWindow::delay(const double c)  const {
	std::string errorMsg;
	errorMsg = "Tardiness cannot be computed for this type of time window";
	throw FuzzyFWException("Time Window", errorMsg);
	return -1.0;
}



//=============================================================================
//		INPUT / OUTPUT
//=============================================================================
//====  Input operator  =======================================================
std::istream & operator>>(std::istream & is, TimeWindow * d) {
	d->readFromStream(is);
	return is;
}


//====  Output operator  ======================================================
std::ostream & operator<<(std::ostream & os, TimeWindow * d) {
	os << d->toString();
	return os;
}





//=============================================================================
//
//	Class TimeWindowCrisp
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
TimeWindowCrisp::TimeWindowCrisp()
	: TimeWindow() {
	this->timeLimit = -1.0;
}


//====  Main constructor  =====================================================
TimeWindowCrisp::TimeWindowCrisp(const double twValue)
	: TimeWindow() {
	if (compareDouble(twValue, 0.0) == -1) {
		std::string errorMsg = "Invalid time window: ";
		errorMsg += valueToString(twValue) + ".";
		errorMsg += "Negative values are not allowed";
		throw FuzzyFWException("Time Window", errorMsg);
	}

	this->timeLimit = twValue;
	this->loadedData = true;
}


//====  Copy constructor  =====================================================
TimeWindowCrisp::TimeWindowCrisp(const TimeWindowCrisp &tw)
	: TimeWindow(tw) {
	this->timeLimit = tw.timeLimit;
}


//====  Clone method  =========================================================
TimeWindow * TimeWindowCrisp::clone() const {
	return new TimeWindowCrisp(*this);
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Agreement index (TFN)  ===============================================
double TimeWindowCrisp::agreementIndex(const TFN c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid time window";
		throw FuzzyFWException("Time Window", errorMsg);
	}

	double c1 = c.a;
	double c2 = c.b;
	double c3 = c.c;
	double dd = this->timeLimit;

	// c3 <= d
	if (compareDouble(c3, dd) < 1)
		return 1.0;
	// c1 >= d (and c2 >= d and c3 > d)
	if (compareDouble(c1, dd) > -1)
		return 0.0;
	// c2 >= d (and c3 > d and c1 < d)
	if (compareDouble(c2, dd) > -1)
		return ((dd - c1)*(dd - c1)) / ((c2 - c1)*(c3 - c1));

	// (c3 > d and c1 < d and c2 < d)
	else {
		return (c3*(dd - c2) + dd*(c3 - dd) - c1*(c3 - c2)) /
			((c3 - c2) * (c3 - c1));
	}

	// Case not controlled
	return -1.0;
}


//=====  Agreement index (crips value)  =======================================
double TimeWindowCrisp::agreementIndex(const double c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid time window";
		throw FuzzyFWException("Time Window", errorMsg);
	}

	// c <= d
	if (compareDouble(c, this->timeLimit < 1))
		return 1.0;
	return 0.0;
}


//=====  Delay (TFN)  =========================================================
TFN TimeWindowCrisp::delay(const TFN c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid time window";
		throw FuzzyFWException("Time Window", errorMsg);
	}
	double x = c.a - this->timeLimit;
	double y = c.b - this->timeLimit;
	double z = c.c - this->timeLimit;
	return TFN(x, y, z);
}


//=====  Delay (crisp value)  =================================================
double TimeWindowCrisp::delay(const double c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid time window";
		throw FuzzyFWException("Time Window", errorMsg);
	}
	return c - this->timeLimit;
}



//=============================================================================
//		INPUT / OUTPUT
//=============================================================================
//=====  Read from a stream  ==================================================
std::istream & TimeWindowCrisp::readFromStream(std::istream & is) {
	is >> this->timeLimit;
	this->loadedData = true;
	return is;
}


//=====  Convert to string  ===================================================
std::string TimeWindowCrisp::toString() {
	if (!this->loadedData) {
		std::string errorMsg = "Time window not initialized";
		throw FuzzyFWException("Time Window", errorMsg);
	}
	return valueToString(this->timeLimit);
}





//=============================================================================
//
//	Class TimeWindowLinear
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//====  Default constructor  ==================================================
TimeWindowLinear::TimeWindowLinear()
	: TimeWindow() {
	this->d1 = this->d2 = -1.0;
}


//====  Main constructor  =====================================================
TimeWindowLinear::TimeWindowLinear(const double dd1, const double dd2)
	: TimeWindow() {
	if (compareDouble(dd1, 0.0) == -1 || compareDouble(dd2, 0.0) == -1) {
		std::string errorMsg = "Invalid time window: ";
		errorMsg += "(" + valueToString(d1) + ",";
		errorMsg += valueToString(d2) + "). ";
		errorMsg += "Negative values are not allowed";
		throw FuzzyFWException("Time Window", errorMsg);
	}
	else if (compareDouble(d2, d1) == -1) {
		std::string errorMsg = "Invalid time window: ";
		errorMsg += "(" + valueToString(d1) + ",";
		errorMsg += valueToString(d2) + "). ";
		errorMsg += "Second value must be equal or greater than the first one";
		throw FuzzyFWException("Time Window", errorMsg);
	}

	this->d1 = dd1;
	this->d2 = dd2;
	this->loadedData = true;
}


//====  Copy constructor  =====================================================
TimeWindowLinear::TimeWindowLinear(const TimeWindowLinear &tw)
	: TimeWindow(tw) {
	this->d1 = tw.d1;
	this->d2 = tw.d2;
}


//====  Clone method  =========================================================
TimeWindow * TimeWindowLinear::clone() const {
	return new TimeWindowLinear(*this);
}



//=============================================================================
//		METHODS
//=============================================================================
//=====  Agreement index (TFN)  ===============================================
double TimeWindowLinear::agreementIndex(const TFN c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid time window";
		throw FuzzyFWException("Time Window", errorMsg);
	}

	double c1 = c.a;
	double c2 = c.b;
	double c3 = c.c;

	double ip1, ip2;	// Intersection points

	// c2 <= d1 and c3 <= d2
	if (compareDouble(c2, d1) < 1 && compareDouble(c3, d2) < 1)
		return 1.0;
	// c1 >= d2
	if (compareDouble(c1, d2) > -1)
		return 0.0;
	// c2 >= d1 and c3 >= d2
	if (compareDouble(c2, d1) > -1 && compareDouble(c3, d2) > -1) {
		ip1 = ((c2*d2) - (c1*d1)) / (c2 - c1 + d2 - d1);
		if (compareDouble(c1, c2) == 0)
			return ((ip1 - d2)*(d2 - c1)) / ((d1 - d2)*(c3 - c1));
		else
			return ((ip1 - c1)*(d2 - c1)) / ((c2 - c1)*(c3 - c1));
	}
	// c2 < d1 and c3 > d2
	if (compareDouble(c2, d1) < 0 && compareDouble(c3, d2) > 0) {
		ip2 = ((c3*d1) - (c2*d2)) / (c3 - c2 + d1 - d2);
		return (c3*(ip2 - c2) + d2*(c3 - ip2) - c1*(c3 - c2)) /
			((c3 - c2) * (c3 - c1));
	}
	else {
		ip1 = ((c2*d2) - (c1*d1)) / (c2 - c1 + d2 - d1);
		if (compareDouble(c1, c3) == 0)
			return (ip1 - d2) / (d1 - d2);
		else {
			ip2 = ((c3*d1) - (c2*d2)) / (c3 - c2 + d1 - d2);
			return (ip1*(c1 - d2) + ip2*(d2 - c3) + d2*(c3 - c1)) /
				((d2 - d1) * (c3 - c1));
		}
	}
	return -1.0;
}



//=====  Agreement index (crips value)  =======================================
double TimeWindowLinear::agreementIndex(const double c)  const {
	if (!this->loadedData) {
		std::string errorMsg = "Agreement index cannot be computed. ";
		errorMsg += "Invalid time window";
		throw FuzzyFWException("Time Window", errorMsg);
	}

	// c <= d1
	if (compareDouble(c, this->d1) < 1)
		return 1.0;
	// c >= d2
	if (compareDouble(c, this->d2) > -1)
		return 0.0;
	// d1 < c < d2
	else
		return (c - d2) / (d1 - d2);
	return -1.0;
}


	
//=============================================================================
//		INPUT / OUTPUT
//=============================================================================
//=====  Read from a stream  ==================================================
std::istream & TimeWindowLinear::readFromStream(std::istream & is) {
	char c;
	is >> c >> this->d1 >> c >> this->d2 >> c;
	this->loadedData = true;
	return is;
}


//=====  Convert to string  ===================================================
std::string TimeWindowLinear::toString() {
	if (!this->loadedData) {
		std::string errorMsg = "Time Window is not initialized";
		throw FuzzyFWException("Time Window", errorMsg);
	}
	std::string value = "(" + valueToString(this->d1);
	value = "," + valueToString(this->d2) + ")";
	return value;
}
}
